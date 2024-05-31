#include "controlaction.h"
#include "mydebug.h"

ControlAction::ControlAction()
{

}

ControlAction::ControlAction(ControlObj *shape)
{
    init(shape);
}

ControlAction::~ControlAction()
{

}

void ControlAction::init(ControlObj *shape)
{
    if( shape == nullptr || shape->getMainShape()->getAisShape().IsNull())
    {
        DEBUG()<<"ControlAction init failed!";
        return;
    }
    mControlObj = shape;

    mInit = true;
}

int ControlAction::bend(double angle, gp_Ax1 ax, gp_Pln pln, baseViewWindow *view)
{
    if(ISDEBUG) DEBUG()<<"entry";
    if(mInit == false)
    {
        DEBUG()<<"ControlAction was not init!";
        return -1;
    }

    double radian = angle/180*M_PI;

    TopoDS_Shape mainShape;
    TopoDS_Shape theShape;
    TopoDS_Shape faceShape;
    Transverter transverter;
    double cutLen = 0;


    //! 切割板材
    theShape = mControlObj->getMainShape()->getAisShape()->Shape();

    TopoDS_Face fc = BRepBuilderAPI_MakeFace(pln,-1,/*1000*/mControlObj->getMainShape()->getThickness()*1000,-10000,10000);
//    gp_Pnt p = gp_Pnt(0,0,0);
//    gp_Dir d = gp_Dir(1,0,0);

//    TopoDS_Face fc = BRepBuilderAPI_MakeFace(gp_Pln(p,d),-1,1000,-10000,10000);
    int count = mControlObj->getMainShape()->getSlaveList().size();

    int leftIndex = -1,rightIndex = -1;

    int slaveIndex = -1;
    for(int i = 0;i < count;i++){
        if(mControlObj->getMainShape()->getSlave(i)->getIsFront())
            slaveIndex = i;
    }
    if(slaveIndex == 0)
    {
        leftIndex = slaveIndex+1;
        rightIndex = mControlObj->getMainShape()->getSlaveList().size()-1;
    }else if(slaveIndex == mControlObj->getMainShape()->getSlaveList().size()-1){
        leftIndex = 0;
        rightIndex = slaveIndex-1;
    }else{
        leftIndex = slaveIndex+1;
        rightIndex = slaveIndex-1;
    }

    if(slaveIndex == -1)
    {
        WARNING()<<" bend error : not found front edge";
        return -1;
    }

    if(ISDEBUG) DEBUG()<<"leftIndex"<<leftIndex<<"slaveIndex"<<slaveIndex<<"rightIndex"<<rightIndex;
    if(ISDEBUG) DEBUG()<<(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size() == 0)
          <<(mControlObj->getMainShape()->getSlave(slaveIndex)->getBendSite() != mControlObj->getMainShape()->getRelativeCurrentPosition().X())
         <<mControlObj->getMainShape()->getSlave(slaveIndex)->getBendSite()<<mControlObj->getMainShape()->getRelativeCurrentPosition().X();

    if( mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size() == 0 ||
            mControlObj->getMainShape()->getSlave(slaveIndex)->getBendSite() != mControlObj->getMainShape()->getRelativeCurrentPosition().X() )
    {
        BOPAlgo_MakerVolume mv;
        mv.AddArgument(theShape);
        mv.AddArgument(fc);
        Standard_Real aFuzzyValue = 1.e-4;
        mv.SetFuzzyValue (aFuzzyValue);
        mv.Perform();


//        TopoDS_Shape mv = transverter.splitModel(theShape,fc);

        //! 碰撞检测对象
        Bnd_OBB OBBs;
        //! 碰撞检测对象
        Bnd_OBB OBBf;
        BRepBndLib::AddOBB(theShape, OBBs, Standard_False, Standard_False, Standard_False);
        BRepBndLib::AddOBB(fc, OBBf, Standard_False, Standard_False, Standard_False);

        if(ISDEBUG) DEBUG()<<" ------ start ------";
//        mControlObj->getMainShape()->translation(gp_Vec(mControlObj->getMainShape()->getRelativeCurrentPosition().X(),0,0),true);
//return 0;

        TopTools_IndexedMapOfShape aShapeMap;
        TopExp::MapShapes(mv.Shape(), TopAbs_SOLID, aShapeMap);
//        TopExp::MapShapes(mv, TopAbs_SOLID, aShapeMap);

        if(ISDEBUG) DEBUG()<<" -- bend 2 -- "<<QTime::currentTime().msec();

        if(ISDEBUG) DEBUG() <<" ----- 切割板材 ------ "<< aShapeMap.Extent();
        if(aShapeMap.Extent() < 2)
        {
//            void *p;
//            Handle(AIS_Shape) ss = new AIS_Shape(theShape);
//            p = &ss;
//            view->display(p);
//            ss = new AIS_Shape(fc);
//            p = &ss;
//            view->display(p);
            INFO()<<" HasErrors "<<mv.HasErrors()<<"HasWarnings"<<mv.HasWarnings()<<OBBs.IsOut(OBBf);;
            if(mv.HasErrors()){
                Standard_SStream aSStream;
                mv.DumpErrors(aSStream);
                std::string result;
                aSStream >> result;
                WARNING()<<"BOPAlgo_MakerVolume Error: "<< QString(result.c_str());
            }
            if(mv.HasWarnings()){
                Standard_SStream aSStream;
                mv.DumpWarnings(aSStream);
                std::string result;
                aSStream >> result;
                WARNING()<<"BOPAlgo_MakerVolume Warning: "<< QString(result.c_str());
            }


            WARNING()<<__FILE__<<__FUNCTION__<<"shape cutting failure, aShapeMap num: "<< aShapeMap.Extent();
//            void *ptr;
//            Handle(AIS_Shape) sss = new AIS_Shape(theShape);
//            ptr = &sss;
//            if(view != nullptr)
//                view->display(ptr);
            return -1;
        }

        cutLen = mControlObj->getMainShape()->getBoundaryToCenter(slaveIndex)-mControlObj->getMainShape()->getRelativeCurrentPosition().X();
        if(!ISDEBUG) DEBUG()<<" cutLen "<<cutLen<<"getRelativeCurrentPosition_X"<<mControlObj->getMainShape()->getRelativeCurrentPosition().X();
        mControlObj->getMainShape()->setBoundaryToCenter(slaveIndex,mControlObj->getMainShape()->getRelativeCurrentPosition().X());
        mControlObj->getMainShape()->getSlave(slaveIndex)->setBendSite(mControlObj->getMainShape()->getRelativeCurrentPosition().X());



        Standard_Real max_x = 0;
        Standard_Integer max_index = 1;

        for(int i = 1;i <= aShapeMap.Extent();i++){
            GProp_GProps System;
            BRepGProp::LinearProperties(aShapeMap(i),System);
            System.Mass();
            gp_Pnt p = System.CentreOfMass();

            if(p.X() > max_x ){
                max_x = p.X();
                max_index = i;
            }
//            DEBUG() <<<<" ----- "<< i <<" ------ "<<p.X()<<p.Y()<<p.Z();
        }
        mainShape = aShapeMap(max_index);
//        DEBUG()<<" ---------- befor ----------";
        myShape *slave = mControlObj->getMainShape();
        gp_XYZ theAxis,xyz;
        Standard_Real theAngle;
        gp_Quaternion rota = slave->getAisShape()->Shape().Location().Transformation().GetRotation();
        slave->getAisShape()->Shape().Location().Transformation().GetRotation(theAxis,theAngle);
        xyz = slave->getAisShape()->Shape().Location().Transformation().TranslationPart();
//        DEBUG()<<slave->getKey()<<"theAxis"<<theAxis.X()<<theAxis.Y()<<theAxis.Z()<<"theAngle"<<theAngle;
//        DEBUG()<<slave->getKey()<<"Position"<<xyz.X()<<xyz.Y()<<xyz.Z();
//        DEBUG()<<"";


        mControlObj->getMainShape()->getAisShape()->Set(mainShape);
        mControlObj->getMainShape()->setIsRedisplay(true, true);
        mControlObj->getMainShape()->clearOBB();
        mControlObj->getMainShape()->setOBB(mainShape);


//        DEBUG()<<" ---------- after ----------";
        mainShape.Location().Transformation().GetRotation(theAxis,theAngle);
        xyz = mainShape.Location().Transformation().TranslationPart();
//        DEBUG()<<slave->getKey()<<"theAxis"<<theAxis.X()<<theAxis.Y()<<theAxis.Z()<<"theAngle"<<theAngle;
//        DEBUG()<<slave->getKey()<<"Position"<<xyz.X()<<xyz.Y()<<xyz.Z();
//        DEBUG()<<"";

        aShapeMap.RemoveFromIndex(max_index);
        //! 中间值
        Standard_Real min_y = 999999;
        if(ISDEBUG) DEBUG() <<" ----- 切割板材2 ------ "<< aShapeMap.Extent();
        for(int i = 1;i <= aShapeMap.Extent();i++){
            GProp_GProps System;
            BRepGProp::LinearProperties(aShapeMap(i),System);
            System.Mass();
            gp_Pnt p = System.CentreOfMass();

            if(qAbs(p.Y()) < min_y)
            {
//                DEBUG() <<" ----- "<< i <<" ------ "<<p.Y();
                min_y = qAbs(p.Y());
                max_index = i;
            }
            if(ISDEBUG) DEBUG() <<"min_y"<<min_y<<"max_index"<<max_index<<" ----- "<< i <<" ------ "<<p.X()<<p.Y()<<p.Z();
        }
        if(ISDEBUG) DEBUG()<<" -- bend 2.1 -- "<<QTime::currentTime().msec();
        myShape *SlaveMain = new myShape(QString("SlaveMain %1 - %2").arg(mControlObj->getMainShape()->getSlave(slaveIndex)->getKey()).arg(myShape::getCounter()));
        SlaveMain->initData();
        SlaveMain->setAisShape(new AIS_ColoredShape(aShapeMap(max_index)));
        SlaveMain->setKey(QString("SlaveMain %1 - %2").arg(mControlObj->getMainShape()->getSlave(slaveIndex)->getKey()).arg(myShape::getCounter()));
        SlaveMain->setModelType(CUT);
        SlaveMain->setWidth(mControlObj->getMainShape()->getWidth());
        SlaveMain->setHeight(cutLen);
        SlaveMain->setThickness(mControlObj->getMainShape()->getThickness());
        SlaveMain->setBendSite(mControlObj->getMainShape()->getSlave(slaveIndex)->getBendSite());
        SlaveMain->setRelativeCurrentPosition(mControlObj->getMainShape()->getRelativeCurrentPosition());
        SlaveMain->setRotateAngle(mControlObj->getMainShape()->getRotateAngle());
        SlaveMain->setOBB(aShapeMap(max_index));
        SlaveMain->setIsRedisplay(true);
        SlaveMain->setIndex(myShape::getCounter());
        SlaveMain->setColor(mControlObj->getMainShape()->getColor());
        TopoDS_Shape aFaces = Transverter::section(aShapeMap(max_index),fc); /* resulting faces */
        if(aFaces.IsNull())
        {
            WARNING() << "Error: section failed\n";
            return -1;
        }
        if(0&TEST_DISPLAY)
        {
            gp_Pnt theP1[8];
            SlaveMain->getOBB().GetVertex(theP1);


            TopoDS_Compound aRes1;
            BRep_Builder aBuilder1;
            aBuilder1.MakeCompound (aRes1);
            for(int i =0;i < 8;i++)
            {
                aBuilder1.Add(aRes1,BRepBuilderAPI_MakeVertex(theP1[i]));
            }
            Handle(AIS_ColoredShape) shape = new AIS_ColoredShape(aRes1);
            void* sh;
            sh = &shape;
            view->display(sh,QString("SlaveMain111"),Quantity_NOC_BLUE);

            shape = new AIS_ColoredShape(aShapeMap(max_index));
            sh = &shape;
            view->display(sh,QString("SlaveMain1113"),Quantity_NOC_BLUE);
        }
        SlaveMain->setSection(aFaces);


//        mControlObj->getMainShape()->getSlave(slaveIndex)->debugArg();
//        DEBUG() <<" ----------1 --------- ";
        if(ISDEBUG) DEBUG()<<" -- bend 3 -- "<<QTime::currentTime().msec();

        if(mControlObj->getMainShape()->getSlave(slaveIndex)->hasModelTypeSlave(EAR)){
            QList<int> listIn;
            for(int i = 0;i < mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size();i++){
                if(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getModelType() == EAR){
                    GProp_GProps System;
                    BRepGProp::LinearProperties(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getAisShape()->Shape(),
                                                System);
                    System.Mass();
                    gp_Pnt p = System.CentreOfMass();
                    if(p.X() <0){
                        SlaveMain->addSlave(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i));
                        listIn.append(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getIndex());
                        if(ISDEBUG) DEBUG()<<"listIn.size()"<<mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getIndex()<<
                                  mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getKey();
                    }
                }
            }

            for(int i =0; i < listIn.size();i++){
                for(int tmpi = 0;tmpi < mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size();tmpi++ ){
                    if(listIn.at(i) ==  mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(tmpi)->getIndex()){
                        mControlObj->getMainShape()->getSlave(slaveIndex)->removeSlave(tmpi);
                    }
                }
            }
        }
//        mControlObj->getMainShape()->getSlave(slaveIndex)->debugArg();
//        DEBUG() <<" ----------2 --------- ";
        if(ISDEBUG) DEBUG()<<" -- bend 4 -- "<<QTime::currentTime().msec();

        if(mControlObj->getMainShape()->getSlave(slaveIndex)->hasModelTypeSlave(CUT)){
            int iii = -1;
            DEBUG()<<mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size();
            for(int i = 0;i < mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size();i++){
                DEBUG()<<mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getModelType();
                if(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i)->getModelType() == CUT){
                    SlaveMain->addSlave(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(i));
                    iii = i;
                }
                DEBUG()<<" iii "<<iii;
            }
            mControlObj->getMainShape()->getSlave(slaveIndex)->removeSlave(iii);

        }
