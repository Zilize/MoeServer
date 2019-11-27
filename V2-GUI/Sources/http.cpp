#include "http.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <limits>
#define BUF_SIZE 1024
#define SMALL_BUF 100
#define MAX_CLNT 256
#define MAX_PTHD 256
using namespace std;

HTTP::HTTP(char * msg) {
    message = msg;
}
void HTTP::printHTTP() {
    cout << message << endl;
}
string HTTP::getCommand() {
    int len = message.find_first_of('\r');
    return message.substr(0, len);
}
string HTTP::getMethod() {
    int len = message.find_first_of(' ');
    return message.substr(0, len);
}
string HTTP::getPath() {
    int h = message.find_first_of('/');
    int t = message.find_first_of(' ', h);
    return message.substr(h, t-h);
}
string HTTP::getType() {
    if (this->getPath().length() == 1) return "";  //访问根目录的情况
    string path = this->getPath();
    string fileName = path.substr(path.find_last_of('/') + 1, path.length() - path.find_last_of('/'));  //获得文件名
    string contentType = fileName.substr(fileName.find_last_of('.') + 1, fileName.length() - fileName.find_last_of('.'));  //获得后缀
    if (contentType == "html") return "text/html";
    if (contentType == "js") return "text/javascript";
    if (contentType == "css") return "text/css";
    if (contentType == "jpg") return "image/jpg";
    if (contentType == "jpeg") return "image/jpeg";
    if (contentType == "png") return "image/png";
    if (contentType == "gif") return "image/gif";
    if (contentType == "ico") return "image/x-icon";
    if (contentType == "mp3") return "audio/mp3";
    if (contentType == "mp4") return "video/mpeg4";
    if (contentType == "pdf") return "application/pdf";
    return "*/*";
}
string HTTP::response(int clnt_sock, string ROOT_PATH) {  //返回处理结果字符串
    if (this->getMethod() != "GET") {  //方法不允许
        char resStatus[] = "HTTP/1.1 405 Method Not Allowed\r\n";
        char resCon[] = "Connection: keep-alive\r\n";
        char resConLen[] = "Content-Length: 77\r\n";
        char resConType[] = "Content-Type: text/html\r\n\r\n";
        char resContent[] = "<html><head><title>Error</title></head><body>Method Not Allowed</body></html>";
        send(clnt_sock, resStatus, strlen(resStatus), 0);
        send(clnt_sock, resCon, strlen(resCon), 0);
        send(clnt_sock, resConLen, strlen(resConLen), 0);
        send(clnt_sock, resConType, strlen(resConType), 0);
        send(clnt_sock, resContent, strlen(resContent), 0);
        return "Method Not Allowed.";
    }
    fstream inFile;
    string path = this->getPath();
    string fileName = path.substr(path.find_last_of('/') + 1, path.length() - path.find_last_of('/'));  //获得文件名
    if (fileName.find_first_of('.') == -1) {  //如果请求不是文件形式则返回404
        string file404 = "/404.html";
        string pathString = ROOT_PATH + file404;
        const char * pathChar = pathString.data();
        inFile.open(pathChar, ios::in|ios::binary);
        if (!inFile.is_open()) {
            cout << "404 Page Not found" << endl;
            return "404 Page Not Found";
        }
        inFile.ignore(numeric_limits<streamsize>::max());   ////////////////////
        char resConLen[SMALL_BUF];
        int fileLen = inFile.gcount();
        sprintf(resConLen, "Content-Length: %d\r\n", fileLen);
        char * fileBuf = (char *)malloc(fileLen);
        inFile.seekg(0);
        inFile.read(fileBuf, fileLen);
        char resStatus[] = "HTTP/1.1 404 Not Found\r\n";
        char resCon[] = "Connection: keep-alive\r\n";
        char resConType[] = "Content-Type: text/html\r\n\r\n";
        send(clnt_sock, resStatus, strlen(resStatus), 0);
        send(clnt_sock, resCon, strlen(resCon), 0);
        send(clnt_sock, resConLen, strlen(resConLen), 0);
        send(clnt_sock, resConType, strlen(resConType), 0);
        send(clnt_sock, fileBuf, fileLen, 0);
        free(fileBuf);
        return "Not request for a file.";
    }
    string pathString = ROOT_PATH + this->getPath();
    const char * pathChar = pathString.data();
    inFile.open(pathChar, ios::in|ios::binary);
    if (!inFile.is_open()) {  //找不到文件返回404
        string file404 = "/404.html";
        pathString = ROOT_PATH + file404;
        const char * pathChar404 = pathString.data();
        inFile.open(pathChar404, ios::in|ios::binary);
        if (!inFile.is_open()) {
            cout << "404 Page Not found" << endl;
            return "404 Page Not Found";
        }
        inFile.ignore(numeric_limits<streamsize>::max());
        char resConLen[SMALL_BUF];
        int fileLen = inFile.gcount();
        sprintf(resConLen, "Content-Length: %d\r\n", fileLen);
        char * fileBuf = (char *)malloc(fileLen);
        inFile.seekg(0);
        inFile.read(fileBuf, fileLen);
        char resStatus[] = "HTTP/1.1 404 Not Found\r\n";
        char resCon[] = "Connection: keep-alive\r\n";
        char resConType[] = "Content-Type: text/html\r\n\r\n";
        send(clnt_sock, resStatus, strlen(resStatus), 0);
        send(clnt_sock, resCon, strlen(resCon), 0);
        send(clnt_sock, resConLen, strlen(resConLen), 0);
        send(clnt_sock, resConType, strlen(resConType), 0);
        send(clnt_sock, fileBuf, fileLen, 0);
        free(fileBuf);
        return "File Not Found.";
    }
    //现在返回找到的文件
    char resStatus[] = "HTTP/1.1 200 OK\r\n";
    char resCon[] = "Connection: keep-alive\r\n";
    inFile.ignore(numeric_limits<streamsize>::max());
    char resConLen[SMALL_BUF];
    int fileLen = inFile.gcount();
    sprintf(resConLen, "Content-Length: %d\r\n", fileLen);
    char * fileBuf = (char *)malloc(fileLen);
    inFile.seekg(0);
    inFile.read(fileBuf, fileLen);
    char resConType[SMALL_BUF];
    sprintf(resConType, "Content-Type: %s\r\n\r\n", this->getType().data());
    send(clnt_sock, resStatus, strlen(resStatus), 0);
    send(clnt_sock, resCon, strlen(resCon), 0);
    send(clnt_sock, resConLen, strlen(resConLen), 0);
    send(clnt_sock, resConType, strlen(resConType), 0);
    send(clnt_sock, fileBuf, fileLen, 0);
    free(fileBuf);
    return "A successful response.";
}
