#include "modelinitconfig.h"

ModelInitConfig::ModelInitConfig()
{

}

ModelInitConfig::~ModelInitConfig()
{

}

void ModelInitConfig::init()
{
    isOverturn                   = false;
    rotationAng                  = 0.0;
    width                        = 0.0;
    height                       = 0.0;
    thickness                    = 0.0;
    edgeNum                      = 4;
    ldBoundaryToCenter.clear();
}

int ModelInitConfig::getControlMode() const
{
//    INFO()<<" ModelInitConfig::getControlMode "<<controlMode;
    return controlMode;
}

void ModelInitConfig::setControlMode(int value)
{
    INFO()<<" ModelInitConfig::setControlMode "<<controlMode <<"->" <<value;
    controlMode = value;
}

QString ModelInitConfig::getPath() const
{
    return path;
}

void ModelInitConfig::setPath(const QString &value)
{
    INFO()<<"set path is "<<value;
    path = value;
}

void ModelInitConfig::clearPath()
{
    path.clear();
}

gp_Pnt ModelInitConfig::getCenter() const
{
    return center;
}

void ModelInitConfig::setCenter(const gp_Pnt &value)
{
    center = value;
}

QList<double> ModelInitConfig::getBoundaryToCenter() const
{
    return ldBoundaryToCenter;
}

void ModelInitConfig::setBoundaryToCenter(const QList<double> &value)
{
    ldBoundaryToCenter = value;
}

int ModelInitConfig::getEdgeNume() const
{
    return edgeNum;
}

void ModelInitConfig::setEdgeNume(int value)
{
    edgeNum = value;
}

double ModelInitConfig::getThickness() const
{
    return thickness;
}

void ModelInitConfig::setThickness(double value)
{
    thickness = value;
}

double ModelInitConfig::getHeight() const
{
    return height;
}

void ModelInitConfig::setHeight(double value)
{
    height = value;
}

double ModelInitConfig::getWidth() const
{
    return width;
}

void ModelInitConfig::setWidth(double value)
{
    width = value;
}

double ModelInitConfig::getRotationAng() const
{
    return rotationAng;
}

void ModelInitConfig::setRotationAng(double value)
{
    rotationAng = value;
}

bool ModelInitConfig::getIsOverturn() const
{
    return isOverturn;
}

void ModelInitConfig::setIsOverturn(bool value)
{
    isOverturn = value;
}