//DEBUG() <<" ---------- --------- ";
        if(ISDEBUG) DEBUG()<<" -- bend 5 -- "<<QTime::currentTime().msec();
        mControlObj->getMainShape()->getSlave(slaveIndex)->addSlave(SlaveMain);

        aShapeMap.RemoveFromIndex(max_index);

        for(int i = 1;i <= aShapeMap.Extent();i++){
            GProp_GProps System;
            BRepGProp::LinearProperties(aShapeMap(i),System);
            System.Mass();
            gp_Pnt p = System.CentreOfMass();

            DEBUG()<<" min_y "<<min_y <<" -- ear --- "<< i <<" ------ "<<p.X()<<p.Y()<<p.Z();
            if(p.Y() < min_y){
                myShape *ear = new myShape("left");
                ear->initData();
//                ear->setViewer(mControlObj->getMainShape()->getViewer());
                ear->setAisShape(new AIS_ColoredShape(aShapeMap(i)));
                ear->setKey("left ear");
                ear->setModelType(EAR);
                ear->setThickness(mControlObj->getMainShape()->getThickness());
                ear->setBendSite(mControlObj->getMainShape()->getSlave(slaveIndex)->getBendSite());
                ear->setRelativeCurrentPosition(mControlObj->getMainShape()->getRelativeCurrentPosition());
                ear->setRotateAngle(mControlObj->getMainShape()->getRotateAngle());
                ear->setFollowIndex(mControlObj->getMainShape()->getFrontIndex());
                ear->setIndex(myShape::getCounter());
                ear->setOBB(aShapeMap(i));
                ear->setColor(mControlObj->getMainShape()->getColor());
                mControlObj->getMainShape()->getSlave(leftIndex)->addSlave(ear);
//                DEBUG()<<"leftIndex"<<leftIndex;
                ear->setIsRedisplay(true);
                aFaces = Transverter::section(aShapeMap(i),fc); /* resulting faces */
                if(aFaces.IsNull())
                {
                    WARNING() << "Error: section failed\n";
                    return -1;
                }
                ear->setSection(aFaces);

                TopTools_IndexedMapOfShape aShapeMapedge1;
                TopExp::MapShapes(aFaces, TopAbs_FACE, aShapeMapedge1);
                DEBUG()<<" ---------- left aShapeMapedge1 ---------- "<< i << aShapeMapedge1.Extent();
            }
            if(p.Y() > min_y){
                myShape *ear = new myShape("right");
                ear->initData();
//                ear->setViewer(mControlObj->getMainShape()->getViewer());
                ear->setAisShape(new AIS_ColoredShape(aShapeMap(i)));
                ear->setKey(QString("right ear - %1").arg(i));
                ear->setModelType(EAR);
                ear->setThickness(mControlObj->getMainShape()->getThickness());
                ear->setBendSite(mControlObj->getMainShape()->getSlave(slaveIndex)->getBendSite());
                ear->setRelativeCurrentPosition(mControlObj->getMainShape()->getRelativeCurrentPosition());
                ear->setRotateAngle(mControlObj->getMainShape()->getRotateAngle());
                ear->setFollowIndex(mControlObj->getMainShape()->getFrontIndex());
                ear->setIndex(myShape::getCounter());
                ear->setOBB(aShapeMap(i));
                ear->setColor(mControlObj->getMainShape()->getColor());
                mControlObj->getMainShape()->getSlave(rightIndex)->addSlave(ear);
//                DEBUG()<<"rightIndex"<<rightIndex;
                ear->setIsRedisplay(true);
                aFaces = Transverter::section(aShapeMap(i),fc); /* resulting faces */
                if(aFaces.IsNull())
                {
                    WARNING() << "Error: section failed\n";
                    return -1;
                }
                ear->setSection(aFaces);

                TopTools_IndexedMapOfShape aShapeMapedge1;
                TopExp::MapShapes(aFaces, TopAbs_FACE, aShapeMapedge1);
                DEBUG()<<" ---------- right aShapeMapedge1 ---------- "<< ear->getKey() << aShapeMapedge1.Extent();
            }
        }

    }

    if(ISDEBUG) DEBUG()<<" -- bend 6 -- "<<QTime::currentTime().msec();

    if(angle == 0 ) return 0;

