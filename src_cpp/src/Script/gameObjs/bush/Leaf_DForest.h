/*
 * ========================= Leaf_DForest.h ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.04.11
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 *   2~3 片竖立的叶片 bush，DForest
 * ----------------------------
 */
#ifndef TPR_LEAF_DARK_FOREST_H_
#define TPR_LEAF_DARK_FOREST_H_

//-------------------- CPP --------------------//
#include <string>
#include <vector>
#include <functional>

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "GameObj.h"
#include "GameObjMesh.h"
#include "AnimFrameSet.h"
#include "PubBinaryValType.h"
#include "MapAltitude.h"
#include "Density.h"


namespace gameObjs{//------------- namespace gameObjs ----------------


//-- 定义了 go.binary 的数据格式 --
inline std::vector<PubBinaryValType> leaf_DForest_pubBinaryValTypes {
    PubBinaryValType::HP,
    PubBinaryValType::MP
};

struct Leaf_DForest_PvtBinary{
    size_t   leaf_DForestId {0};
            //- 简单的从 几种款式中，随机挑选一款 [0,7]
    //===== padding =====//
    //...
};


class Leaf_DForest{
public:
    Leaf_DForest() = default;

    //--- 延迟init ---//
    void init_in_autoMod(   GameObj &goRef_,
                            const IntVec2 &mpos_,
					        float fieldWeight_,
					        const MapAltitude &alti_,
					        const Density &_density );
                            
    void bind( GameObj &goRef_ );

    //--- 从硬盘读取到 go实例数据后，重bind callback
    void rebind( GameObj &goRef_ );

    //--- callback ---//
    void OnRenderUpdate( GameObj &goRef_ ); 
    void OnLogicUpdate( GameObj &goRef_ ); 


    //--  每次调用回调函数，都需要做的 指针重绑定 --
    inline Leaf_DForest_PvtBinary *rebind_ptr( GameObj &goRef_ ){

        tprAssert( goRef_.species == Leaf_DForest::specId );
        return reinterpret_cast<Leaf_DForest_PvtBinary*>(goRef_.get_pvtBinaryPtr());
    }


    //======== static ========//
    static  u32_t  specId; //- 在 onGoSpecIds_SignUp() 中手动设置...


private:
    //--- callback ---//
    void OnActionSwitch( GameObj &goRef_, ActionSwitchType type_ );

};

//---------- static ----------//
inline u32_t  Leaf_DForest::specId {0}; //- 具体值在 goSpecIds.cpp 中分配

//=====< Leaf_DForest类 唯一的 保留实例 >=====
inline Leaf_DForest  leaf_DForest {};


}//------------- namespace gameObjs: end ----------------
#endif 

