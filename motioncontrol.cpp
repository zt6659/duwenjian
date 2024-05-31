#include "motioncontrol.h"
#ifndef PRO
#include "3D_simulation/occwindow.h"
#else
#include "occwindow.h"
#endif
#include "mydebug.h"
#ifndef PRO
#include "3D_simulation/global_info.h"
#else
#include "global_info.h"
#endif
#include <QThread>

#include <Prs3d_Drawer.hxx>
#include <BRepTools.hxx>
#include <TopTools.hxx>
MotionControl::MotionControl(bool isOsg, QWidget *GLWidget, QObject *parent) : QObject(parent)
{

        pViewer = new occWindow(GLWidget);
        pViewer->setShapeMap(getControlObjs());


//return;
#ifdef PRO
    init();
#endif
    INFO()<<" create MotionControl object ";
}

MotionControl::~MotionControl()
{
    _delete();
    if(pViewer)
        delete pViewer;
    if(pMC){
        pMC->close();
        delete pMC;
    }
    if(mModelInitConfig)
        delete mModelInitConfig;
    if(mPathConfig)
        delete mPathConfig;
    if(mGCodePattern)
        delete mGCodePattern;
    if(mMenuPattern)
        delete mMenuPattern;
    if(pNewModel)
        delete pNewModel;

}

void MotionControl::init()
{
    if(mModelInitConfig == nullptr){
        mModelInitConfig = new ModelInitConfig();
    }

    if(mPathConfig == nullptr)
    {
        mPathConfig = new pathconfig();
    }

    if(pMC == nullptr){
        pMC = new ModelConfig(nullptr);
        pMC->setShapeMap(&mShapeMap);
        pMC->setModelInitConfig(mModelInitConfig);
        connect(pMC,SIGNAL(signal_configFinish()),this,SLOT(initModel()));
    }
    if(pNewModel == nullptr){
        pNewModel = new newModel(nullptr);
        pNewModel->setShapeMap(&mShapeMap);
        pNewModel->setModelInitConfig(mModelInitConfig);
        connect(pNewModel,SIGNAL(signal_initModel()),this,SLOT(initModel()));
        if(pMC != nullptr)
            connect(pNewModel,SIGNAL(signal_setConfitText(QString)),pMC,SLOT(slot_setConfigText(QString)));
    }



    mGCodePattern = new GCodePattern("gcode");
    mGCodePattern->setView(pViewer);
    mGCodePattern->setShapeMap(getControlObjs());
    mGCodePattern->setModelInitConfig(mModelInitConfig);
    mGCodePattern->setView(pViewer);
    mMenuPattern = new MenuPattern("menu");
    mMenuPattern->setShapeMap(getControlObjs());
    mMenuPattern->setModelInitConfig(mModelInitConfig);

    AsyncTask *pLoadmodelTask       = new AsyncTask();
    pLoadmodelTask->setTaskRunnable([=]{

    int ret = 0;

    ret = createModel(mPathConfig->getUpKnifePath(),mPathConfig->getDnKnifePath(),"XZ", QVector3D(150,150,150));

    if(ret != 0) {CRITICAL()<<"createModel XZ failed";
        emit signal_loadModelFinish(); return;}
    ret = createModel(mPathConfig->getDnPressKnifePath(),"DW", QVector3D(220,230,240));
    if(ret != 0) {CRITICAL()<<"createModel dd failed";
        emit signal_loadModelFinish(); return;}
    if(get_sys_seriestype_config(KEYSYSCOMMON_SERIESTYPE_ID) == SERIESTYPE_X){
        ret = createModel(mPathConfig->getSuckerPath(),"UC", QVector3D(255,255,255));
        if(ret != 0) {CRITICAL()<<"createModel UC failed";
            emit signal_loadModelFinish(); return;}
    }else{
        ret = createModel(mPathConfig->getDnPressArmPath(),"UC", QVector3D(255,255,255));
        if(ret != 0) {CRITICAL()<<"createModel UC failed";
            emit signal_loadModelFinish(); return;}
        ret = createModel(mPathConfig->getUpPressArmPath(),"1W", QVector3D(255,255,255));
        if(ret != 0) {CRITICAL()<<"createModel 1W failed";
            emit signal_loadModelFinish(); return;}
    }
    ret = createModel(mPathConfig->getUpPressKnifePath(),"W", QVector3D(255,255,255));
    if(ret != 0) {CRITICAL()<<"createModel W failed";
        emit signal_loadModelFinish(); return;}

    TopoDS_Shape shape = BRepPrimAPI_MakeBox(gp_Pnt(0,-getSize()/2.0,0),75,getSize(),4);
    myShape::setlock(true, __FILE__, __FUNCTION__, __LINE__);
    ControlObj * obj = getControlObj("W");
    if(obj)
        obj->getMainShape()->setOBB(shape);
    myShape::setlock(false, __FILE__, __FUNCTION__, __LINE__);

//    TopoDS_Shape Cylinder = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(-getDnKnifeRadius(),-(getSize()+1000)/2.0,-getDnKnifeRadius()),gp::DY()),sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius())/2.0, (getSize()+1000) );
    TopoDS_Shape Cylinder = BRepPrimAPI_MakeBox(gp_Pnt(-getDnKnifeRadius()-sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius())/2.0,-(getSize()+1000)/2.0,-getDnKnifeRadius()-sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius())/2.0),sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius()),getSize()+1000,sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius()));
    createImpactor(Cylinder,"XZImpDown",gp_Pnt(0,0,0));