//    DEBUG()<<" bend "<<"angle " <<angle <<" radian "<<radian;

    // updata
    if(qAbs(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle()+angle) > 182){
            WARNING()<<"BendAngle to big!";
            return -1;
    }
//return -1;
    int ret = 0;
//    gp_Ax1 ax(gp_Pnt(0,0,(radian>0?2.0:-2.0)*mControlObj->getMainShape()->getThickness()),gp_Dir(0,1,0));
    for(int index = 0; index < mControlObj->getMainShape()->getSlave(slaveIndex)->getSlaveList().size();index++)
    {
        if(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->getModelType() == CUT)
        {

            //! 旋转切割面

            mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->bend(angle,ax);
//            mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->setBendAngle(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->getBendAngle()+angle);
//            mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->setBendAxis(ax);
            if(ISDEBUG) DEBUG()<< " getBendAngle " << mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->getBendAngle();
            if(ISDEBUG) DEBUG()<<" ax "<<ax.Location().X()<<ax.Location().Y()<<ax.Location().Z();
            ret = createBendAng(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index),
                                mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(index)->getBendAngle()/180*M_PI,
                                ax);
            if(ret == -1){
                WARNING()<<"model has error!";
                return ret;
            }

        }

    }

    for(int index = 0; index < mControlObj->getMainShape()->getSlave(leftIndex)->getSlaveList().size();index++){
        //! 当EAR被附加到左右两侧后才不能被当前边的变换改变（待完成）
        /*if(!mControlObj->getMainShape()->getSlave(leftIndex)->hasModelTypeSlave(CUT))*/{
            if(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getModelType() == EAR &&
                    mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getFollowIndex() ==  mControlObj->getMainShape()->getFrontIndex()
                    ){


//                DEBUG()<< "leftIndex"<<leftIndex;

                mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->bend(angle,ax);
//                mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->setBendAxis(ax);

                if(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getBendSite() == mControlObj->getMainShape()->getRelativeCurrentPosition().X()){
//                    GProp_GProps System;
//                    BRepGProp::LinearProperties(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getAisShape()->Shape(),System);
//                    System.Mass();
//                    gp_Pnt p = System.CentreOfMass();
//                    if(p.X()>0) continue;

//                    mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->setBendAngle(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getBendAngle()+angle);
                    createBendAng(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index),
                                  mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getBendAngle()/180*M_PI,
                                  ax);
                }

                if(ret == -1){
                    WARNING()<<"model has error!";
                    return ret;
                }
            }
        }
    }
