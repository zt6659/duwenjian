#include "controlobj.h"
#include "mydebug.h"


#ifndef PRO
#include "3D_simulation/baseviewwindow.h"
#else
#include "baseviewwindow.h"
#endif

static QMutex gMutex;
static int counter = 0;

ControlObj::ControlObj(const QString& key)
{
    mMainShape = new myShape(key);
}

ControlObj::~ControlObj()
{
//    if(mMainShape)
//        delete mMainShape;
    mMainShape = nullptr;
}

myShape::myShape(QString key)
{
    sKey = key;
    DEBUG()<<"creat shape"<<sKey;
    counter++;
    DEBUG()<<"counter"<<counter;
}

myShape::~myShape()
{
    cleanSlave();
    DEBUG()<<"delete shape"<<sKey;
    counter--;
    DEBUG()<<"counter"<<counter;
}

TopoDS_Shape myShape::getShape() const
{
    return Shape;
}

void myShape::setShape(const TopoDS_Shape &value)
{
    Shape = value;
}


void myShape::initData()
{
    //! OCCT obj
//    hAisShape.Nullify();
    clearOBB();
//    //! 显示对象
//    pViewer                      = nullptr;
//    cleanSlave();
    //! 显示数据索引号
    iIndex                       = -1;
    //! 标签
    sKey.clear();
    //! 类型
    iModelType                   = NONETYPE;
    //! 是否压边
    bIsBlankPressing               = false;
    //! 运动状态 (true 发生运动，false 没有发生运动)
    bStateOfMotion              = false;
    //! 最后一次运动
    iLastState                   = NONE;

    //! 公共属性
    //! 原点偏移
    vecOriginPositionOffset      = gp_Vec(0.0, 0.0 ,0.0);
    //! 绝对原点坐标
    vecAbsoluteOriginPosition    = gp_Vec(0.0, 0.0 ,0.0);
    //! 相对原点坐标
    vecRelativeOriginPosition    = gp_Vec(0.0, 0.0 ,0.0);
    //! 相对当前坐标
    vecRelativeCurrentPosition   = gp_Vec(0.0, 0.0 ,0.0);
    //! 旋转角度(代码旋转方向为逆时针，昊折旋转方向为顺时针)
    dRotateAngle                 = 0.0;
    //! 移动向量
    vecTranslation               = gp_Vec(0,0,0);


    //! 专用属性
    //! model
    //! 长
    dWidth                       = 0.0;
    //! 宽
    dHeight                      = 0.0;
    //! 板厚
    dThickness                   = 0.0;
    //! 边界到中心 起始边为要折弯的边,各边刀中心的距离（即长/2，宽/2.按顺时针顺序填）
    ldBoundaryToCenter.clear();
    //! 折弯角度
    dBendAngle                   = 0.0;
    //! 折弯时位置
    dBendSite                    = 0.0;
    //! 初始折弯角度(未变换时存在的折弯角度)
    dInitAngle                   = 0.0;
    //! 折弯轴
    BendAxis                     = gp_Ax1();
    //! 当前边为折弯边
    bIsFront                     = false;
    //! 标志角度(需要旋转多少度到这个边)
    dMarkAngle                   = 0.0;
    //! 复折圆心偏移
    dDFoldOffset                 = 0.0;

    //! 显示属性(状态)
    //! 显示重载
    bIsRedisplay                   = false;
    //! 是否可见
    bIsVisible                     = false;
    //! 是否发生变换
    bHasTransform                  = false;
    //! 删除模型
    bDelModel                      = false;

//    DEBUG()<<"init shape"<<sKey;
    //    DEBUG()<<"counter"<<counter;
}

void myShape::setlock(bool state, QString file, QString fun, int line)
{
    if(ISDEBUG) DEBUG()<<"setlock"<< state<<"file"<<file<<" func " <<fun<<" line "<<line;
    if(state == true){
        gMutex.lock();
    }else{
        gMutex.unlock();
    }
}

