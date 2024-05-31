#include "controlalgo.h"
#include "mydebug.h"

ControlAlgo::ControlAlgo()
{

}

ControlAlgo::~ControlAlgo()
{

}

//! point   圆外切线上的点
//! center  圆心
//! m       圆心和圆外切线上点的线段和经过切点的垂线的交点
//!

bool ControlAlgo::calcPointcut2D(gp_Pnt center, double radius, gp_Pnt point, QVector<gp_Pnt> &pointcut)
{
    gp_Pnt2d  center2D(center.X(),center.Z());
    gp_Pnt2d  point2D(point.X(),point.Z());
    gp_Vec2d p2c(point2D,center2D);

    double p2cDis = p2c.Magnitude();

    gp_Vec2d p2cNormalized = p2c.Normalized();
    if(ISDEBUG) DEBUG()<<" p2cNormalized "<<p2cNormalized.X()<<" "<<p2cNormalized.Y();

    double p2mDis,m2cDis;

    m2cDis = radius*radius/p2cDis;

    gp_Vec2d p2m = p2c-p2cNormalized*m2cDis;
    if(ISDEBUG) DEBUG()<<"m2nDis cal err"<<radius*radius - m2cDis*m2cDis;
    if(radius*radius - m2cDis*m2cDis < 0){
        DEBUG()<<"m2nDis cal err: "<<radius*radius - m2cDis*m2cDis<<"radius"<<radius<<"m2cDis"<<m2cDis;
        return 1;
    }

    double m2nDis = Sqrt(radius*radius - m2cDis*m2cDis);

    gp_Vec2d m2nNormalized1(-p2cNormalized.Y(),p2cNormalized.X());
    gp_Vec2d m2nNormalized2(p2cNormalized.Y(),-p2cNormalized.X());

    if(ISDEBUG) DEBUG()<<" m2nNormalized1 "<<m2nNormalized1.X()<<" "<<m2nNormalized1.Y();
    if(ISDEBUG) DEBUG()<<" m2nNormalized2 "<<m2nNormalized2.X()<<" "<<m2nNormalized2.Y();

    gp_Pnt2d mP(p2m.X()+point2D.X(),p2m.Y()+point2D.Y());

    if(ISDEBUG) DEBUG()<<" mP "<<mP.X()<<" "<<mP.Y();

    gp_Pnt2d m2n1(mP.X()+(m2nNormalized1*m2nDis).X(),mP.Y()+(m2nNormalized1*m2nDis).Y());
    gp_Pnt2d m2n2(mP.X()+(m2nNormalized2*m2nDis).X(),mP.Y()+(m2nNormalized2*m2nDis).Y());

    pointcut.append(gp_Pnt(m2n1.X(),0,m2n1.Y()));
    pointcut.append(gp_Pnt(m2n2.X(),0,m2n2.Y()));
    return 0;
}

double ControlAlgo::deviationAngle2D(gp_Pnt centerPoint, gp_Pnt relativePoint, int bendDir)
{
    if(ISDEBUG) DEBUG()<<" ---------- ";
    gp_Pnt2d  center2D(centerPoint.X(),centerPoint.Z());
    gp_Pnt2d  point2D((relativePoint.X()),relativePoint.Z());
    if(ISDEBUG) DEBUG()<<point2D.X()<<point2D.Y()<<center2D.X()<<center2D.Y();
    double ang = 0;
    double k = 0;
    if(qAbs(center2D.X() - point2D.X()) < 1e-6)
    {
        ang = 90;
    }else{
        k = -(point2D.Y()-center2D.Y())/(point2D.X()-center2D.X());
        ang = atan(k);
    }
    if(ISDEBUG) DEBUG()<<" ang "<< ang;
    if(bendDir == 1){
        if(qAbs(center2D.X() - point2D.X()) < 1e-6) return ang;
    }else if(bendDir == -1){
        if(qAbs(center2D.X() - point2D.X()) < 1e-6) return -ang;
    }

    if(point2D.X() < center2D.X() && point2D.Y() > center2D.Y()){
        // 第一象限
        if(ISDEBUG) DEBUG()<<"第一象限"<<ang/M_PI*180;
        return ang/M_PI*180;
    }else if(point2D.X() > center2D.X() && point2D.Y() > center2D.Y()){
        // 第二象限
        if(ISDEBUG) DEBUG()<<"第二象限"<<(ang)/M_PI*180+180;
        return (ang)/M_PI*180+180;
    }else if(point2D.X() < center2D.X() && point2D.Y() < center2D.Y()){
        // 第三象限
        if(ISDEBUG) DEBUG()<<"第三象限"<<ang/M_PI*180;
        return ang/M_PI*180;
    }else if(point2D.X() > center2D.X() && point2D.Y() < center2D.Y()){
        // 第四象限
        if(ISDEBUG) DEBUG()<<"第四象限"<<(ang)/M_PI*180-180;
        return (ang)/M_PI*180-180;
    }

    return 0;
}