//    DEBUG()<< "rightIndex"<<rightIndex;
    for(int index = 0; index < mControlObj->getMainShape()->getSlave(rightIndex)->getSlaveList().size();index++){
        //! 当EAR被附加到左右两侧后才不能被当前边的变换改变（待完成）
        /*if(!mControlObj->getMainShape()->getSlave(rightIndex)->hasModelTypeSlave(CUT))*/{
            if(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getModelType() == EAR &&
                    mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getFollowIndex() ==  mControlObj->getMainShape()->getFrontIndex()
                    ){

                if(ISDEBUG) DEBUG()<< "rightIndex"<<rightIndex;
                mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->bend(angle,ax);
//                mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->setBendAxis(ax);


                if(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getBendSite() == mControlObj->getMainShape()->getRelativeCurrentPosition().X()){
//                    GProp_GProps System;
//                    BRepGProp::LinearProperties(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getAisShape()->Shape(),System);
//                    System.Mass();
//                    gp_Pnt p = System.CentreOfMass();
//                    if(p.X()>0) continue;

//                    mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->setBendAngle(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getBendAngle()+angle);
                    createBendAng(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index),
                                  mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getBendAngle()/180*M_PI,
                                  ax);
                }

                if(ret == -1){
                    WARNING()<<"model has error!";
                    return ret;
                }

            }
        }
    }
    if(ISDEBUG) DEBUG()<<"end";
    return 0;
}

