#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#define BUF_SIZE 1024
#define SMALL_BUF 100
#define MAX_CLNT 256
#define MAX_PTHD 256
using namespace std;

//config
#define LISTEN_IP "0.0.0.0"
#define LISTEN_PORT 9000
#define ROOT_PATH "/home/wwwroot/workspace"

void * handle_clnt(void * arg);
void * handle_serv(void * arg);
int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
int serveSock;
int pthd_cnt = 0;
pthread_t pthds[MAX_PTHD];
pthread_mutex_t mutx;  //指示客户套接字
pthread_mutex_t mutxp;  //指示线程数组

class HTTP{
	public:
		string message; //客户端发送的报文内容
		HTTP(char * msg);  //构造对象
		void printHTTP();  //打印HTTP报文
		string getCommand(); //获取请求命令行
		string getMethod();  //获取请求方式
		string getPath();  //获取路径
		string getType();  //获取类型
		string response(int clnt_sock);  //构造响应报文并发送
};
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
string HTTP::response(int clnt_sock) {  //返回处理结果字符串
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
		inFile.ignore(numeric_limits<streamsize>::max());  ////////////////////
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
	inFile.ignore(numeric_limits<streamsize>::max());  ////////////////////
	char resConLen[SMALL_BUF];
	int fileLen = inFile.gcount();
	sprintf(resConLen, "Content-Length: %d\r\n", fileLen);
	char * fileBuf = (char *)malloc(fileLen);  //注意释放！！！！！！！！！！！！！！！
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

int main(int argc, char *argv[]){
	pthread_t t_id;  //线程ID
	pthread_mutex_init(&mutx, NULL);  //mutx信号灯初始化
	pthread_mutex_init(&mutxp, NULL);  //mutxp信号灯初始化
	string command;
	int flag = 0, i;  //服务器是否开启
	cout << "Enter s to start, enter q to quit, enter e to exit." << endl;
	cin >> command;
	while(1) {
		if (command == "e") exit(0);
		else if (command == "s" && flag == 0) { //开启服务器线程
			flag = 1;
			cout << "START SERVING!" << endl;
			pthread_create(&t_id, NULL, handle_serv, NULL);
			pthread_mutex_lock(&mutxp);
			pthds[pthd_cnt++] = t_id;  //将服务器线程加入线程数组
			pthread_mutex_unlock(&mutxp);
			pthread_detach(t_id);
			cin >> command;
		}
		else if (command == "s" && flag == 1) {
			cout << "THE SERVER IS ON!" << endl;
			cin >> command;
		}
		else if (command == "q" && flag == 0) {
			cout << "THE SERVER IS OFF!" << endl;
			cin >> command;
		}
		else if (command == "q" && flag == 1) { //记得关闭servsock！！！！！
			flag = 0;
			cout << "STOP SERVING" << endl;
			int clnt_sock;
			pthread_mutex_lock(&mutxp);
			for (i = 0; i < pthd_cnt; i++) {
				t_id = pthds[i];
				pthread_cancel(t_id);
			}
			pthd_cnt = 0;
			pthread_mutex_unlock(&mutxp);
			pthread_mutex_lock(&mutx);
			close(serveSock);
			for (i = 0; i < clnt_cnt; i++) {
				clnt_sock = clnt_socks[i];
				close(clnt_sock);
			}
			clnt_cnt = 0;
			pthread_mutex_unlock(&mutx);
			cin >> command;
		}
		else {
			cout << "COMMAND ERROR" << endl;
			cin >> command;
		}
	}
	return 0;
}

void * handle_serv(void * arg) {
	int serv_sock, clnt_sock, option;
	
	struct sockaddr_in serv_adr;
	struct sockaddr_in clnt_adr;
	socklen_t clnt_adr_sz, optlen;
	pthread_t t_id;  //线程ID

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(serv_sock == -1)
		cout << "socket() error" << endl;
	
	pthread_mutex_lock(&mutx);
	serveSock = serv_sock;  //放到全局变量中方便关闭
	pthread_mutex_unlock(&mutx);
	optlen = sizeof(option);
	option = true;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);  //设置端口为可重用，避免绑定失败
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(LISTEN_IP);
	serv_adr.sin_port = htons(LISTEN_PORT);

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		cout << "bind() error" << endl;
	
	if(listen(serv_sock, 20) == -1)
		cout << "listen() error" << endl;

	while(1) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);  //accept是阻塞函数
		pthread_testcancel();  //pthread_cancel使阻塞函数返回错误码，此处需要设置退出点testcancel
		if(clnt_sock == -1)
			cout << "accept() error" << endl;

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)(clnt_socks + clnt_cnt - 1));
		pthread_mutex_lock(&mutxp);
		pthds[pthd_cnt++] = t_id;  //将客户处理器线程加入线程数组
		pthread_mutex_unlock(&mutxp);
		pthread_detach(t_id);
	}
	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg) {  //一个无状态的客户请求处理器
	int clnt_sock=*((int*)arg);
	pthread_t self = pthread_self();
	struct sockaddr_in cur_adr;  //当前sock地址
	socklen_t cur_adr_sz;  //当前sock地址的长度
	cur_adr_sz = sizeof(cur_adr);

	getpeername(clnt_sock, (struct sockaddr*)&cur_adr, &cur_adr_sz);  //获得客户端的信息

	int str_len, i;
	char message[BUF_SIZE];

	str_len = recv(clnt_sock, message, BUF_SIZE, 0);  //接收客户报文
	message[str_len] = 0;
	HTTP http(message);
	string command = http.getCommand();
	string resStatus = http.response(clnt_sock);  //发送响应并返回响应状态说明字符串

	pthread_mutex_lock(&mutx);
	cout << "Client IP: " << inet_ntoa(cur_adr.sin_addr) << " Port: " << cur_adr.sin_port << " Request Command: \"" << command << "\" Response Status: " << resStatus << endl;
	for(i = 0; i < clnt_cnt; i++) { //已关闭的套接字需要移除
		if (clnt_sock == clnt_socks[i]) {
			while (i < clnt_cnt - 1) {
				clnt_socks[i] = clnt_socks[i+1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	pthread_mutex_lock(&mutxp);
	for (i = 0; i < pthd_cnt; i++) { //已关闭的线程需要移除
		if (self == pthds[i]) {
			while (i < pthd_cnt - 1) {
				pthds[i] = pthds[i+1];
				i++;
			}
			break;
		}
	}
	pthd_cnt--;
	pthread_mutex_unlock(&mutxp);
	close(clnt_sock);  //不关闭套接字就无法发送成功
	return NULL;
}