#ifndef CONTROLOBJ_H
#define CONTROLOBJ_H

#include <TopoDS.hxx>
#include <Bnd_OBB.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ColoredShape.hxx>
#include <BRepBndLib.hxx>
#include <gp_Quaternion.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>

#include <QVector>
#include <QMutex>
#include <QMap>

#include "mydebug.h"
class ControlObj;
enum LAST_MOTION_STATE{
    NONE = 0,
    TRANSLATION,    //! 平移
    BEND,           //! 折弯
    ROTATION        //! 旋转
};

enum MODEL_TYPE{
    NONETYPE = 0,
    PLATES,         //! 板材
    MODEL,          //! 模型
    SLAVE,          //! 从动
    NODE,
    IMPACTOR,       //! 碰撞块
    CUT,            //! 切割出的模型
    BENDANGLE,      //! 折弯角模型
    EAR,            //! 折弯切割出的小块模型
    TEST            //! 测试模型
};
/**
 * 模型对象结构优化
 * 基类   baseModel
 * 基类属性
 *      occ对象
 *      碰撞块map
 *      节点链表
 *      颜色
 *      索引号（预留）
 *      模块标签
 *      类型
 *      状态
 *      原点偏移
 *      绝对原点坐标
 *      相对原点坐标
 *      相对当前坐标
 *      旋转轴
 *      旋转角度(代码正值旋转方向为逆时针，昊折正值旋转方向为顺时针)
 *      移动向量
 *      显示加载标志
 *      显示可见标志
 *      状态变化标志
 *      删除标志
 * 基类函数
 *      旋转
 *      移动
 *
 * 子类  bendModel
 */




class myShape{
public:
    myShape(QString key = " ");
    ~myShape();


    static void setlock(bool state, QString file, QString fun, int line);
    static int getCounter();

private:
    //! OCCT obj
    Handle(AIS_ColoredShape) hAisShape;
    TopoDS_Shape Shape;
    //! 碰撞检测对象
    Bnd_OBB OBB;
    //! 切片（折弯角的基本面）
    TopoDS_Shape  theSection;
    //! 显示颜色
    Quantity_Color theColor;
    //! 从动对象 前 左 后 右
    QList<myShape*> lpSlave;
    //! 显示数据索引号
    int iIndex                          = -1;
    //! 标签
    QString sKey;
    //! 类型
    int iModelType                      = NONETYPE;
    //! 是否压边
    bool bIsBlankPressing               = false;
    //! 运动状态 (true 发生运动，false 没有发生运动)
    bool bStateOfMotion                 = false;
    //! 最后一次运动
    int iLastState                      = NONE;
    //! 跟随折弯边的索引
    int iFollowIndex                    = -1;
    //! 折弯边索引
    int iFrontIndex                     = -1;


    //! 公共属性
    //! 原点偏移
    gp_Vec vecOriginPositionOffset      = gp_Vec(0.0, 0.0 ,0.0);
    //! 绝对原点坐标
    gp_Vec vecAbsoluteOriginPosition    = gp_Vec(0.0, 0.0 ,0.0);
    //! 相对原点坐标
    gp_Vec vecRelativeOriginPosition    = gp_Vec(0.0, 0.0 ,0.0);
    //! 相对当前坐标
    gp_Vec vecRelativeCurrentPosition   = gp_Vec(0.0, 0.0 ,0.0);
    //! 旋转角度(代码正值旋转方向为逆时针，昊折正值旋转方向为顺时针)
    double dRotateAngle                 = 0.0;
    //! 移动向量
    gp_Vec vecTranslation               = gp_Vec(0,0,0);


    //! 板材专用属性
    //! model
    //! 宽（与刀平行边的长度）
    double dWidth                       = 0.0;
    //! 长（不与刀平行边的长度）
    double dHeight                      = 0.0;
    //! 板厚
    double dThickness                   = 0.0;
    //! 边界到中心 起始边为要折弯的边,各边刀中心的距离（即长/2，宽/2）
    //! 顺序         0
    //!         1        3
    //!             2
    QList<double> ldBoundaryToCenter;
    //! 折弯角度 前 左 后 右
    double dBendAngle                   = 0.0;
    //! 折弯时坐标位置
    double dBendSite                    = 0.0;
    //! 初始折弯角度(未变换时存在的折弯角度)
    double dInitAngle                   = 0.0;
    //! 折弯轴（旋转轴）
    gp_Ax1 BendAxis                     = gp_Ax1();
    //! 当前边为折弯边
    bool bIsFront                       = false;
    //! 标志角度(需要旋转多少度到这个边)
    double dMarkAngle                   = 0.0;
    //! 复折圆心偏移
    double dDFoldOffset                 = 0.0;

    //! 显示属性(状态)
    //! 显示重载
    bool bIsRedisplay                   = false;
    //! 是否可见
    bool bIsVisible                     = false;
    //! 是否发生变换
    bool bHasTransform                  = false;
    //! 删除模型
    bool bDelModel                      = false;



    //! 压刀模型参数
    // 路径
    QString Path;
    //! 模型位置
    // 模型左位置
    gp_Vec leftSite = gp_Vec(0,0,0);
    // 模型右位置
    gp_Vec rightSite = gp_Vec(0,0,0);
    // 类型
    int knifeType = 0;


public:
    void initData();

