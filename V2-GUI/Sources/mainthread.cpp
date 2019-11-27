#include "mainthread.h"
#include "mainwindow.h"
#include "http.h"
#include <QDebug>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#define BUF_SIZE 1024
using namespace std;

MainThread::MainThread(QString ipQString, QString portQString, QString dirQString)
{
    ipQS = ipQString;
    portQS = portQString;
    dirQS = dirQString;
}

void MainThread::run()
{
    string ipString = ipQS.toStdString();
    const char* ipChar = ipString.c_str();
    string portString = portQS.toStdString();
    const char* portChar = portString.c_str();
    int port = stoi(portChar, 0, 10);

    int serv_sock, clnt_sock, option;

    struct sockaddr_in serv_adr;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz, optlen;
    std::thread::id t_id;  //线程ID

    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        emitor("[ERROR] Function socket() error.\n");

    mutexSock.lock();
    serveSock = serv_sock;  //放到全局变量中方便关闭
    mutexSock.unlock();
    optlen = sizeof(option);
    option = true;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);  //设置端口为可重用，避免绑定失败

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(ipChar);
    serv_adr.sin_port = htons(port);

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        emitor("[ERROR] Function bind() error.\n");

    if(listen(serv_sock, 20) == -1)
        emitor("[ERROR] Function listen() error.\n");

    while(1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);  //accept是阻塞函数
        if(clnt_sock == -1)
            emitor("[ERROR] Function accept() error.\n");

        mutexSock.lock();
        clnt_socks[clnt_cnt++] = clnt_sock;
        mutexSock.unlock();

        std::thread clnt_thread(&MainThread::handle_clnt, clnt_sock, this);
        mutexThread.lock();
        threads[thread_cnt++] = t_id;
        mutexThread.unlock();
        clnt_thread.detach();
    }
}

void MainThread::stop_proxy()
{
    int clnt_sock, i;
    mutexSock.lock();
    close(serveSock);
    for (i = 0; i < clnt_cnt; i++) {
        clnt_sock = clnt_socks[i];
        close(clnt_sock);
    }
    clnt_cnt = 0;
    mutexSock.unlock();
}

void MainThread::emitor(QString info)
{
    emit threadInfoSender(info);
}

void MainThread::handle_clnt(int clnt_sock, MainThread *mt)
{
    std::thread::id self = std::this_thread::get_id();
    struct sockaddr_in cur_adr;  //当前sock地址
    socklen_t cur_adr_sz;  //当前sock地址的长度
    cur_adr_sz = sizeof(cur_adr);

    getpeername(clnt_sock, (struct sockaddr*)&cur_adr, &cur_adr_sz);  //获得客户端的信息

    int str_len, i;
    char message[BUF_SIZE];
    string ROOT_PATH = mt->dirQS.toStdString();

    str_len = recv(clnt_sock, message, BUF_SIZE, 0);  //接收客户报文
    message[str_len] = 0;
    HTTP http(message);
    string command = http.getCommand();
    string resStatus = http.response(clnt_sock, ROOT_PATH);  //发送响应并返回响应状态说明字符串

    ostringstream infoStream;
    infoStream << "[REQUEST] Client IP: " << inet_ntoa(cur_adr.sin_addr) << " Port: " << cur_adr.sin_port << " Request Command: \"" << command << "\" Response Status: " << resStatus << endl;
    string infoString = infoStream.str();
    QString infoQString = QString::fromStdString(infoString);
    mt->emitor(infoQString);
    mt->mutexSock.lock();
    for(i = 0; i < mt->clnt_cnt; i++) { //已关闭的套接字需要移除
        if (clnt_sock == mt->clnt_socks[i]) {
            while (i < mt->clnt_cnt - 1) {
                mt->clnt_socks[i] = mt->clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    mt->clnt_cnt--;
    mt->mutexSock.unlock();
    mt->mutexThread.lock();
    for (i = 0; i < mt->thread_cnt; i++) { //已关闭的线程需要移除
        if (self == mt->threads[i]) {
            while (i < mt->thread_cnt - 1) {
                mt->threads[i] = mt->threads[i+1];
                i++;
            }
            break;
        }
    }
    mt->thread_cnt--;
    mt->mutexThread.unlock();
    close(clnt_sock);  //不关闭套接字就无法发送成功
}