int myShape::getCounter()
{
    return counter;
}

int myShape::getKnifeType() const
{
    return knifeType;
}

void myShape::setKnifeType(int value)
{
    knifeType = value;
}

gp_Vec myShape::getRightSite() const
{
    return rightSite;
}

void myShape::setRightSite(const gp_Vec &value)
{
    rightSite = value;
}

gp_Vec myShape::getLeftSite() const
{
    return leftSite;
}

void myShape::setLeftSite(const gp_Vec &value)
{
    leftSite = value;
}

QString myShape::getPath() const
{
    return Path;
}

void myShape::setPath(const QString &value)
{
    Path = value;
}
#include <Prs3d_LineAspect.hxx>
void myShape::setAisShape(Handle_AIS_ColoredShape AisShape)
{
    hAisShape = AisShape;
    hAisShape->Attributes()->SetFaceBoundaryDraw(true);
    hAisShape->Attributes()->SetFaceBoundaryAspect(
                new Prs3d_LineAspect(Quantity_NOC_BLACK,Aspect_TOL_SOLID,0.5));
}

void myShape::setSection(const TopoDS_Shape &aSection)
{
    theSection = aSection;
}

void myShape::setColor(const Quantity_Color& aColor)
{
    theColor = aColor;
    if(!hAisShape.IsNull())
        hAisShape->SetColor(aColor);
}

//void myShape::setViewer(baseViewWindow *viewer)
//{
//    pViewer = viewer;
//}

void myShape::setIndex(int index)
{
//    DEBUG()<<"setIndex"<<index;
    iIndex = index;
}

void myShape::setKey(QString key)
{
    DEBUG()<<"setKey"<<key;
    sKey = key;
}

void myShape::setModelType(MODEL_TYPE ModelType)
{
    iModelType = ModelType;
}

void myShape::setIsBlankPressing(bool flag)
{
    bIsBlankPressing = flag;
}

void myShape::setOriginPositionOffset(gp_Vec OriginPositionOffset)
{
    vecOriginPositionOffset = OriginPositionOffset;
}

void myShape::setAbsoluteOriginPosition(gp_Vec AbsoluteOriginPosition)
{
    vecAbsoluteOriginPosition = AbsoluteOriginPosition;
}

void myShape::setRelativeOriginPosition(gp_Vec RelativeOriginPosition)
{
    vecRelativeOriginPosition = RelativeOriginPosition;
}

void ControlObj::setMainShape(const TopoDS_Shape& shape, const QString& key, gp_Vec AOP, gp_Vec ROP, gp_Vec RCP, double BA)
{
    if(mMainShape->getAisShape().IsNull()){
        DEBUG()<<"setMainShape new"<<key;
        Handle(AIS_ColoredShape) AisShape = new AIS_ColoredShape(shape);
        mMainShape->setAisShape(AisShape);
    }else{
        DEBUG()<<"setMainShape set"<<key;
        mMainShape->getAisShape()->Set(shape);
    }
    mMainShape->setKey(key);
    mMainShape->setBendAngle(BA);
    mMainShape->setAbsoluteOriginPosition(AOP);
    mMainShape->setRelativeOriginPosition(ROP);
    mMainShape->setRelativeCurrentPosition(RCP);
    mMainShape->setOriginPositionOffset(AOP-ROP);
}

void ControlObj::setMainShape(Handle(AIS_ColoredShape)shape, const QString& key, gp_Vec AOP, gp_Vec ROP, gp_Vec RCP, double BA)
{
    mMainShape->setAisShape(shape);
    mMainShape->setKey(key);
    mMainShape->setBendAngle(BA);
    mMainShape->setAbsoluteOriginPosition(AOP);
    mMainShape->setRelativeOriginPosition(ROP);
    mMainShape->setRelativeCurrentPosition(RCP);
    mMainShape->setOriginPositionOffset(AOP-ROP);
}