    void setAisShape(Handle_AIS_ColoredShape AisShape);
    void setSection(const TopoDS_Shape &aSection);
    void setColor(const Quantity_Color &aColor);
//    void setViewer(baseViewWindow* viewer);
    void setIndex(int index);
    void setKey(QString key);
    void setModelType(MODEL_TYPE ModelType);
    void setIsBlankPressing(bool flag);
    void setStateOfMotion(bool state);
    void setLastState(LAST_MOTION_STATE state);
    void setFollowIndex(int Index);
    void setOriginPositionOffset(gp_Vec OriginPositionOffset);
    void setAbsoluteOriginPosition(gp_Vec AbsoluteOriginPosition);
    void setRelativeOriginPosition(gp_Vec RelativeOriginPosition);
    void setRelativeCurrentPosition(gp_Vec RelativeCurrentPosition);
    void setRelativeCurrentPositionForBatch(gp_Vec RelativeCurrentPosition);
    void setBendAngle(double BendAngle);
    void setRotateAngle(double RotateAngle);
    void setThickness(double Thickness);
    void setWidth(double width);
    void setHeight(double height);
    void setBoundaryToCenter(QList<double> BoundaryToCenter);
    void setBoundaryToCenter(int index, double BoundaryToCenter);
    void setBendSite(double BendSite);
    void setIsFront(bool is);
    void setMarkAngle(double ang);
    void setDFoldOffset(double offset);
    void setFrontIndex(int index);
    void setOBB(TopoDS_Shape shape);
    void setOBB(Bnd_Box obb);
    void setOBB(const gp_Pnt& theCenter, const gp_Dir& theXDirection, const Standard_Real theHXSize
                , const gp_Dir& theYDirection, const Standard_Real theHYSize
                , const gp_Dir& theZDirection, const Standard_Real theHZSize);

    //! 设置显示属性(状态)
    void setIsRedisplay(bool state, bool onlyOne = false);
    void setIsVisible(bool state, bool onlyOne = false);
    void setHasTransform(bool state);
    void setDelModel(bool state);
    void setInitAngle(double ang);
    void setBendAxis(gp_Ax1 ax);


    Handle_AIS_ColoredShape getAisShape();
    const TopoDS_Shape &getSection() const;
    const Quantity_Color &getColor() const;
//    baseViewWindow* getViewer();
    int getIndex() const;
    QString getKey() const;
    int getModelType();
    bool getIsBlankPressing();
    bool getStateOfMotion() const;
    int getLastState() const;
    int getFollowIndex() const;
    gp_Vec getOriginPositionOffset() const;
    gp_Vec getAbsoluteOriginPosition() const;
    gp_Vec getRelativeOriginPosition() const;
    gp_Vec getRelativeCurrentPosition() const;
    double getBendAngle() const;
    double getRotateAngle() const;
    double getThickness() const;
    double getWidth() const;
    double getHeight() const;
    QList<double> getBoundaryToCenter() const;
    double getBoundaryToCenter(int index) const;
    double getBendSite() const;
    bool getIsFront() const;
    double getMarkAngle() const;
    double getDFoldOffset() const;
    int getFrontIndex() const;
    Bnd_OBB getOBB() const;

    //! 获取显示属性(状态)
    bool getIsRedisplay();
    bool getIsVisible();
    bool getHasTransform();
    bool getDelModel();
    double getInitAngle() const;
    gp_Ax1 getBendAxis() const;

    //! 清除Ais_Shape的内存，不能使用getAisShape().Nullify();清除内存
    void AisShapeNullify();
    void clearOBB();


    void addSlave(myShape* Slave);
    void modifySlave(int index,myShape* Slave);
    void removeSlave(int index);
    void removeSlave(myShape* Slave);
    myShape* getSlave(int index);
    myShape* getSlave(MODEL_TYPE type);
    QList<myShape *> getSlaveList();
    int getSlaveNum();
    myShape *getSlaveForCurrentFront();
    bool hasModelTypeSlave(MODEL_TYPE ModelType);
    void debugArg();

    void cleanSlave();



    //! 移动
    int translation(gp_Vec destPnt, bool transEar = true, const Standard_Boolean Copy = Standard_False, bool isDiaplay = false);
    //! 旋转
    int rotation(double angle, gp_Ax1 axis, const Standard_Boolean Copy = Standard_False);
    //! 折弯
    int bend(double angle, gp_Ax1 axis);

    //! 碰撞检查
    myShape* collisionDetection(Bnd_OBB s1, gp_Ax1 ax, QMap<QString, ControlObj *> *shapeMap);

    TopoDS_Shape getShape() const;
    void setShape(const TopoDS_Shape &value);

    QString getPath() const;
    void setPath(const QString &value);

    gp_Vec getLeftSite() const;
    void setLeftSite(const gp_Vec &value);

    gp_Vec getRightSite() const;
    void setRightSite(const gp_Vec &value);

    int getKnifeType() const;
    void setKnifeType(int value);

private:
    //! 移动
    int _translation(gp_Vec destPnt, const Standard_Boolean Copy, bool isDiaplay);
    //! 旋转
    int _rotation(double angle, gp_Ax1 axis, const Standard_Boolean Copy);
    int _bend(double angle, gp_Ax1 axis);
    //! 碰撞检查
    myShape* _collisionDetection(Bnd_OBB s1, gp_Ax1 ax, QMap<QString, ControlObj *> *shapeMap);

};

class ControlObj
{
public:
    ControlObj(const QString& key = " ");
    ~ControlObj();

    void setMainShape(const TopoDS_Shape& shape, const QString& key, gp_Vec AOP, gp_Vec ROP, gp_Vec RCP, double BA = 0);
    void setMainShape(Handle(AIS_ColoredShape) shape, const QString& key, gp_Vec AOP, gp_Vec ROP, gp_Vec RCP, double BA = 0);
    void setMainShape(myShape *shape);

    myShape *getMainShape() const;


private:
    myShape* mMainShape = nullptr;
};

#endif // CONTROLOBJ_H
