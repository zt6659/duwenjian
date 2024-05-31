#ifndef MODELINITCONFIG_H
#define MODELINITCONFIG_H

#include <QObject>
#include <AIS_Shape.hxx>
#include "../baseviewwindow.h"

enum CONTROLMODE{
    NOMEMODE = -1,
    MENUMODE,           //! 菜单模式
    INSTRUCTIONMODE     //! 指令模式
};

class ModelInitConfig
{
public:
    ModelInitConfig();
    ~ModelInitConfig();

    void init();

private:
    //! 文件路径
    QString path;
    //! 是否翻面
    bool isOverturn                     = false;
    //! 旋转角度(代码旋转方向为逆时针，昊折旋转方向为顺时针)
    double rotationAng                  = 0.0;
    //! 宽（与刀平行边的长度）
    double width                        = 0.0;
    //! 长（不与刀平行边的长度）
    double height                       = 0.0;
    //! 厚度
    double thickness                    = 0.0;
    //! 边数
    int edgeNum                         = 4;
    //! 起始边为要折弯的边,各边刀中心的距离（即长/2，宽/2.按顺时针顺序填）
    QList<double> ldBoundaryToCenter;
    //! 中心点
    gp_Pnt center;
    //! 控制模式
    int controlMode                     = NOMEMODE;

public:
    bool getIsOverturn() const;
    void setIsOverturn(bool value);
    double getRotationAng() const;
    void setRotationAng(double value);
    double getWidth() const;
    void setWidth(double value);
    double getHeight() const;
    void setHeight(double value);
    double getThickness() const;
    void setThickness(double value);
    int getEdgeNume() const;
    void setEdgeNume(int value);
    QList<double> getBoundaryToCenter() const;
    void setBoundaryToCenter(const QList<double> &value);
    TopoDS_Shape getShape() const;
    void setShape(TopoDS_Shape value);
    int getIndex() const;
    void setIndex(int value);
    gp_Pnt getCenter() const;
    void setCenter(const gp_Pnt &value);
    QString getPath() const;
    void setPath(const QString &value);
    void clearPath();
    int getControlMode() const;
    void setControlMode(int value);
};

#endif // MODELINITCONFIG_H