//    Cylinder = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(getHCenterDistance()-getUpKnifeRadius(),-(getSize()+1000)/2.0,(getVCenterDistance()+getUpKnifeRadius())),gp::DY()),sqrt(2.0*getUpKnifeRadius()*getUpKnifeRadius())/2.0, (getSize()+1000) );
    Cylinder = BRepPrimAPI_MakeBox(gp_Pnt(getHCenterDistance()-getUpKnifeRadius()-sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius())/2.0,-(getSize()+1000)/2.0,(getVCenterDistance()+getUpKnifeRadius())-sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius())/2.0),sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius()),getSize()+1000,sqrt(2.0*getDnKnifeRadius()*getDnKnifeRadius()));
    createImpactor(Cylinder,"XZImpUp",gp_Pnt(0,0,0));

    INFO()<<"load finished!";
    QThread::sleep(1);
    emit signal_loadModelFinish();
    });

    pLoadmodelTask->execute();
}

void MotionControl::initModel()
{
    INFO()<<" enty ";
    if(mModelInitConfig == nullptr) return;

    QString fileName = mModelInitConfig->getPath();
    if(ISDEBUG) DEBUG()<<" 计时 ";

    if(getPattern() != nullptr)
        getPattern()->init();

    myShape::setlock(true, __FILE__, __FUNCTION__, __LINE__);
    if(controlObjIsExist("model")){

        getControlObjs()->value("model")->getMainShape()->setDelModel(true);
        getControlObjs()->value("model")->getMainShape()->setIsVisible(false);
        getControlObjs()->value("model")->getMainShape()->setKey("modelDel");
        getControlObjs()->insert("modelDel",getControlObjs()->value("model"));
        getControlObjs()->remove("model");
    }
    if(controlObjIsExist("UC"))
    {
        double ang = 0 - getControlObj("UC")->getMainShape()->getRotateAngle();
        QVector3D dir = QVector3D(0,0,1);
        gp_Ax1 axis(gp_Pnt(getControlObj("UC")->getMainShape()->getRelativeCurrentPosition().X(),
                           getControlObj("UC")->getMainShape()->getRelativeCurrentPosition().Y(),
                           0),
                    gp_Dir(dir.x(), dir.y(), dir.z()));

        getControlObj("UC")->getMainShape()->rotation(ang,axis);
        if(controlObjIsExist("1W")){
            getControlObj("1W")->getMainShape()->rotation(ang,axis);
        }
    }

    DEBUG()<<" controlMode "<<controlMode<<"mModelInitConfig->getControlMode"<<mModelInitConfig->getControlMode();
    if(controlMode != mModelInitConfig->getControlMode() && !modelCheck()){
        setControlMode(mModelInitConfig->getControlMode());

        ControlObj * obj = nullptr;

        if(controlMode == INSTRUCTIONMODE && !modelCheck()){

            setPattern(mGCodePattern);

            if(ISDEBUG) DEBUG()<<" 计时 XZ ";
            obj = getControlObj("XZ");
            if(obj)
                obj->getMainShape()->setIsVisible(true);
            if(ISDEBUG) DEBUG()<<" 计时 dd ";
            obj = getControlObj("DW");
            if(obj)
                obj->getMainShape()->setIsVisible(true);
            if(ISDEBUG) DEBUG()<<" 计时 UC ";
            obj = getControlObj("UC");
            if(obj)
                obj->getMainShape()->setIsVisible(true);
            obj = getControlObj("1W");
            if(obj)
                obj->getMainShape()->setIsVisible(true);
            if(ISDEBUG) DEBUG()<<" 计时 W";
            obj = getControlObj("W");
            if(obj)
                obj->getMainShape()->setIsVisible(true);
        }else if(controlMode == MENUMODE && !modelCheck()){
            setPattern(mMenuPattern);

            obj = getControlObj("XZ");
            if(obj)
                obj->getMainShape()->setIsVisible(false);
            obj = getControlObj("DW");
            if(obj)
                obj->getMainShape()->setIsVisible(false);
            obj = getControlObj("UC");
            if(obj)
                obj->getMainShape()->setIsVisible(false);
            obj = getControlObj("1W");
            if(obj)
                obj->getMainShape()->setIsVisible(false);
            obj = getControlObj("W");
            if(obj)
                obj->getMainShape()->setIsVisible(false);
        }
    }

    myShape::setlock(false, __FILE__, __FUNCTION__, __LINE__);
    if(!ISDEBUG) DEBUG()<<" 计时 ";

    if(mModelInitConfig->getPath().isEmpty()) {
        WARNING()<<"the path is empty!";
        return;
    }


    if(createPlates(fileName) != 0)
        return;
    if(ISDEBUG) DEBUG()<<" 计时 ";

//    if(qAbs( get_u_height() - mModelInitConfig->getHeight()) > 0.5 ||
//             qAbs(get_v_width() - mModelInitConfig->getWidth()) > 0.5 ){
//        QMessageBox::warning(nullptr, "温馨提示.",
//                             QString("模型尺寸异常，请重新加载模型，否者影响显示效果！ 模型长度：%1，宽度：%2，厚度：%3。实际输入长度：%4，宽度：%5，厚度：%6")
//                             .arg(get_u_height()).arg(get_v_width()).arg(get_uv_thickness())
//                             .arg(mModelInitConfig->getHeight()).arg(mModelInitConfig->getWidth()).arg(mModelInitConfig->getThickness()),
//                             QMessageBox::Ok);
//    }
}