void ControlObj::setMainShape(myShape *shape)
{
    mMainShape = shape;
}

void myShape::setRelativeCurrentPosition(gp_Vec RelativeCurrentPosition)
{
    vecRelativeCurrentPosition = RelativeCurrentPosition;
}

void myShape::setRelativeCurrentPositionForBatch(gp_Vec RelativeCurrentPosition)
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->setRelativeCurrentPositionForBatch(RelativeCurrentPosition);
        }
    }
    if(!getAisShape().IsNull())
        setRelativeCurrentPosition(RelativeCurrentPosition);
}

void myShape::setBendAngle(double BendAngle)
{
    dBendAngle = BendAngle;
    if(ISINFO) INFO()<<"key"<<getKey()<<"dBendAngle"<<dBendAngle;
}

void myShape::setRotateAngle(double RotateAngle)
{
    if(qAbs(RotateAngle) >= 360)
    {
        int i = RotateAngle/360;
        if(RotateAngle >= 360){
            RotateAngle -= 360*i;
        }if(RotateAngle <= -360){
            RotateAngle += 360*i;
        }
    }
    dRotateAngle = RotateAngle;
}

void myShape::setThickness(double Thickness)
{
    dThickness = Thickness;
}

void myShape::setWidth(double width)
{
    dWidth = width;
}

void myShape::setHeight(double height)
{
    dHeight = height;
}

void myShape::setBoundaryToCenter(QList<double> BoundaryToCenter)
{
    ldBoundaryToCenter = BoundaryToCenter;
}

void myShape::setBoundaryToCenter(int index, double BoundaryToCenter)
{
    ldBoundaryToCenter[index] = BoundaryToCenter;
}

void myShape::setBendSite(double BendSite)
{
    dBendSite = BendSite;
}

void myShape::setIsFront(bool is)
{
    bIsFront = is;
}

void myShape::setMarkAngle(double ang)
{

    dMarkAngle = ang;
}

void myShape::setDFoldOffset(double offset)
{
    dDFoldOffset = offset;
}

void myShape::setFrontIndex(int index)
{
    if(ISDEBUG) DEBUG()<<" setFrontIndex "<<index;
    iFrontIndex = index;
}

void myShape::setOBB(TopoDS_Shape shape)
{
    BRepBndLib::AddOBB(shape, OBB, Standard_True, Standard_True, Standard_True);
}

void myShape::setOBB(Bnd_Box obb)
{
    OBB = obb;
}

void myShape::setOBB(const gp_Pnt &theCenter, const gp_Dir &theXDirection, const Standard_Real theHXSize, const gp_Dir &theYDirection, const Standard_Real theHYSize, const gp_Dir &theZDirection, const Standard_Real theHZSize)
{
//    qDebug()<<"setOBB "
//           <<"\n"<<"theCenter"<<theCenter.X()<<theCenter.Y()<<theCenter.Z()
//          <<"\n"<<"theXDirection"<<theXDirection.X()<<theXDirection.Y()<<theXDirection.Z()
//            <<"\n"<<"theYDirection"<<theXDirection.X()<<theXDirection.Y()<<theXDirection.Z()
//              <<"\n"<<"theZDirection"<<theXDirection.X()<<theXDirection.Y()<<theXDirection.Z()
//             <<"\n"<<"theHXSize"<<theHXSize
//               <<"\n"<<"theHYSize"<<theHYSize
//                 <<"\n"<<"theHZSize"<<theHZSize;
    OBB.SetCenter(theCenter);
    OBB.SetXComponent(theXDirection, theHXSize);
    OBB.SetYComponent(theYDirection, theHYSize);
    OBB.SetZComponent(theZDirection, theHZSize);
}

void myShape::setIsRedisplay(bool state, bool onlyOne)
{
//    if(getKey() == "right ear")
//        DEBUG()<<"";
    if(lpSlave.size() != 0 && onlyOne == false){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->setIsRedisplay(state);
        }
    }
    if(!getAisShape().IsNull())
        bIsRedisplay = state;
