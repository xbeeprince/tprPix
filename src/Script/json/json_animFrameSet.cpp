/*
 * ================== json_animFrameSet.cpp =====================
 *                          -- tpr --
 *                                        创建 -- 2019.07.06
 *                                        修改 -- 
 * ----------------------------------------------------------
 */
//--------------- CPP ------------------//
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>
#include <memory>

//--------------- Libs ------------------//
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "tprGeneral.h"

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "tprCast.h"
#include "global.h"
#include "fileIO.h"
#include "AnimFrameSet.h"
#include "AnimAction.h"
#include "ColliderType.h"
#include "esrc_animFrameSet.h"
#include "esrc_state.h"

//--------------- Script ------------------//
#include "Script/json/json_all.h"
#include "Script/json/json_oth.h"
#include "Script/resource/ssrc_all.h" 

using namespace rapidjson;

#include <iostream>
using std::cout;
using std::endl;

namespace json{//------------- namespace json ----------------
namespace afsJson_inn {//-------- namespace: afsJson_inn --------------//

  
    class AFSPng{
    public:
        std::string path {};
        IntVec2 frameNum {};
        size_t totalFrameNum {};
        bool isHaveShadow {};
        bool isPjtSingle {};
        bool isShadowSingle {};
        ColliderType  colliderType {};
        std::vector<std::shared_ptr<AnimActionParam>> actionParams {};
    };

    class AnimFrameSetJsonData{
    public:
        std::string name {};
        std::vector<std::shared_ptr<AFSPng>> afsPngs {};
    };

    //===== funcs =====//
    std::shared_ptr<AFSPng> parse_AFSPng( const Value &pngEnt_ );

    void parse_subspecies_in_handleType( const Value &subspeciesEnt_,
                            std::vector<std::shared_ptr<AnimActionParam>> &params_,
                            bool isPjtSingle_ );
    void parse_subspecies_in_batchType( const Value &subspeciesEnt_,
                            std::vector<std::shared_ptr<AnimActionParam>> &params_,
                            bool isPjtSingle_ );

    void parse_AnimActionParam( size_t  subspeciesIdx_,
                                const Value &actionParamEnt_,
                                std::vector<std::shared_ptr<AnimActionParam>> &params_,
                                const std::vector<AnimLabel> &labels_,
                                bool isPjtSingle_ );

    std::shared_ptr<AnimActionParam> singleFrame(   size_t  subspeciesIdx_,
                                                    const Value &actionParamEnt_,
                                                    const std::vector<AnimLabel> &labels_,
                                                    bool isPjtSingle_ );
    
    std::shared_ptr<AnimActionParam> multiFrame(    size_t  subspeciesIdx_,
                                                    const Value &actionParamEnt_, 
                                                    const std::vector<AnimLabel> &labels_ );