void MotionControl::bindControlObj(const QString &key, ControlObj *shape)
{
    DEBUG()<<key
          <<shape->getMainShape()->getAbsoluteOriginPosition().X()<<" "
          <<shape->getMainShape()->getAbsoluteOriginPosition().Y()<<" "
          <<shape->getMainShape()->getAbsoluteOriginPosition().Z()<<" ";
//    myShape::setlock(true, " bindControlObj ", __LINE__);
    mShapeMap.insert(key,shape);
//    myShape::setlock(false, " bindControlObj ", __LINE__);
}

ControlObj *MotionControl::getControlObj(const QString &key)
{
    if(mShapeMap.contains(key))
        return mShapeMap.value(key);
    else
        return nullptr;
}

QMap<QString,ControlObj*> *MotionControl::getControlObjs()
{
    return &mShapeMap;
}

bool MotionControl::controlObjIsExist(const QString &key)
{
    return mShapeMap.contains(key);
}

int MotionControl::translation(const QString& key, gp_Vec dest)
{
    if(getControlObj(key) == nullptr)
    {
        DEBUG()<<"getControlObj("<<key<<") get is nullptr  ";
        return -1;
    }

    ControlAction ca(getControlObj(key));
    ca.translation(dest);
    return 0;
}

int MotionControl::bend(const QString& key, double ml, gp_Ax1 ax)
{
    if(getControlObj(key) == nullptr)
    {
        DEBUG()<<"getControlObj("<<key<<") get is nullptr  ";
        return -1;
    }
    ControlAction ca(getControlObj(key));
    ca.bend(ml,ax,gp_Pln(gp_Pnt(/*-0.0001*/0,0.,0.),gp_Dir(1,0,0)),pViewer);
    return 0;
}

