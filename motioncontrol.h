#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QMap>

#include <AIS_InteractiveContext.hxx>
#include <GProp_GProps.hxx>
#include <Bnd_OBB.hxx>
#include <BRepBndLib.hxx>
#include <Geom_Point.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Quantity_Color.hxx>

#include "controlobj.h"
#include "controlaction.h"
#include "controlalgo.h"
#include "processcontrol.h"
#include "modelinitconfig.h"
#include "baseanalysis.h"
#include "configWidget/modelconfig.h"
#include "configWidget/pathconfig.h"
#ifndef PRO
#include "3D_simulation/common.h"
#include "3D_simulation/basedraw.h"
#include "3D_simulation/baseviewwindow.h"
#include "3D_simulation/motioncontrol/pattern/basepattern.h"
#include "3D_simulation/motioncontrol/pattern/gcodepattern.h"
#include "3D_simulation/motioncontrol/pattern/menupattern.h"
#include "3D_simulation/motioncontrol/newModel/newmodel.h"
#else
#include "common.h"
#include "basedraw.h"
#include "baseviewwindow.h"
#include "pattern/basepattern.h"
#include "pattern/gcodepattern.h"
#include "pattern/menupattern.h"
#include "newModel/newmodel.h"
#endif

//#include "processcontrol.h"


#define BLACK Quantity_Color(Quantity_NOC_BLACK)
#define BLUE Quantity_Color(Quantity_NOC_MATRABLUE)
#define BROWN Quantity_Color(Quantity_NOC_BROWN4)
#define FIREBROCK Quantity_Color(Quantity_NOC_FIREBRICK)
#define FOREST Quantity_Color(Quantity_NOC_FORESTGREEN)
#define GRAY Quantity_Color(Quantity_NOC_GRAY70)
#define MYCOLOR Quantity_Color(41.0/255.0, 36.0/255.0, 33.0/255.0, Quantity_TOC_RGB)
#define BEET Quantity_Color(Quantity_NOC_BEET)
#define WHITE Quantity_Color(Quantity_NOC_WHITE)

//enum ClampState{
//    CLAMP = false,
//    UNDO = true
//};

class MotionControl:public QObject
{
    Q_OBJECT
public:
    MotionControl(bool isOsg, QWidget * GLWidget, QObject *parent = 0);
    ~MotionControl();

    void init();
public slots:
    void initModel();

signals:
    void signal_indexForListView(int index);

    void signal_ChangeStateForMainWindow(bool flag);

    void signal_loadModelFinish();

public:
    //! 绑定控制对象
    void bindControlObj(const QString& key, ControlObj *shape);
    //! 获取控制对象
    ControlObj *getControlObj(const QString& key);
    QMap<QString, ControlObj *> *getControlObjs();
    bool controlObjIsExist(const QString& key);

    int translation(const QString& key, gp_Vec dest);

    int bend(const QString& key, double ml,gp_Ax1 ax);
    int rotation(const QString& key, double angle, gp_Ax1 axis, bool copy = false);

    ModelInitConfig *getModelInitConfig() const;
    void setModelInitConfig(ModelInitConfig *modelInitConfig);

    baseViewWindow *getViewer() const;
    void setViewer(baseViewWindow *value);

    int getControlMode() const;

    ModelConfig *getMC() const;
    pathconfig *getPathConfig() const;
    newModel *getNewModel() const;


    void setPattern(basePattern* pattern);
    basePattern *getPattern();

    void setPauseState(bool value);
    //! 校验模型是否完整
    //! 返回值： 模型缺失返回true ,模型完整返回false
    bool modelCheck();

protected:
    void setControlMode(int value);

private:

    void _delete();
    int createPlates(QString fileName);
    void  _createPlates(QString key, TopoDS_Shape shape, gp_Pnt p, bool isDisplay);
public:
    /**
     * @brief createModel
     * @param fileName
     * @param key           map索引值
     */
    int createModel(QString fileName, QString key, QVector3D Color = QVector3D(255,255,255));
    ControlObj *createModel(TopoDS_Shape shape, QString key, QVector3D Color = QVector3D(255,255,255));
    int createModel(QString UP, QString DN, QString key, QVector3D Color = QVector3D(255,255,255));

    void createImpactor(TopoDS_Shape shape,QString key ,gp_Pnt site = gp_Pnt(0,0,0));

private:
    //! 控制对象
    QMap<QString,ControlObj*> mShapeMap;
    ControlObj* model = nullptr;
    //! 控制模式对象
    basePattern *mBasePattern               = nullptr;
    GCodePattern *mGCodePattern             = nullptr;
    MenuPattern *mMenuPattern               = nullptr;

    ModelInitConfig *mModelInitConfig       = nullptr;
    baseViewWindow  *pViewer                = nullptr;
    ModelConfig     *pMC                    = nullptr;
    pathconfig      *mPathConfig            = nullptr;
    newModel        *pNewModel              = nullptr;

    //! 控制模式
    int controlMode                     = NOMEMODE;

};

#endif // MOTIONCONTROL_H