//    if(state == true)
//        DEBUG()<<"";
}

void myShape::setIsVisible(bool state, bool onlyOne)
{
    if(lpSlave.size() != 0 && onlyOne == false){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->setIsVisible(state);
        }
    }
    if(!getAisShape().IsNull())
        bIsVisible = state;
}

void myShape::setHasTransform(bool state)
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->setHasTransform(state);
        }
    }
    if(!getAisShape().IsNull())
        bHasTransform = state;
}

void myShape::setDelModel(bool state)
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->setDelModel(state);
        }
    }

    bDelModel = state;

}

void myShape::setInitAngle(double ang)
{
    dInitAngle = ang;
}

void myShape::setBendAxis(gp_Ax1 ax)
{
    BendAxis = ax;
}

void myShape::setStateOfMotion(bool state)
{
    bStateOfMotion = state;
}

void myShape::setLastState(LAST_MOTION_STATE state)
{
    iLastState = state;
}

void myShape::setFollowIndex(int Index)
{
    iFollowIndex = Index;
}

Handle_AIS_ColoredShape myShape::getAisShape()
{
//    DEBUG()<<"getAisShape";
    return hAisShape;
}

const TopoDS_Shape &myShape::getSection() const
{
    return theSection;
}

const Quantity_Color &myShape::getColor() const
{
    return theColor;
}

//baseViewWindow *myShape::getViewer()
//{
//    return pViewer;
//}

int myShape::getIndex() const
{
    return iIndex;
}

QString myShape::getKey() const
{
    return sKey;
}

int myShape::getModelType()
{
    return iModelType;
}

bool myShape::getIsBlankPressing()
{
    return bIsBlankPressing;
}

gp_Vec myShape::getOriginPositionOffset() const
{
    return vecOriginPositionOffset;
}

gp_Vec myShape::getAbsoluteOriginPosition() const
{
    return vecAbsoluteOriginPosition;
}

gp_Vec myShape::getRelativeOriginPosition() const
{
    return vecRelativeOriginPosition;
}

myShape* ControlObj::getMainShape() const
{
    return mMainShape;
}

gp_Vec myShape::getRelativeCurrentPosition() const
{
    return vecRelativeCurrentPosition;
}

double myShape::getBendAngle() const
{
    return dBendAngle;
}

double myShape::getRotateAngle() const
{
    return dRotateAngle;
}

double myShape::getThickness() const
{
    return dThickness;
}

double myShape::getWidth() const
{
    return dWidth;
}

double myShape::getHeight() const
{
    return dHeight;
}

QList<double> myShape::getBoundaryToCenter() const
{
    return ldBoundaryToCenter;
}

double myShape::getBoundaryToCenter(int index) const
{
    return ldBoundaryToCenter[index];
}

double myShape::getBendSite() const
{
    return dBendSite;
}

bool myShape::getIsFront() const
{
    return bIsFront;
}

double myShape::getMarkAngle() const
{
    return dMarkAngle;
}

double myShape::getDFoldOffset() const
{
    return dDFoldOffset;
}

int myShape::getFrontIndex() const
{
    return iFrontIndex;
}

Bnd_OBB myShape::getOBB() const
{
    return OBB;
}

bool myShape::getIsRedisplay()
{
    return bIsRedisplay;
}

bool myShape::getIsVisible()
{
    return bIsVisible;
}

bool myShape::getHasTransform()
{
    return bHasTransform;
}

bool myShape::getDelModel()
{
    return bDelModel;
}

double myShape::getInitAngle() const
{
    return dInitAngle;
}

gp_Ax1 myShape::getBendAxis() const
{
    return BendAxis;
}

void myShape::AisShapeNullify()
{
    hAisShape.Nullify();
}

void myShape::clearOBB()
{
    OBB.SetVoid();
}