int MotionControl::rotation(const QString &key, double angle, gp_Ax1 axis, bool copy)
{
    if(getControlObj(key) == nullptr)
    {
        DEBUG()<<"getControlObj("<<key<<") get is nullptr  ";
        return -1;
    }
    ControlAction ca(getControlObj(key));
    ca.rotation(angle,axis,copy);
    return 0;
}



void MotionControl::_delete()
{
    QList<QString> list = mShapeMap.keys();
    for(int i = 0;i < list.size();i++){
        if(mShapeMap.value(list.at(i))){
            delete mShapeMap.value(list.at(i));
        }
    }
    mShapeMap.clear();
}

ModelInitConfig *MotionControl::getModelInitConfig() const
{
    return mModelInitConfig;
}

void MotionControl::setModelInitConfig(ModelInitConfig *modelInitConfig)
{
    mModelInitConfig = modelInitConfig;
}

baseViewWindow *MotionControl::getViewer() const
{
    return pViewer;
}

void MotionControl::setViewer(baseViewWindow *value)
{
    pViewer = value;
}

int MotionControl::createPlates(QString fileName)
{
    TopoDS_Shape myBody;
    Transverter transverter;

    if(QString(fileName.split(".").last()).toUpper() == "DXF"){
        myBody = transverter.loadCAD(fileName,mModelInitConfig->getThickness());
        if(myBody.IsNull())
        {
            DEBUG()<<"loadCAD fileName failed"<<fileName;
            return -1;
        }
    }else if(QString(fileName.split(".").last()).toUpper() == "STEP"){
        myBody = transverter.loadMode(fileName);
        if(myBody.IsNull())
        {
            DEBUG()<<"loadMode fileName failed";
            return -1;
        }
        set_uv_thickness(mModelInitConfig->getThickness());
        set_u_height(mModelInitConfig->getHeight());
        set_v_width(mModelInitConfig->getWidth());
    }else{
        WARNING()<<" model path is error!";
        return -1;
    }
INFO()<<" uv_thickness "<<get_uv_thickness();
    //获取中心点
    double theXmin;double theYmin;double theZmin;
    double theXmax;double theYmax; double theZmax;
    Bnd_Box bbox;
    bbox.SetGap(0);

    BRepBndLib ret;
    ret.Add(myBody, bbox, Standard_True);
    bbox.Get(theXmin, theYmin, theZmin, theXmax, theYmax, theZmax);
    gp_Pnt center = gp_Pnt((theXmin+theXmax)/2.0,(theYmin+theYmax)/2.0,0);
    if(ISDEBUG) DEBUG()<<" center "<<" X "<<(theXmin+theXmax)/2.0<<" Y "<<(theYmin+theYmax)/2.0<<" Z "<<(theZmin+theZmax)/2.0;

    gp_Pnt p = mModelInitConfig->getCenter();

    gp_Trsf trsf;
    trsf.SetTranslation(center,gp_Pnt(mModelInitConfig->getHeight()/2.0,0,0));
    myBody = BRepBuilderAPI_Transform(myBody,trsf,true);
    trsf.SetTranslation(gp_Pnt(0,0,0),gp_Pnt(-mModelInitConfig->getHeight()/2,0,0));
    myBody = BRepBuilderAPI_Transform(myBody,trsf,true);
    if(mModelInitConfig->getRotationAng() != 0){
        trsf.SetRotation(gp_Ax1(gp_Pnt(0,0,0),gp::DZ()),mModelInitConfig->getRotationAng()/180*M_PI);
        myBody = BRepBuilderAPI_Transform(myBody,trsf,true);
    }
    if(mModelInitConfig->getIsOverturn()){
        trsf.SetRotation(gp_Ax1(gp_Pnt(0,0,(theZmin+theZmax)/2.0),gp::DX()),M_PI);
        myBody = BRepBuilderAPI_Transform(myBody,trsf,true);
    }

    if(qAbs(mModelInitConfig->getRotationAng()) == 90 || qAbs(mModelInitConfig->getRotationAng()) == 270){
        double v = get_v_width();
        double u = get_u_height();
        set_v_width(u);
        set_u_height(v);
    }


    TopoDS_Shape shape = myBody;

    if(ISDEBUG) DEBUG()<<"aa"<<p.X()<<p.Y()<<p.Z();
    myShape::setlock(true, __FILE__, __FUNCTION__, __LINE__);
    _createPlates("model", shape, p, true);

//    _createPlates("modelBend", shape, p, false);
    myShape::setlock(false, __FILE__, __FUNCTION__, __LINE__);

    return 0;
}