int ControlAction::pressingBend(double angle, gp_Ax1 ax, gp_Vec site, bool flag, baseViewWindow *view)
{
    if(ISDEBUG) DEBUG()<<"entry";
    if(mInit == false)
    {
        DEBUG()<<"ControlAction was not init!";
        return -1;
    }
//    if(angle == 0 ) return 0;
    int ret;
    int  slaveIndex =  mControlObj->getMainShape()->getFrontIndex();
    int leftIndex = -1,rightIndex = -1;
    if(slaveIndex == 0)
    {
        leftIndex = slaveIndex+1;
        rightIndex = mControlObj->getMainShape()->getSlaveList().size()-1;
    }else if(slaveIndex == mControlObj->getMainShape()->getSlaveList().size()-1){
        leftIndex = 0;
        rightIndex = slaveIndex-1;
    }else{
        leftIndex = slaveIndex+1;
        rightIndex = slaveIndex-1;
    }
    //! slaveIndex
    {
        if(qAbs(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle()+angle) > 182){
            angle = angle > 0 ? 180-mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle()
                              :-180-mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle();
        }

        if(flag)
        {
            DEBUG()<< "///***  flag"<<flag;
            mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->translation(site);
            gp_Vec xyz = mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getRelativeCurrentPosition();
            xyz.SetZ(0);
            mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->setRelativeCurrentPositionForBatch(xyz);

        }

        mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->bend(angle,ax);
        //    mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->setBendAngle(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle()+angle);

        if(ISDEBUG || 1) DEBUG()<< " getBendAngle 2 " << mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle();
        if(ISDEBUG) DEBUG()<<" ax "<<ax.Location().X()<<ax.Location().Y()<<ax.Location().Z();
        ret = createBendAng(mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT),
                                mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->getBendAngle()/180*M_PI,
                                ax);
        mControlObj->getMainShape()->getSlave(slaveIndex)->getSlave(CUT)->setIsBlankPressing(true);
    }

    //! leftIndex
    for(int index = 0; index < mControlObj->getMainShape()->getSlave(leftIndex)->getSlaveList().size();index++){
        //! 当EAR被附加到左右两侧后才不能被当前边的变换改变（待完成）
        /*if(!mControlObj->getMainShape()->getSlave(leftIndex)->hasModelTypeSlave(CUT))*/{
            if(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getModelType() == EAR &&
                    mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getFollowIndex() ==  mControlObj->getMainShape()->getFrontIndex()
                    ){
                DEBUG()<< "///***  leftIndex"<<leftIndex;
                if(flag)
                {
                    DEBUG()<< "///***  flag1"<<flag << site.X()<<site.Y()<<site.Z();
                    mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->translation(site);
                    gp_Vec xyz = mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getRelativeCurrentPosition();
                    xyz.SetZ(0);
                    mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->setRelativeCurrentPositionForBatch(xyz);
                    DEBUG()<< "///***  flag1"<<flag << mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getRelativeCurrentPosition().X()<<mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getRelativeCurrentPosition().Y()<<mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getRelativeCurrentPosition().Z();
                }

                mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->bend(angle,ax);
                createBendAng(mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index),
                              mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->getBendAngle()/180*M_PI,
                              ax);
                mControlObj->getMainShape()->getSlave(leftIndex)->getSlave(index)->setIsBlankPressing(true);

            }
        }
    }
    //! rightIndex
    for(int index = 0; index < mControlObj->getMainShape()->getSlave(rightIndex)->getSlaveList().size();index++){
        //! 当EAR被附加到左右两侧后才不能被当前边的变换改变（待完成）
        /*if(!mControlObj->getMainShape()->getSlave(rightIndex)->hasModelTypeSlave(CUT))*/{
            if(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getModelType() == EAR &&
                    mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getFollowIndex() ==  mControlObj->getMainShape()->getFrontIndex()
                    ){

                DEBUG()<< "****/// rightIndex"<<rightIndex;
                if(flag)
                {
                    DEBUG()<< "///***  flag1"<<flag << site.X()<<site.Y()<<site.Z();
                    mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->translation(site);
                    gp_Vec xyz = mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getRelativeCurrentPosition();
                    xyz.SetZ(0);
                    mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->setRelativeCurrentPositionForBatch(xyz);
                }

                mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->bend(angle,ax);
                createBendAng(mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index),
                              mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->getBendAngle()/180*M_PI,
                              ax);
                mControlObj->getMainShape()->getSlave(rightIndex)->getSlave(index)->setIsBlankPressing(true);

            }
        }
    }

    return ret;
}

