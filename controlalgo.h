#ifndef CONTROLALGO_H
#define CONTROLALGO_H

#include <math.h>

#include <QObject>
#include <QTime>

#include <TopoDS.hxx>
#include <Bnd_OBB.hxx>
#include <BRepBndLib.hxx>
#include <GProp_GProps.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <AIS_ColoredShape.hxx>
#include <AIS_InteractiveContext.hxx>
class ControlAlgo
{
public:
    ControlAlgo();
    ~ControlAlgo();


    //! 计算圆外切点
    //! center  圆心
    //! radius  半径
    //! point   切线经过的圆外点
    //! pointcut     输出的切点
    bool calcPointcut2D(gp_Pnt center, double radius, gp_Pnt point, QVector<gp_Pnt>& pointcut);

    //! 两点间的相对偏移角
    //! centerPoint 中心点
    //! relativePoint 相对点
    //! return 角度
    double deviationAngle2D(gp_Pnt centerPoint, gp_Pnt relativePoint, int bendDir);


};

#endif // CONTROLALGO_H