void MotionControl::_createPlates(QString key, TopoDS_Shape shape, gp_Pnt p, bool isDisplay)
{
    ControlObj *cobj_model = nullptr;
    int index = -1;
    gp_Vec pos = gp_Vec(0,0,0);
    double ang = 0;

    if(getControlObjs()->contains(key+"Del"))
        index = getControlObjs()->value(key+"Del")->getMainShape()->getIndex();

    cobj_model = new ControlObj(key);
    bindControlObj(key,cobj_model);

    cobj_model->getMainShape()->initData();
    cobj_model->setMainShape(shape,key,gp_Vec(0,0,0),gp_Vec(0,0,0),pos,0);
    cobj_model->getMainShape()->setModelType(PLATES);
    cobj_model->getMainShape()->setWidth(mModelInitConfig->getWidth());
    cobj_model->getMainShape()->setHeight(mModelInitConfig->getHeight());
    cobj_model->getMainShape()->setThickness(mModelInitConfig->getThickness());
    if(!mModelInitConfig->getBoundaryToCenter().isEmpty()){
        cobj_model->getMainShape()->setBoundaryToCenter(mModelInitConfig->getBoundaryToCenter());
    }
    else{
        QList<double> toCenter;
        for(int i = 0;i < 4;i++)
        {
            toCenter.append((i%2==0)?mModelInitConfig->getHeight()/2:mModelInitConfig->getWidth()/2);
        }
        mModelInitConfig->setBoundaryToCenter(toCenter);
        cobj_model->getMainShape()->setBoundaryToCenter(mModelInitConfig->getBoundaryToCenter());
    }
    cobj_model->getMainShape()->setFrontIndex(0);
    cobj_model->getMainShape()->setOBB(shape);
    cobj_model->getMainShape()->setIsRedisplay(isDisplay);
//    cobj_model->getMainShape()->setIndex(index);
//    Quantity_Color aMyColor (192.0/255.0, 192.0/255.0, 192.0/255.0, Quantity_TOC_RGB);
//    cobj_model->getMainShape()->getAisShape()->SetColor(MYCOLOR);
    cobj_model->getMainShape()->setColor(MYCOLOR);
    if(ISDEBUG) DEBUG()<<"getFrontIndex1"<<cobj_model->getMainShape()->getFrontIndex();
    cobj_model->getMainShape()->debugArg();

    for(int i = 0;i < mModelInitConfig->getEdgeNume();i++){
        myShape* Slave = nullptr;
        Slave = new myShape(QString::number(i));
        cobj_model->getMainShape()->addSlave(Slave);
        Slave->initData();
        Slave->setModelType(NODE);
        Slave->setKey(QString::number(i));
        Slave->setRelativeCurrentPosition(cobj_model->getMainShape()->getRelativeCurrentPosition());
        if(0 == i) Slave->setIsFront(true);
        if(ISDEBUG) DEBUG()<<" IsFront "<<Slave->getIsFront();
        Slave->setMarkAngle(90*i);
        Slave->setIsRedisplay(false);

    }
}