int  ControlAction::translation(gp_Vec dest/*目标点位*/)
{
    if(mInit == false)
    {
        DEBUG()<<"ControlAction was not init!";
        return -1;
    }

//    DEBUG()<<"-----" <<mControlObj->getMainShape()->getKey();
    mControlObj->getMainShape()->translation(dest);

    return 0;
}

int ControlAction::rotation(double angle, gp_Ax1 axis, bool copy)
{
    if(mInit == false)
    {
        DEBUG()<<__FILE__<<__FUNCTION__<<"ControlAction was not init!";
        return -1;
    }

//    gp_Ax1 axis(gp_Pnt(mControlObj->getMainShape()->getRelativeCurrentPosition().X(),
//                       mControlObj->getMainShape()->getRelativeCurrentPosition().Y(),
//                       mControlObj->getMainShape()->getThickness()/2),
//                gp_Dir(dir.x(), dir.y(), dir.z()));


    mControlObj->getMainShape()->rotation(angle,axis,copy);

    if(mControlObj->getMainShape()->getKey() == "model"){
        if(mControlObj->getMainShape()->getLastState() == ROTATION){
            for(int i = 0; i < mControlObj->getMainShape()->getSlaveList().size();i++){
                mControlObj->getMainShape()->getSlave(i)->setIsFront(false);
//                DEBUG()<< i <<" IsFront "<<mControlObj->getMainShape()->getSlave(i)->getIsFront();
//                DEBUG()<<" MarkAngle "<<mControlObj->getMainShape()->getSlave(i)->getMarkAngle()
//                        <<mControlObj->getMainShape()->getRotateAngle();
                if(mControlObj->getMainShape()->getSlave(i)->getMarkAngle() ==
                        mControlObj->getMainShape()->getRotateAngle()){
                    mControlObj->getMainShape()->getSlave(i)->setIsFront(true);
                    mControlObj->getMainShape()->setFrontIndex(i);
//                     DEBUG()<< i <<" IsFront "<<mControlObj->getMainShape()->getSlave(i)->getIsFront();
                }
            }
        }

    }


    return 0;
}