    NineDirection jsonStr_2_ninedirection( const std::string &str_ );
}//------------- namespace: afsJson_inn end --------------//


/* ===========================================================
 *             parse_from_animFrameSetJsonFile
 * -----------------------------------------------------------
 * Do Not Worry About Performance !!!
 */
void parse_from_animFrameSetJsonFile(){

    cout << "   ----- parse_from_animFrameSetJsonFile: start ----- " << endl;

    std::vector<std::string> path_files {};
    collect_fileNames( "animFrameSet", "files.json", path_files );
    //---
    AnimActionPos::prepare_colliPointOffs();

    //-----------------------------//
    //         load file
    //-----------------------------//
    std::string path_file {};

    for( const auto &path : path_files ){

        auto jsonBufUPtr = read_a_file( path );

        //-----------------------------//
        //      parce JSON data
        //-----------------------------//
        Document doc;
        doc.Parse( jsonBufUPtr->c_str() );

        tprAssert( doc.IsArray() );
        for( auto &ent : doc.GetArray() ){

            afsJson_inn::AnimFrameSetJsonData jsonData {};

            {//--- name ---//
                const auto &a = check_and_get_value( ent, "name", JsonValType::String );
                jsonData.name = a.GetString();
            }
            {//--- pngs [] ---//
                const auto &a = check_and_get_value( ent, "pngs", JsonValType::Array );
                for( auto &pngEnt : a.GetArray() ){//- foreach pngs
                    jsonData.afsPngs.push_back( afsJson_inn::parse_AFSPng( pngEnt ) );
                }
            }

            //--- insert to esrc::animFrameSets
            auto &animFrameSetRef = esrc::insert_new_animFrameSet( jsonData.name );
            for( auto &pngSPtr : jsonData.afsPngs ){

                animFrameSetRef.insert_a_png(   pngSPtr->path,
                                                pngSPtr->frameNum,
                                                pngSPtr->totalFrameNum,
                                                pngSPtr->isHaveShadow,
                                                pngSPtr->isPjtSingle,
                                                pngSPtr->isShadowSingle,
                                                pngSPtr->colliderType,
                                                pngSPtr->actionParams );
            }
        }

    }

    //-------- special ids ----------//
    esrc::set_emptyPixId( esrc::apply_a_random_animSubspeciesId( "emptyPix", emptyAnimLabels, 1) );
    esrc::set_fieldRimId( esrc::apply_a_random_animSubspeciesId( "fieldRim", emptyAnimLabels, 1) );
    //...

    
    //-------------
    esrc::insertState("json_animFrameSet");
    cout << "   ----- parse_from_animFrameSetJsonFile: end ----- " << endl;
}


namespace afsJson_inn {//-------- namespace: afsJson_inn --------------//



/* ===========================================================
 *                    parse_AFSPng
 * -----------------------------------------------------------
 */
std::shared_ptr<AFSPng> parse_AFSPng( const Value &pngEnt_ ){

    auto afsPng = std::make_shared<AFSPng>();

    {//--- path ---//
        const auto &a = check_and_get_value( pngEnt_, "path", JsonValType::String );
        afsPng->path = a.GetString();
    }
    {//--- frameNum.col ---//
        const auto &a = check_and_get_value( pngEnt_, "frameNum.col", JsonValType::Int );
        afsPng->frameNum.x =  a.GetInt();
    }
    {//--- frameNum.row ---//
        const auto &a = check_and_get_value( pngEnt_, "frameNum.row", JsonValType::Int );
        afsPng->frameNum.y =  a.GetInt();
    }
    {//--- totalFrameNum ---//
        const auto &a = check_and_get_value( pngEnt_, "totalFrameNum", JsonValType::Uint64 );
        afsPng->totalFrameNum =  cast_2_size_t(a.GetUint64());
    }
    {//--- isHaveShadow ---//
        const auto &a = check_and_get_value( pngEnt_, "isHaveShadow", JsonValType::Bool );
        afsPng->isHaveShadow =  a.GetBool();
    }
    {//--- isPjtSingle ---//
        const auto &a = check_and_get_value( pngEnt_, "isPjtSingle", JsonValType::Bool );
        afsPng->isPjtSingle =  a.GetBool();
    }
    {//--- isShadowSingle ---//
        const auto &a = check_and_get_value( pngEnt_, "isShadowSingle", JsonValType::Bool );
        afsPng->isShadowSingle =  a.GetBool();
    }
    {//--- ColliderType ---//
        const auto &a = check_and_get_value( pngEnt_, "ColliderType", JsonValType::String );
        afsPng->colliderType =  str_2_ColliderType( a.GetString() );
    }
    {//--- subspecies ---//
        const auto &a = check_and_get_value( pngEnt_, "subspecies", JsonValType::Array );
        for( auto &ent : a.GetArray() ){//- foreach subspecies
            //--- type ---//
            const auto &typeStr = check_and_get_value( ent, "type", JsonValType::String );
            std::string subType = typeStr.GetString();
            if( subType == "batch" ){
                afsJson_inn::parse_subspecies_in_batchType( ent, afsPng->actionParams, afsPng->isPjtSingle );
            }else if( subType == "handle" ){
                afsJson_inn::parse_subspecies_in_handleType( ent, afsPng->actionParams, afsPng->isPjtSingle );
            }else{
                tprAssert(0);
            }
        }
    }

    return afsPng;
}




/* ===========================================================
 *                parse_subspecies_in_handleType
 * -----------------------------------------------------------
 */
void parse_subspecies_in_handleType(  const Value &subspeciesEnt_,
                        std::vector<std::shared_ptr<AnimActionParam>> &params_,
                        bool isPjtSingle_ ){

    
    std::vector<AnimLabel> labels {}; //- 允许是空的
    size_t      subIdx  {};

    {//--- animLabels ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "animLabels", JsonValType::Array );
        if( a.Size() > 0 ){
            for( auto &ent : a.GetArray() ){//- foreach AnimLabel
                tprAssert( ent.IsString() );
                labels.push_back( str_2_AnimLabel(ent.GetString()) );
            }
        }
    }
    {//--- subIdx ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "subIdx", JsonValType::Uint64 );
        subIdx =  cast_2_size_t(a.GetUint64());
    }
    {//--- AnimActionParams ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "AnimActionParams", JsonValType::Array );
        for( auto &ent : a.GetArray() ){//- foreach AnimActionParam
            afsJson_inn::parse_AnimActionParam( subIdx, ent, params_, labels, isPjtSingle_ );
        }
    }
}


/* ===========================================================
 *                parse_subspecies_in_batchType
 * -----------------------------------------------------------
 */
void parse_subspecies_in_batchType(  const Value &subspeciesEnt_,
                        std::vector<std::shared_ptr<AnimActionParam>> &params_,
                        bool isPjtSingle_ ){
    
    std::vector<AnimLabel> labels {}; //- 允许是空的
    std::string   actionName {};
    NineDirection actionDir {};
    size_t        fstIdx     {};
    size_t        idxNums    {};
    size_t        jFrameIdx  {};
    bool          isOpaque   {};

    {//--- animLabels ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "animLabels", JsonValType::Array );
        if( a.Size() > 0 ){
            for( auto &ent : a.GetArray() ){//- foreach AnimLabel
                tprAssert( ent.IsString() );
                labels.push_back( str_2_AnimLabel(ent.GetString()) );
            }
        }
    }

    {//--- fstIdx ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "fstIdx", JsonValType::Uint64 );
        fstIdx =  cast_2_size_t(a.GetUint64());
    }
    {//--- idxNums ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "idxNums", JsonValType::Uint64 );
        idxNums =  cast_2_size_t(a.GetUint64());
    }
    {//--- name ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "name", JsonValType::String );
        actionName = a.GetString();
    }
    {//--- dir ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "dir", JsonValType::String );
        actionDir = jsonStr_2_ninedirection( a.GetString() );
    }
    {//--- isOpaque ---//
        const auto &a = check_and_get_value( subspeciesEnt_, "isOpaque", JsonValType::Bool );
        isOpaque = a.GetBool();
    }

    for( size_t i=fstIdx; i<fstIdx+idxNums; i++ ){
        isPjtSingle_ ?
            jFrameIdx = 0 :
            jFrameIdx = i;
        params_.push_back( std::make_shared<AnimActionParam>(i, actionName, actionDir, jFrameIdx, i, isOpaque, labels) );
    }

}




/* ===========================================================
 *                    parse_AnimActionParam
 * -----------------------------------------------------------
 */
void parse_AnimActionParam( size_t  subspeciesIdx_,
                            const Value &actionParamEnt_,
                            std::vector<std::shared_ptr<AnimActionParam>> &params_,
                            const std::vector<AnimLabel> &labels_,
                            bool isPjtSingle_ ){

    std::string type {};

    {//--- type ---//
        const auto &a = check_and_get_value( actionParamEnt_, "type", JsonValType::String );
        type = a.GetString();
    }

    if( type == "singleFrame" ){
        params_.push_back( afsJson_inn::singleFrame(subspeciesIdx_, actionParamEnt_, labels_, isPjtSingle_ ) );
    }
    else if( type == "multiFrame" ){
        params_.push_back( afsJson_inn::multiFrame(subspeciesIdx_, actionParamEnt_, labels_) );
    }else{
        tprAssert(0);
    }
}


/* ===========================================================
 *                    singleFrame
 * -----------------------------------------------------------
 */
std::shared_ptr<AnimActionParam> singleFrame(   size_t  subspeciesIdx_,
                                                const Value &actionParamEnt_,
                                                const std::vector<AnimLabel> &labels_,
                                                bool isPjtSingle_ ){

    std::string   actionName {};
    NineDirection actionDir {};
    size_t        jFrameIdx  {};
    size_t        lFrameIdx  {};
    bool          isOpaque   {};
    {//--- actionName ---//
        const auto &a = check_and_get_value( actionParamEnt_, "name", JsonValType::String );
        actionName = a.GetString();
    }
    {//--- dir ---//
        const auto &a = check_and_get_value( actionParamEnt_, "dir", JsonValType::String );
        actionDir = jsonStr_2_ninedirection( a.GetString() );
    }
    {//--- lFrameIdx ---//
        const auto &a = check_and_get_value( actionParamEnt_, "lFrameIdx", JsonValType::Uint64 );
        lFrameIdx = cast_2_size_t(a.GetUint64());
    }
    {//--- isOpaque ---//
        const auto &a = check_and_get_value( actionParamEnt_, "isOpaque", JsonValType::Bool );
        isOpaque = a.GetBool();
    }

    isPjtSingle_ ?
        jFrameIdx = 0 :
        jFrameIdx = lFrameIdx;

    return std::make_shared<AnimActionParam>( subspeciesIdx_, actionName, actionDir, jFrameIdx, lFrameIdx, isOpaque, labels_ );
}


/* ===========================================================
 *                    multiFrame
 * -----------------------------------------------------------
 */
std::shared_ptr<AnimActionParam> multiFrame(size_t  subspeciesIdx_,
                                            const Value &actionParamEnt_, 
                                            const std::vector<AnimLabel> &labels_ ){

    std::string         actionName {};
    NineDirection       actionDir {};
    AnimActionType      actionType {};
    bool                isOrder {};
    bool                isOpaque   {};
    bool                isSeries   {}; //- 是否存储为 连续帧
    bool                isTimeStepSame  {};
    size_t              jFrameIdx  {0}; // tmp, 如果某action 有多个 frame，那么 本值一定为 0 ...
    std::vector<size_t> lFrameIdxs {};
    std::vector<size_t> timeSteps {}; //- only for DiffTimeStep
    size_t              timeStep  {}; //- only for SameTimeStep

    {//--- name ---//
        const auto &a = check_and_get_value( actionParamEnt_, "name", JsonValType::String );
        actionName = a.GetString();
    }
    {//--- dir ---//
        const auto &a = check_and_get_value( actionParamEnt_, "dir", JsonValType::String );
        actionDir = jsonStr_2_ninedirection( a.GetString() );
    }
    {//--- actionType ---//
        const auto &a = check_and_get_value( actionParamEnt_, "actionType", JsonValType::String );
        actionType = str_2_AnimActionType( a.GetString() );
    }
    {//--- isOrder ---//
        const auto &a = check_and_get_value( actionParamEnt_, "isOrder", JsonValType::Bool );
        isOrder = a.GetBool();
    }
    {//--- isOpaque ---//
        const auto &a = check_and_get_value( actionParamEnt_, "isOpaque", JsonValType::Bool );
        isOpaque = a.GetBool();
    }
    {//--- isSeries ---//
        const auto &a = check_and_get_value( actionParamEnt_, "isSeries", JsonValType::Bool );
        isSeries = a.GetBool();
    }
    {//--- isTimeStepSame ---//
        const auto &a = check_and_get_value( actionParamEnt_, "isTimeStepSame", JsonValType::Bool );
        isTimeStepSame = a.GetBool();
    }

    if( isSeries ){//- 动画帧在 png中 连续存储
        size_t fstFrameIdx {};
        size_t frameNums   {};
        {//--- fstFrameIdx ---//
            const auto &a = check_and_get_value( actionParamEnt_, "fstFrameIdx", JsonValType::Uint64 );
            fstFrameIdx = cast_2_size_t(a.GetUint64());
        }
        {//--- frameNums ---//
            const auto &a = check_and_get_value( actionParamEnt_, "frameNums", JsonValType::Uint64 );
            frameNums = cast_2_size_t(a.GetUint64());
        }
        for( size_t i=0; i<frameNums; i++ ){
            lFrameIdxs.push_back( fstFrameIdx + i );
        }

    }else{//- 动画帧在 png中 非连续存储，需要手动收集所有帧 idx
        {//--- lFrameIdxs [] ---//
            const auto &a = check_and_get_value( actionParamEnt_, "lFrameIdxs", JsonValType::Array );
            for( SizeType i=0; i<a.Size(); i++ ){//- foreach lFrameIdx
                lFrameIdxs.push_back( cast_2_size_t(a[i].GetUint64()) );
            }
        }

    }

    if( isTimeStepSame ){
        {//--- timeStep ---//
            const auto &a = check_and_get_value( actionParamEnt_, "timeStep", JsonValType::Uint64 );
            timeStep = cast_2_size_t(a.GetUint64());
        }
        //---
        return std::make_shared<AnimActionParam>(   subspeciesIdx_,
                                                    actionName,
                                                    actionDir,
                                                    actionType,
                                                    isOrder,
                                                    isOpaque,
                                                    jFrameIdx,
                                                    lFrameIdxs,
                                                    timeStep,
                                                    labels_ );
    }else{
        {//--- timeSteps [] ---//
            const auto &a = check_and_get_value( actionParamEnt_, "timeSteps", JsonValType::Array );
            for( SizeType i=0; i<a.Size(); i++ ){//- foreach timeStep
                timeSteps.push_back( cast_2_size_t(a[i].GetUint64()) );
            }
        }
        //---
        return std::make_shared<AnimActionParam>(   subspeciesIdx_,
                                                    actionName,
                                                    actionDir,
                                                    actionType,
                                                    isOrder,
                                                    isOpaque,
                                                    jFrameIdx,
                                                    lFrameIdxs,
                                                    timeSteps,
                                                    labels_ );
    }
}


NineDirection jsonStr_2_ninedirection( const std::string &str_ ){

    if(       str_ == "Mid" ){ return NineDirection::Mid;
    }else if( str_ == "L" ){   return NineDirection::Left;
    }else if( str_ == "LT" ){  return NineDirection::LeftTop;
    }else if( str_ == "T" ){   return NineDirection::Top;
    }else if( str_ == "RT" ){  return NineDirection::RightTop;
    }else if( str_ == "R" ){   return NineDirection::Right;
    }else if( str_ == "RB" ){  return NineDirection::RightBottom;
    }else if( str_ == "B" ){   return NineDirection::Bottom;
    }else if( str_ == "LB" ){  return NineDirection::LeftBottom;
    }else{
        tprAssert(0);
        return NineDirection::Mid; // never reach
    }
}


}//------------- namespace: afsJson_inn end --------------//
}//------------- namespace json: end ----------------