//        trsf.SetTranslation(p,gp_Pnt(4.26,0,87));   //xz
//        trsf.SetTranslation(p,gp_Pnt(70.93,0,-23.75)); //dd
//        trsf.SetTranslation(p,gp_Pnt(0,0,-19.75)); //UC
//        trsf.SetTranslation(p,gp_Pnt(82.96,0,297.477)); //W
int MotionControl::createModel(QString fileName, QString key, QVector3D Color)
{
    Transverter transverter;
    TopoDS_Shape x ;
//! 优化： 检查文件，判断是否存在
//!       在加载模型时进行异常处理
    if(fileName.split(".").last() == "dxf"){
        x = transverter.loadCAD(fileName);
        if(x.IsNull())
        {
            CRITICAL()<<" createModel error "<<fileName;
            return -1;
        }
    }else if(fileName.split(".").last() == "STEP"){
        x = transverter.loadMode(fileName);
        if(x.IsNull())
        {
            CRITICAL()<<" createModel error "<<fileName;
            return -1;
        }
    }else{
        return -1;
    }

    ControlObj *cobj_model = new ControlObj(key);
    cobj_model->getMainShape()->initData();
    cobj_model->setMainShape(x,key,gp_Vec(0,0,0),gp_Vec(0,0,0),gp_Vec(0,0,0),0);
    cobj_model->getMainShape()->setModelType(MODEL);
    cobj_model->getMainShape()->setIsRedisplay(true);
    cobj_model->getMainShape()->setIsVisible(true);
    bindControlObj(key,cobj_model);
    return 0;
}

ControlObj *MotionControl::createModel(TopoDS_Shape shape, QString key, QVector3D Color)
{
    ControlObj *cobj_model = new ControlObj(key);
    cobj_model->getMainShape()->initData();
    cobj_model->setMainShape(shape,key,gp_Vec(0,0,0),gp_Vec(0,0,0),gp_Vec(0,0,0),0);
    cobj_model->getMainShape()->setModelType(MODEL);
    cobj_model->getMainShape()->setIsRedisplay(true);
    cobj_model->getMainShape()->setIsVisible(true);
    bindControlObj(key,cobj_model);
    return cobj_model;
}

int MotionControl::createModel(QString UP, QString DN, QString key, QVector3D Color)
{
    Transverter transverter;
    TopoDS_Shape x ;
    TopoDS_Shape upShape ;
    TopoDS_Shape dnShape ;
//! 优化： 检查文件，判断是否存在
//!       在加载模型时进行异常处理
    if(UP.split(".").last() == "dxf"){
        upShape = transverter.loadCAD(UP);
        if(upShape.IsNull())
            return -1;
    }else if(UP.split(".").last() == "STEP"){
        upShape = transverter.loadMode(UP);
        if(upShape.IsNull())
        {
            CRITICAL()<<UP;
            return -1;
        }
    }else{
        return -1;
    }

    gp_Trsf trsf;
    if(key == "XZ"){
        trsf.SetTranslation(gp_Vec(getHCenterDistance(),0,getVCenterDistance()));
        upShape = BRepBuilderAPI_Transform(upShape,trsf).Shape();
    }

    if(DN.split(".").last() == "dxf"){
        dnShape = transverter.loadCAD(DN);
        if(dnShape.IsNull())
            return -1;
    }else if(DN.split(".").last() == "STEP"){
        dnShape = transverter.loadMode(DN);
        if(dnShape.IsNull())
        {
            CRITICAL()<<DN;
            return -1;
        }
    }else{
        return -1;
    }

    TopoDS_Compound aRes;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound (aRes);
    aBuilder.Add (aRes, upShape);
    aBuilder.Add (aRes, dnShape);
    x = aRes;

    ControlObj *cobj_model = new ControlObj(key);
    cobj_model->getMainShape()->initData();
    cobj_model->setMainShape(x,key,gp_Vec(0,0,0),gp_Vec(0,0,0),gp_Vec(0,0,0),0);
    cobj_model->getMainShape()->setModelType(MODEL);
    cobj_model->getMainShape()->setIsRedisplay(true);
    cobj_model->getMainShape()->setIsVisible(true);
    bindControlObj(key,cobj_model);
    return 0;
}

