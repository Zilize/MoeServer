#ifndef MAINTHREAD_H
#define MAINTHREAD_H
#include <QThread>
#include <mutex>
#include <thread>
using namespace std;

class MainThread : public QThread
{
    Q_OBJECT

public:
    MainThread(QString ipQString, QString portQString, QString dirQString);
    void emitor(QString info);
    static void handle_clnt(int clnt_sock, MainThread *mt);
    void stop_proxy();

public:
    virtual void run();

signals:
    void threadInfoSender(QString info);

public:
    QString ipQS;
    QString portQS;
    QString dirQS;
    int clnt_cnt = 0;
    int clnt_socks[256];
    int serveSock;
    int thread_cnt = 0;
    std::thread::id threads[256];
    std::mutex mutexSock;
    std::mutex mutexThread;


};

#endif // MAINTHREAD_H