void myShape::addSlave(myShape *Slave)
{
    DEBUG()<<" addSlave "<<this->getKey()<<" add "<<Slave->getKey();
    lpSlave.append(Slave);
}

void myShape::modifySlave(int index, myShape *Slave)
{
    if(lpSlave.size() > index ){
        DEBUG()<<" modifySlave "<<this->getKey()<<index<<" modify "<<Slave->getKey();
        lpSlave[index] = Slave;
    }
    else
        DEBUG()<<"lpSlave.size() error";
}

void myShape::removeSlave(int index)
{
    DEBUG()<<" removeSlave "<<this->getKey()<<index<<" remove "<<lpSlave.at(index)->getKey();
    lpSlave.removeAt(index);
}

void myShape::removeSlave(myShape *Slave)
{
    lpSlave.removeOne(Slave);
}

myShape *myShape::getSlave(int index)
{
    if(lpSlave.size()){
        if(index >= lpSlave.size() || lpSlave.size() < 0)
            DEBUG()<<"getSlave error";

//        if(ISDEBUG)DEBUG()<<getKey()<<"index"<<index<<"lpSlave.size() "<<lpSlave.size() /*<<&lpSlave[index]*/;
//        myShape *ret = lpSlave[index];
        return lpSlave[index];
    }


    return nullptr;
}

myShape *myShape::getSlave(MODEL_TYPE type)
{
    for(int i = 0;i < lpSlave.size();i++){
        if(lpSlave.at(i)->getModelType() == type)
            return lpSlave[i];
    }
    return nullptr;
}

QList<myShape *> myShape::getSlaveList()
{
    return lpSlave;
}

int myShape::getSlaveNum()
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        int scount = 0;
        for(int i = 0;i < count;i++){
            scount += lpSlave[i]->getSlaveNum();
        }
        return count+scount;
    }
    return 0;
}

myShape *myShape::getSlaveForCurrentFront()
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            if(lpSlave[i]->getIsFront())
                return lpSlave[i];
        }
    }
    return nullptr;
}

bool myShape::hasModelTypeSlave(MODEL_TYPE ModelType)
{
    for(int i = 0;i < getSlaveList().size();i++){
        if(getSlave(i)->getModelType() == ModelType)
            return true;
    }
    return false;
}

void myShape::debugArg()
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->debugArg();
        }
    }
    if(ISDEBUG) DEBUG()<<"debugArg"<<getIndex()<<getKey();
}

void myShape::cleanSlave()
{
    lpSlave.clear();
//    if(lpSlave.size() != 0 ){
//        int count = lpSlave.size();
//        for(int i = 0;i < count;i++){
//            myShape *tmp = lpSlave[i];
//            tmp->cleanSlave();
//            tmp->AisShapeNullify();
//            delete tmp;
//            if(ISDEBUG) DEBUG()<<tmp;
//            lpSlave[i] = nullptr;
//        }
//        lpSlave.clear();
//    }
}



