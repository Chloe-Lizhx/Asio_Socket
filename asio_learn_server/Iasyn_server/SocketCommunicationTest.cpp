#include <iostream>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <memory>
#include <chrono>
#include <set>
#include <thread>
#include "SocketCommunication.hpp"
#include "Communication.hpp"
using namespace com;
unsigned short port = 0;
bool reuseAddress = false;
std::string networkName = "lo";
std::string addressDirectory = ".";

void acceptCon_requestConTest(unsigned short port = 0,
                              bool reuseAddress = false,
                              std::string networkName = "lo",
                              std::string addressDirectory = ".")
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
    int acceptorRank = 0;
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
}

void acceptConAsServer_requestConAsClient(unsigned short port = 0,
                                          bool reuseAddress = false,
                                          std::string networkName = "lo",
                                          std::string addressDirectory = ".")
{
    using CommPtr = std::shared_ptr<Communication>;
    using SocketCommPtr = std::shared_ptr<SocketCommunication>;
    int testNumber = 4;//请求和接收端各创建testNumber/2个实例
    std::vector<CommPtr> Comm(testNumber);
    for(int i=0;i<testNumber;i++)
    {
        SocketCommPtr socketcomm(new SocketCommunication(port,reuseAddress,networkName,addressDirectory));
        Comm[i]=std::move(socketcomm);
    }
    std::string acceptorName = "lzx";
    std::string requesterName = "xzl";
    std::string tag = "0";
    std::set<int>  acceptorRanks{2,3};
    //Comm的0,1作为request端，2,3作为accept端
    //创建四个进程
    pid_t pid1 = fork();
    pid_t pid2 = fork();
    if(pid1!=0&&pid2!=0)
    {//等待服务端打开
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm[0]->requestConnectionAsClient(acceptorName,requesterName,tag,acceptorRanks,0);
        Comm[0]->send(8,2);
        std::cout<<"rank 0 send '8' to rank 2"<<std::endl;
    }else if(pid1!=0&&pid2==0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm[1]->requestConnectionAsClient(acceptorName,requesterName,tag,acceptorRanks,1);
    }else if(pid1==0&&pid2!=0)
    {
        int item = -1;
        Comm[2]->acceptConnectionAsServer(acceptorName,requesterName,tag,2,2);
        Comm[2]->receive(item,0);
        std::cout<<"rank 2 recv "<<item<<" from rank 0"<<std::endl;
    }else if(pid1==0&&pid2==0)
    {
        Comm[3]->acceptConnectionAsServer(acceptorName,requesterName,tag,3,2);
    }
    
}
int main()
{
    acceptConAsServer_requestConAsClient();
    //acceptCon_requestConTest();
    return 0;
}