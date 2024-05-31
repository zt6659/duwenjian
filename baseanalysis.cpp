#include "baseanalysis.h"
#ifndef PRO
#include "3D_simulation/common.h"
#else
#include "common.h"
#endif
baseAnalysis::baseAnalysis(QObject *parent) : QObject(parent)
{

}

baseAnalysis::~baseAnalysis()
{

}

QMap<QString, double> baseAnalysis::getData()
{
    QMap<QString, double> tmp = mAxialMotionData;
    mAxialMotionData.clear();

    return tmp;
}

void baseAnalysis::analysisGCode(QString GCode)
{
    QStringList strList = GCode.split(" ");
    if(strList.size() == 1){
        strList = QString(strList.at(0)).split("\n");
        if(strList.size() == 2)
            strList.pop_back();
    }
    if((strList.at(0) != "G0"
        && strList.at(0) != "G1"
        && strList.at(0) != "G99"
        && strList.at(0) != "M100")
//    && strList.at(0) != "#"
    || (strList.at(0) == "#" && strList.at(1) == "CPARAS"))
    {
        return;
    }
    for(int i = 0;i < strList.size() ;i++){
        QString str = strList.at(i);
        QString axis;
        if(str.at(0).isLetter()){
            axis = str.at(0);

        }else if(strList.at(0) == "#"){
            if(0 == i) continue;

        }else{
            axis += str.at(0);
            axis += str.at(1);
        }

        QString value = str.split(axis).last();
        mAxialMotionData.insert(axis,value.toDouble());
    }
}
