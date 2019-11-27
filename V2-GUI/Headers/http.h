#ifndef HTTP_H
#define HTTP_H
#include <string>
using namespace std;

class HTTP
{
public:
    string message; //客户端发送的报文内容
    HTTP(char * msg);  //构造对象
    void printHTTP();  //打印HTTP报文
    string getCommand(); //获取请求命令行
    string getMethod();  //获取请求方式
    string getPath();  //获取路径
    string getType();  //获取类型
    string response(int clnt_sock, string ROOT_PATH);  //构造响应报文并发送
};

#endif // HTTP_H
