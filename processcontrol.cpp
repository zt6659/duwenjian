#include "processcontrol.h"
#include "mydebug.h"

processControl::processControl(QObject *parent) : QObject(parent)
{

}

processControl::~processControl()
{

}

void processControl::push(int index)
{
    QMutexLocker mloc(&mMutex);
    mExecuteQueues.append(index);
}

int processControl::pop()
{
    QMutexLocker mloc(&mMutex);
    if(ISDEBUG) DEBUG()<<mExecuteQueues.first();
    return mExecuteQueues.takeFirst();
}

void processControl::slot_execute(int index)
{
//    if(pTask == nullptr) return;
    if(mLastIndex == index) return;
    if(ISDEBUG) DEBUG()<<"index"<<index;

    push(index);
    mLastIndex = index;
}
#include <QTime>
void processControl::slot_threadFinished()
{
    INFO()<<"slot_threadFinished";
    QMutexLocker mloc(&mMutex);
    mFinished = true;
    pTask = nullptr;
    mCurstate = IDLE;
    mLastIndex = -1;
    mExecuteQueues.clear();
    mTimeCount = 0;
    if(ISDEBUG) DEBUG()<<QTime::currentTime();
}

void processControl::slot_taskThread()
{
    if(pTask == nullptr){
        pTask = new AsyncTask();
        connect(pTask,SIGNAL(finished()),this,SLOT(slot_threadFinished()));
        if(ISDEBUG) DEBUG()<< "slot_taskThread" << "pTask"<<pTask;
        mFinished = false;
        mCurstate = IDLE;
        pTask->setTaskRunnable([=]{
            if(ISDEBUG) DEBUG()<<" 1 "<<QTime::currentTime();
            while(1){
                mMutex.lock();
                int state = mCurstate;
                mMutex.unlock();

                if(mExecuteQueues.size() > 0 && state == IDLE){
//                    DEBUG()<<" 1 "<<QTime::currentTime();
                    //! 发生信号
                    if(ISDEBUG) DEBUG()<<" signal_ControlIndex ";
                    mLastSendIndex = pop();
                    emit signal_ControlIndex(mLastSendIndex);
                    if(ISDEBUG) DEBUG()<<" signal_ControlIndex end";
                    mMutex.lock();
                    mCurstate = BUSY;
                    mMutex.unlock();
                    mTimeCount = 0;
//                    DEBUG()<<" 2 "<<QTime::currentTime();
                }



                QThread::usleep(4);

                if(get_zwj_statue()==IDLE_STATUE){      //人工取消
                    mFinished = true;
                }

                if(mFinished) break;

//                if(state == BUSY && mTimeCount > 100000){
//                    DEBUG()<<" timeout ";
//                    emit signal_ControlIndex(mLastSendIndex);
//                    mTimeCount = 0;
//                }

//                mTimeCount++;
            }
        });
        // 启动异步任务
        pTask->execute();
    }
}

void processControl::slot_ChangeState(int state)
{
    if(ISDEBUG) DEBUG()<<" entry ";
    QMutexLocker mloc(&mMutex);
    mCurstate = state;
}
