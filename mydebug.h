#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>
#include <QTime>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#ifdef PRO
#define ENDSTR  0;
#else
#define ENDSTR  1;
#endif

#define ISDEBUG false
#define ISINFO false

#ifndef QT_DEBUG
#define DEBUG() qDebug()<<"\n"<<QTime::currentTime().toString("hh:mm:ss.zzz")<<QThread::currentThreadId()<<__FILE__<<" "<<__FUNCTION__<<" "<<__LINE__<<"\n"
#define INFO() qInfo()<<"\n"<<QThread::currentThreadId()<<__FILE__<<" "<<__FUNCTION__<<" "<<__LINE__<<"\n"
#define CRITICAL() qCritical()<<"\n"<<QThread::currentThreadId()<<__FILE__<<" "<<__FUNCTION__<<" "<<__LINE__<<"\n"
#define WARNING() qWarning()<<"\n"<<QThread::currentThreadId()<<__FILE__<<" "<<__FUNCTION__<<" "<<__LINE__<<"\n"
#else
#define DEBUG() qDebug()<<"\n"
#define INFO() qInfo()<<"\n"
#define CRITICAL() qCritical()<<"\n"
#define WARNING() qWarning()<<"\n"
#endif

#define TEST_DISPLAY 0

#endif // MYDEBUG_H