int ControlAction::createBendAng(myShape* myshape, double radian, gp_Ax1 ax)
{
    Transverter transverter;
    TopoDS_Shape faceShape;
    gp_Dir dir(1,0,0);
//    gp_Ax1 ax(gp_Pnt(0,0,(radian>0?2.0:-2.0)*myshape->getThickness()),gp_Dir(0,1,0));
    dir.Rotate(ax,myshape->getBendAngle()/180*M_PI);


    if(ISDEBUG) DEBUG()<<" dir "<<dir.X()<<dir.Y()<<dir.Z();

    faceShape = _bendAngle(myshape->getSection(), radian, ax);
//    DEBUG()<<"faceShape"<<faceShape.NbChildren();
//faceShape = TopoDS::Face(mapShape(1));
    if(ISDEBUG) DEBUG()<<" --------- radian -------- "<<radian;

    if(myshape->hasModelTypeSlave(BENDANGLE)){
        for(int i = 0; i < myshape->getSlaveList().size();i++){
            if(myshape->getSlave(i)->getModelType() == BENDANGLE){
//                DEBUG()<<"BENDANGLE  ";
                myshape->getSlave(i)->setLastState(BEND);
                myshape->getSlave(i)->getAisShape()->Set(faceShape);
                myshape->getSlave(i)->setIsRedisplay(true);
            }
        }
    }else{
//        DEBUG()<<"new BENDANGLE  ";
        myShape *SlaveMain = new myShape("SlaveBend");
        SlaveMain->initData();
//        SlaveMain->setViewer(mControlObj->getMainShape()->getViewer());
        SlaveMain->setAisShape(new AIS_ColoredShape(faceShape));
//        DEBUG()<<"getAisShape"<<SlaveMain->getAisShape()->Shape().NbChildren();
        SlaveMain->setKey("SlaveBend");
        SlaveMain->setModelType(BENDANGLE);
        SlaveMain->setThickness(mControlObj->getMainShape()->getThickness());
        SlaveMain->setBendSite(myshape->getBendSite());
        SlaveMain->setBendAngle(myshape->getBendAngle());
        SlaveMain->setBendAxis(myshape->getBendAxis());
        SlaveMain->setRelativeCurrentPosition(mControlObj->getMainShape()->getRelativeCurrentPosition());
        SlaveMain->setRotateAngle(mControlObj->getMainShape()->getRotateAngle());
        SlaveMain->setLastState(BEND);
        SlaveMain->setOBB(faceShape);
        SlaveMain->setIsRedisplay(true);
        SlaveMain->setColor(mControlObj->getMainShape()->getColor());
        myshape->addSlave(SlaveMain);
//        DEBUG()<<" bend index"<<SlaveMain->getIndex();
    }
    return 0;
}

