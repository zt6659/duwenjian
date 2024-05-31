#ifndef PROCESSCONTROL_H
#define PROCESSCONTROL_H

#include <QObject>
#include <QList>
#include <QDebug>
#include <QMutex>
#include <QThread>
#ifndef PRO
#include "handwheeloperation/utilities/asynctask/asynctask.h"
#include "3D_simulation/common.h"
#else
#include "motioncontrol/asynctask/asynctask.h".
#include "common.h"
#endif


class processControl : public QObject
{
    Q_OBJECT
public:
    explicit processControl(QObject *parent = 0);
    ~processControl();

    void push(int index);
    int pop();

signals:
    void signal_ControlIndex(int index);
public slots:
    void slot_execute(int index);
    void slot_threadFinished();
    void slot_taskThread();

    void slot_ChangeState(int state);
private:
    AsyncTask* pTask = nullptr;

    //! 执行队列 Execute Queues
    QList<int> mExecuteQueues;
    //! 当前执行状态
    int mCurstate                 = IDLE;
    //! 执行序列中最后一个序号
    int mLastIndex                = -1;
    //! 提前结束标志
    bool mFinished                = false;
    //! 最后一次发送的执行序号
    int mLastSendIndex            = -1;
    //! 计时器
    int mTimeCount                = 0;
    //! 线程锁
    QMutex mMutex;
};

#endif // PROCESSCONTROL_H