void MotionControl::createImpactor(TopoDS_Shape shape, QString key, gp_Pnt site)
{
    Transverter transverter;
    TopoDS_Shape x = shape;

    ControlObj *cobj_model = new ControlObj(key);
    cobj_model->getMainShape()->initData();

    cobj_model->setMainShape(x,key,gp_Vec(0,0,0),gp_Vec(0,0,0),gp_Vec(site.XYZ()),0);
    cobj_model->getMainShape()->setModelType(IMPACTOR);
    cobj_model->getMainShape()->setIsRedisplay(false);
    cobj_model->getMainShape()->setIsVisible(false);
    cobj_model->getMainShape()->setColor(Quantity_NOC_ORANGERED4);
    cobj_model->getMainShape()->setOBB(cobj_model->getMainShape()->getAisShape()->Shape());
    bindControlObj(key,cobj_model);
}

void MotionControl::setPauseState(bool value)
{
    myShape::setlock(true, __FILE__, __FUNCTION__, __LINE__);
    setPause(value);
    myShape::setlock(false, __FILE__, __FUNCTION__, __LINE__);
}

bool MotionControl::modelCheck()
{
    //! 吸盘模型

    if(get_sys_seriestype_config(KEYSYSCOMMON_SERIESTYPE_ID) == SERIESTYPE_P){
        if(!controlObjIsExist("1W")){
            DEBUG()<<"1W not exist";
            return true;
        }

    }

    if(!controlObjIsExist("XZ")){
        DEBUG()<<"XZ not exist";
        return true;
    }
    if(!controlObjIsExist("DW")){
        DEBUG()<<"dd not exist";
        return true;
    }
    if(!controlObjIsExist("W")){
        DEBUG()<<"W not exist";
        return true;
    }
    if(!controlObjIsExist("XZImpDown")){
        DEBUG()<<"XZImpDown not exist";
        return true;
    }
    if(!controlObjIsExist("XZImpUp")){
        DEBUG()<<"XZImpUp not exist";
        return true;
    }
    if(!controlObjIsExist("UC")){
        DEBUG()<<"UC not exist";
        return true;
    }
    return false;
}

int MotionControl::getControlMode() const
{
     if(ISINFO) INFO()<<" getControlMode "<<controlMode;
    return controlMode;
}

void MotionControl::setPattern(basePattern *pattern)
{
     if(ISINFO) INFO()<<" setPattern "<<pattern->getName();
    mBasePattern = pattern;
}

basePattern *MotionControl::getPattern()
{
     if(ISINFO) INFO()<<"BasePattern is "<<((mBasePattern == nullptr)?"no create":mBasePattern->getName());
    return mBasePattern;
}

ModelConfig *MotionControl::getMC() const
{
    return pMC;
}

pathconfig *MotionControl::getPathConfig() const
{
    return mPathConfig;
}

newModel *MotionControl::getNewModel() const
{
    return pNewModel;
}

void MotionControl::setControlMode(int value)
{
     if(ISINFO) INFO()<<" MotionControl::setControlMode "<<controlMode <<"->" <<value;
    controlMode = value;
}