int ControlAction::_translation( TopoDS_Shape &shape, gp_Vec vec)
{
    gp_Trsf trsf;
    trsf.SetTranslation(vec);
//    shape = BRepBuilderAPI_Transform(shape,trsf);
    shape.Move(TopLoc_Location(trsf));

    return 0;
}

TopoDS_Shape ControlAction::_bendAngle(const TopoDS_Shape& shape, double radian, gp_Ax1 ax)
{
    if(shape.IsNull() && shape.ShapeType() != TopAbs_FACE)
    {
        DEBUG()<<__FILE__<<__FUNCTION__<<"shape is not Face!";
        return shape;
    }

    if(radian == 0)
        return shape;

    if(ISDEBUG) DEBUG()<<"_bendAngle radian"<<radian;
    TopoDS_Shape ret;
//    gp_Ax1 ax(gp_Pnt(0,0,(radian>0?2.0:-2.0)*mControlObj->getMainShape()->getThickness()),gp_Dir(0,1,0));
    try{
        ret = BRepPrimAPI_MakeRevol(shape,ax,radian).Shape();
    }catch(...){
        DEBUG()<<" error!";
    }

    return ret;
}

int ControlAction::_rotation(TopoDS_Shape &shape, double radian, gp_Ax1 axis)
{
    gp_Trsf trsf;
    trsf.SetRotation(axis,radian);
//    shape = BRepBuilderAPI_Transform(shape,trsf);
    shape.Move(TopLoc_Location(trsf));

    return 0;
}
