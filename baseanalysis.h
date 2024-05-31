#ifndef BASEANALYSIS_H
#define BASEANALYSIS_H

#include <QObject>
#include <QMap>

class baseAnalysis : public QObject
{
    Q_OBJECT
public:
    explicit baseAnalysis(QObject *parent = 0);
    ~baseAnalysis();

    void analysisGCode(QString GCode);

    QMap<QString,double> getData();

signals:

public slots:

private:
    QMap<QString,double> mAxialMotionData;

};

#endif // BASEANALYSIS_H