int myShape::_translation(gp_Vec destPnt, const Standard_Boolean Copy, bool isDiaplay)
{
    if(ISDEBUG) DEBUG()<<" func: "<<" _translation "<<" key "<<getKey()
          <<destPnt.X()<<destPnt.Y()<<destPnt.Z();
    gp_Vec dis = destPnt - getRelativeCurrentPosition();
    if(ISDEBUG) DEBUG()<<" _translation dis "<<dis.X()<<dis.Y()<<dis.Z();
    TopoDS_Shape shape = getAisShape()->Shape();
    gp_Trsf trsf;
    trsf.SetTranslation(dis);
    if(Copy){

        shape = BRepBuilderAPI_Transform(shape,trsf,Copy);
    }else{
//        shape = BRepBuilderAPI_Transform(shape,trsf);
        TopLoc_Location l = shape.Location();
        TopLoc_Location loc = trsf*l;
        shape.Location(loc);
    }
    getAisShape()->Set(shape);

    {
        gp_Vec dis_section = dis;
        if(dis_section.Z() != 0)
            dis_section.SetZ(0);
        gp_Trsf trsf_section;
        trsf_section.SetTranslation(dis_section);
        //! 切割截面移动
        TopoDS_Shape theSectionShape = getSection();
        if(Copy){

            theSectionShape = BRepBuilderAPI_Transform(theSectionShape,trsf_section,Copy);
        }else{
            TopLoc_Location l = theSectionShape.Location();
            TopLoc_Location loc = trsf_section*l;
            theSectionShape.Location(loc);
        }
        setSection(theSectionShape);
    }

    if(ISDEBUG) DEBUG()<<"befor"<<getRelativeCurrentPosition().X()
         <<getRelativeCurrentPosition().Y()
         <<getRelativeCurrentPosition().Z();
    setRelativeCurrentPosition(getRelativeCurrentPosition()+dis);
    if(ISDEBUG) DEBUG()<<"after"<<getRelativeCurrentPosition().X()
         <<getRelativeCurrentPosition().Y()
         <<getRelativeCurrentPosition().Z();
    setStateOfMotion(true);
    setLastState(TRANSLATION);

    if(!getOBB().IsVoid()){
        gp_Vec center = getOBB().Center();
        center = center + dis;
        setOBB(gp_Pnt(center.XYZ()),
               gp_Dir(getOBB().XDirection()),
               getOBB().XHSize(),
               gp_Dir(getOBB().YDirection()),
               getOBB().YHSize(),
               gp_Dir(getOBB().ZDirection()),
               getOBB().ZHSize());
    }
    setStateOfMotion(false);
    if(!getIsRedisplay())
        setHasTransform(true);
    if(ISDEBUG) DEBUG()<<" func: "<<" _translation ";

    return 1;
}

int myShape::translation(gp_Vec destPnt, bool transEar, const Standard_Boolean Copy, bool isDiaplay)
{
    if(getModelType() == EAR && transEar == false)
        return 1;
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->translation(destPnt,transEar,Copy);
        }
    }
    if(!getAisShape().IsNull())
        _translation(destPnt, Copy, isDiaplay);
    return 1;
}

int myShape::rotation(double angle, gp_Ax1 axis, const Standard_Boolean Copy)
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->rotation(angle,axis,false);
        }
    }
    if(!getAisShape().IsNull())
        _rotation(angle,axis,false);
    return 1;
}

int myShape::bend(double angle, gp_Ax1 axis)
{
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            lpSlave[i]->bend(angle,axis);
        }
    }
    if(!getAisShape().IsNull())
        _bend(angle,axis);
    return 1;
}

myShape *myShape::collisionDetection(Bnd_OBB s1, gp_Ax1 ax, QMap<QString, ControlObj *> *shapeMap)
{
    myShape* ret = nullptr;
    if(lpSlave.size() != 0){
        int count = lpSlave.size();
        for(int i = 0;i < count;i++){
            ret = lpSlave[i]->collisionDetection(s1,ax,shapeMap);
            if(ret != nullptr && ret->getIsBlankPressing())
            {
                //! 折弯边大于160°
                WARNING()<<ret->getKey()<<" getIsBlankPressing "<<ret->getIsBlankPressing();
                continue;
            }
            if(ret != nullptr){
                INFO()<<getKey()<<"collisionDetection1"<<ret<<getBendAngle();
                return ret;
            }
        }
    }
    if(!getAisShape().IsNull())
        ret = _collisionDetection(s1,ax,shapeMap);
    if(ISINFO) INFO()<<getKey()<<"collisionDetection2"<<ret;
    return ret;
}


