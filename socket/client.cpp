/*
 * 程序名：client.cpp，此程序用于演示socket的客户端
*/
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;
 
int main(int argc,char *argv[])
{

  // 第1步：创建客户端的socket。  
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if (sockfd==-1)
  {
    perror("socket"); return -1;
  }
 
  // 第2步：向服务器发起连接请求。 
  struct hostent* h;    // 用于存放服务端IP的结构体。
  if ( (h = gethostbyname("192.168.44.129")) == 0 )  // 把字符串格式的IP转换成结构体。
  { 
    cout << "gethostbyname failed.\n" << endl; close(sockfd); return -1;
  }
  struct sockaddr_in servaddr;              // 用于存放服务端IP和端口的结构体。
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  memcpy(&servaddr.sin_addr,h->h_addr,h->h_length); // 指定服务端的IP地址。
  servaddr.sin_port = htons(atoi("5005"));         // 指定服务端的通信端口。
  
  if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))!=0)  // 向服务端发起连接清求。
  { 
    perror("connect"); close(sockfd); return -1; 
  }
  
  // 第3步：与服务端通讯，客户发送一个请求报文后等待服务端的回复，收到回复后，再发下一个请求报文。
  //char buffer[1024];
  std::string buffer;
  for(int j=0;j<10;j++)
  {
  for (int ii=0;ii<1000;ii++)  // 循环3次，将与服务端进行三次通讯。
  {
    int iret;
    //memset(buffer,0,sizeof(buffer));
    buffer.resize(1024);
    // 接收服务端的回应报文，如果服务端没有发送回应报文，recv()函数将阻塞等待。
    if ( (iret=recv(sockfd,buffer.data(),1024,0))<=0)
    {
       
    }
    //cout << "接收：" << buffer << endl;
  }
  cout<<buffer<<endl;
  }
  // memset(buffer,0,sizeof(buffer));
  // sprintf(buffer,"OK");
  // int iret;
  // if ( (iret=send(sockfd,buffer,strlen(buffer),0))<=0)
  //   { 
  //     perror("send");  
  //   }
  // cout << "发送：" << buffer << endl;
  // 第4步：关闭socket，释放资源。
  close(sockfd);
}