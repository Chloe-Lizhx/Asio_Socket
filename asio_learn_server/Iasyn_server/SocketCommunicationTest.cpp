#include <iostream>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <memory>
#include "SocketCommunication.hpp"
#include "Communication.hpp"
using namespace com;
unsigned short port = 0;
bool reuseAddress = false;
std::string networkName = "lo";
std::string addressDirectory = ".";
    
int main()
{
    std::shared_ptr<Communication> comm ;
    std::shared_ptr<Communication> comm1 ;
    std::shared_ptr<SocketCommunication> socketcomm(new SocketCommunication(port,reuseAddress,networkName,addressDirectory));
    std::shared_ptr<SocketCommunication> socketcomm1(new SocketCommunication(port,reuseAddress,networkName,addressDirectory));
    comm = socketcomm;
    comm1 = socketcomm1;
    std::string acceptorName = "lzx";
    std::string requesterName = "xzl";
    std::string tag = "0";
    int acceptorRank = -1;//默认的rank，让客户端找到地址文件
    int requesterRank = 0;
    int rankOffset = 0;
    int requesterCommunicationSzie = 1;
    //用两个进程分别处理两个套接字通信
    pid_t pid = fork();
    //子进程
    if(pid==0)
    {
        char str[1024];
        comm1->acceptConnection(acceptorName,requesterName,tag,acceptorRank,rankOffset);
        for(int i=0;i<10;i++)
        {
            memset(str,0,1024);
            sprintf(str,"我是第%d个超级男孩,编号00%d",i,i);
            comm1->send(str,requesterRank + rankOffset);
            std::cout<<"发送："<<str<<std::endl;
        }
    }else {//父进程
        sleep(1);
        std::string str;
        comm->requsetConnection(acceptorName,requesterName,tag,requesterRank,requesterCommunicationSzie);
        for(int i=0;i<10;i++)
        {
            str.clear();
            comm->receive(str,acceptorRank);//请求连接端不需要offsetRank
            std::cout<<"接收："<<str.c_str()<<std::endl;
        }
    }
    return 0;
}