int myShape::_rotation(double angle, gp_Ax1 axis, const Standard_Boolean Copy)
{
    if(ISDEBUG) DEBUG()<<" func: "<<" _rotation "<<" key "<<getKey();

    double ang = angle<0?angle+360:angle;
    double radian = angle/180*M_PI;
    TopoDS_Shape shape = getAisShape()->Shape();
    gp_Trsf trsf;
    trsf.SetRotation(axis,-radian);
//    DEBUG()<<" _translation 1";
    if(Copy){
        shape = BRepBuilderAPI_Transform(shape,trsf,Copy);
    }else{
//        shape = BRepBuilderAPI_Transform(shape,trsf);
        TopLoc_Location loc = trsf*shape.Location();
        shape.Location(loc);
    }
//    DEBUG()<<" _translation 1";
    if(shape.IsNull())
        return -1;
    getAisShape()->Set(shape);
    setRotateAngle(getRotateAngle()+ang);
    setStateOfMotion(true);
    setLastState(ROTATION);

    if(!getOBB().IsVoid()){
        gp_Pnt center = getOBB().Center();
        center = center.Rotated(axis,-radian);
        setOBB(center.XYZ(),
               gp_Dir(getOBB().XDirection()).Rotated(axis,-ang/180.0*M_PI),
               getOBB().XHSize(),
               gp_Dir(getOBB().YDirection()).Rotated(axis,-ang/180.0*M_PI),
               getOBB().YHSize(),
               gp_Dir(getOBB().ZDirection()).Rotated(axis,-ang/180.0*M_PI),
               getOBB().ZHSize());

    }

    setStateOfMotion(false);
    if(!getIsRedisplay())
        setHasTransform(true);
    return 1;
}

int myShape::_bend(double angle, gp_Ax1 axis)
{

    double radian = angle/180*M_PI;
//    if(getKey() == "SlaveMain" )
    if(ISDEBUG) DEBUG()<<" axis "<<axis.Location().X()<<axis.Location().Y()<<axis.Location().Z();
    TopoDS_Shape shape = getAisShape()->Shape();
    gp_Trsf trsf;
    trsf.SetRotation(axis,radian);
//    shape = BRepBuilderAPI_Transform(shape,trsf,false);
    TopLoc_Location loc = trsf*shape.Location();
    shape.Location(loc);

    getAisShape()->Set(shape);
    setStateOfMotion(true);
    setLastState(BEND);
    setBendAxis(axis);
    setBendAngle(getBendAngle()+angle);

    if(!getOBB().IsVoid()){
        gp_Pnt center = getOBB().Center();
        center = center.Rotated(axis,radian);
        setOBB(center.XYZ(),
               gp_Dir(getOBB().XDirection()).Rotated(axis,angle/180.0*M_PI),
               getOBB().XHSize(),
               gp_Dir(getOBB().YDirection()).Rotated(axis,angle/180.0*M_PI),
               getOBB().YHSize(),
               gp_Dir(getOBB().ZDirection()).Rotated(axis,angle/180.0*M_PI),
               getOBB().ZHSize());
    }

    setStateOfMotion(false);
    if(!getIsRedisplay())
        setHasTransform(true);
    return 1;
}

