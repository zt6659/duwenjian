#ifndef CONTROLACTION_H
#define CONTROLACTION_H

#include <QObject>
#include <QTime>

#include <AIS_InteractiveContext.hxx>
#include <gp_Trsf.hxx>
#include <BRepGProp.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <GProp_GProps.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BOPAlgo_MakerVolume.hxx>
//#include <./OCCSoundCode/BOPAlgo_MakerVolume.h>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BOPAlgo_Splitter.hxx>
#include <BOPAlgo_Tools.hxx>

#ifndef PRO
#include "3D_simulation/motioncontrol/controlobj.h"
#include "3D_simulation/transverter.h"
#include "3D_simulation/baseviewwindow.h"
#else
#include "motioncontrol/controlobj.h"
#include "transverter.h"
#include "baseviewwindow.h"
#endif


class ControlAction
{


public:
    ControlAction();
    //! Key 操作模型的键值
    //! shape 操作模型的数据对象
    //! AisContext 显示上下文对象
    ControlAction(ControlObj *shape);
    ~ControlAction();

    //! 初始化
    void init(ControlObj *shape);
    //! 折弯
    //! angle   折弯角度
    //! ax      折弯角的旋转轴
    //! pln     模型的切割平面
    int bend(double angle, gp_Ax1 ax, gp_Pln pln = gp_Pln(gp_Pnt(-0.0001,0.,0.),gp_Dir(1,0,0)), baseViewWindow *view= nullptr );
    //! 压边折弯
    int pressingBend(double angle, gp_Ax1 ax, gp_Vec site = gp_Vec(0, 0, 0), bool flag = false, baseViewWindow *view= nullptr);

    //! 移动
    //! vec 目标点位
    //! mode 临时动作模式 -1为默认模式
    int translation(gp_Vec dest);
    //! 旋转
    int rotation(double angle, gp_Ax1 axis,bool copy);

    //! 生成折弯角
    int createBendAng(myShape* myshape, double radian, gp_Ax1 ax);
private:
    //! 移动
    //! vec 位移向量
    int _translation(TopoDS_Shape &shape, gp_Vec vec);
    //! 折弯
    //! 外边折弯半径 板厚*2
    //! IsRevol 是否生成折弯角 由面旋转
    TopoDS_Shape _bendAngle(const TopoDS_Shape &shape, double radian, gp_Ax1 ax);
    //! 旋转
    int _rotation(TopoDS_Shape& shape, double radian, gp_Ax1 axis);

private:
    bool mInit           = false;

    ControlObj *mControlObj;
};

#endif // CONTROLACTION_H