myShape *myShape::_collisionDetection(Bnd_OBB s1, gp_Ax1 ax, QMap<QString, ControlObj *> *shapeMap)
{
    bool ret = true;
    if(getModelType() != CUT) return nullptr;
    Bnd_OBB tmpOBB;

    gp_Dir X(s1.XDirection().X(),s1.XDirection().Y(),s1.XDirection().Z());
    gp_Dir Y(s1.YDirection().X(),s1.YDirection().Y(),s1.YDirection().Z());
    gp_Dir Z(s1.ZDirection().X(),s1.ZDirection().Y(),s1.ZDirection().Z());

    tmpOBB.SetCenter(s1.Center());
    tmpOBB.SetXComponent(X.Rotated(ax,(getBendAngle()+45)/180.0*M_PI),s1.XHSize());
    tmpOBB.SetYComponent(Y.Rotated(ax,(getBendAngle()+45)/180.0*M_PI),s1.YHSize());
    tmpOBB.SetZComponent(Z.Rotated(ax,(getBendAngle()+45)/180.0*M_PI),s1.ZHSize());
    if(ISINFO) INFO()<<"key"<<getKey()<<"getBendAngle"<<getBendAngle()<<((getBendAngle()+45)/180.0*M_PI);
    ret = getOBB().IsOut(tmpOBB);
    if(1&TEST_DISPLAY&(ret == false)){

        Handle(AIS_ColoredShape) shape = new AIS_ColoredShape(getAisShape()->Shape());
        if(!shapeMap->contains("ss")){
            ControlObj *cobj_model = new ControlObj("ss");
            cobj_model->getMainShape()->initData();
            cobj_model->setMainShape(shape,"ss",gp_Vec(0,0,0),gp_Vec(0,0,0),gp_Vec(0,0,0),0);
            cobj_model->getMainShape()->setModelType(TEST);
            cobj_model->getMainShape()->setColor(Quantity_NOC_ORANGERED4);
            cobj_model->getMainShape()->setIsRedisplay(true);
            cobj_model->getMainShape()->setIsVisible(true);
            shapeMap->insert("ss",cobj_model);
        }else{
            shapeMap->value("ss")->getMainShape()->getAisShape()->SetShape(getAisShape()->Shape());
            shapeMap->value("ss")->getMainShape()->setIsRedisplay(true);
        }

        gp_Pnt theP1[8];
        getOBB().GetVertex(theP1);
        TopoDS_Compound aRes1;
        BRep_Builder aBuilder1;
        aBuilder1.MakeCompound (aRes1);
//        for(int i = 0;i < 8;i++){
//            aBuilder1.Add(aRes1,BRepBuilderAPI_MakeVertex(theP1[i]));
//        }
        for(int i =0;i < 8;i++)
        {
            /*if(theP1[i].Y() < 0)*/{
                aBuilder1.Add(aRes1,BRepBuilderAPI_MakeVertex(theP1[i]));
            }
        }
//        aBuilder1.Add(aRes1,BRepBuilderAPI_MakeVertex(theP1[3]));
//        aBuilder1.Add(aRes1,BRepBuilderAPI_MakeVertex(theP1[6]));
        shape = new AIS_ColoredShape(aRes1);
        if(!shapeMap->contains("dd")){
            ControlObj *cobj_model = new ControlObj("dd");
            cobj_model->getMainShape()->initData();
            cobj_model->setMainShape(shape,"dd",gp_Vec(0,0,0),gp_Vec(0,0,0),gp_Vec(0,0,0),0);
            cobj_model->getMainShape()->setModelType(TEST);
            cobj_model->getMainShape()->setColor(Quantity_NOC_YELLOW);
            cobj_model->getMainShape()->setIsRedisplay(true);
            cobj_model->getMainShape()->setIsVisible(true);
            shapeMap->insert("dd",cobj_model);
        }else{
            shapeMap->value("dd")->getMainShape()->getAisShape()->SetShape(aRes1);
            shapeMap->value("dd")->getMainShape()->setIsRedisplay(true);
        }



//        gp_Pnt theP[8];
//        tmpOBB.GetVertex(theP);
//        TopoDS_Compound aRes;
//        BRep_Builder aBuilder;
//        aBuilder.MakeCompound (aRes);
//        for(int i = 0;i < 8;i++){
//            aBuilder.Add(aRes,BRepBuilderAPI_MakeVertex(theP[i]));
//        }

    }
    if(ISDEBUG) DEBUG()<<getKey()<<"_collisionDetection"<<ret<<getBendAngle();

    if( ret == false)
        return this;

    return nullptr;
}

bool myShape::getStateOfMotion() const
{
    return bStateOfMotion;
}

int myShape::getLastState() const
{
    return iLastState;
}

int myShape::getFollowIndex() const
{
    return iFollowIndex;
}
