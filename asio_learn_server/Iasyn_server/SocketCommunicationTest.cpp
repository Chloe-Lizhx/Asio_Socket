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

void IntraConnect(unsigned short port = 0,
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
    std::string participantName = "lzx";
    std::string tag = "0";
    //Comm的0,1作为request端，2,3作为accept端
    //创建四个进程
    pid_t pid1 = fork();
    pid_t pid2 = fork();
    if(pid1!=0&&pid2!=0)
    {//等待服务端打开
        int i1=-1,i2=-1,i3=-1;
        Comm[0]->IntraConnect(participantName,tag,0,4);
        Comm[0]->receive(i1,1);
        Comm[0]->receive(i2,2);
        Comm[0]->receive(i3,3);
        std::cout<<"rank1: "<<i1<<" rank2 :"<<i2<<" rank3:"<<i3<<std::endl;
    }else if(pid1!=0&&pid2==0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm[1]->IntraConnect(participantName,tag,1,4);
        Comm[1]->send(8,0);
    }else if(pid1==0&&pid2!=0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm[2]->IntraConnect(participantName,tag,2,4);
        Comm[2]->send(88,0);
    }else if(pid1==0&&pid2==0)
    {
       std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm[3]->IntraConnect(participantName,tag,3,4);
        Comm[3]->send(888,0);
    }
}

void aSend_aRecvTest(unsigned short port = 0,
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
        std::string str="hello,I'm requester 0";
        int i=8;
        double ii=9.0;
        bool iii=1;
        std::vector<RequestPtr> requests;
        requests.push_back(Comm[0]->aSend(i,2));
        requests.push_back(Comm[0]->aSend(ii,2));
        requests.push_back(Comm[0]->aSend(iii,2));
        Request::wait(requests);
    }else if(pid1!=0&&pid2==0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm[1]->requestConnectionAsClient(acceptorName,requesterName,tag,acceptorRanks,1);
        std::vector<int> arr1={0,1,2,3,4,5,6,7,8,9};
        std::vector<double> arr2={9,8,7,6,5,4,3,2,1,0};
        std::vector<RequestPtr> requests;
        requests.push_back(Comm[1]->aSend(arr1,3));
        requests.push_back(Comm[1]->aSend(arr2,3));
        Request::wait(requests);
    }else if(pid1==0&&pid2!=0)
    {
        std::string str;
        Comm[2]->acceptConnectionAsServer(acceptorName,requesterName,tag,2,2);
        int i;
        double ii;
        bool iii;
        std::vector<RequestPtr> requests;
        requests.push_back(Comm[2]->aReceive(i,0));
        requests.push_back(Comm[2]->aReceive(ii,0));
        requests.push_back(Comm[2]->aReceive(iii,0));
        Request::wait(requests);
        std::cout<<"str i ii iii:"<<str<<" "<<i<<" "<<ii<<" "<<iii<<std::endl;
    }else if(pid1==0&&pid2==0)
    {
        Comm[3]->acceptConnectionAsServer(acceptorName,requesterName,tag,3,2);
        int arr1[10];
        double arr2[10];
        std::vector<RequestPtr> requests;
        requests.push_back(Comm[3]->aReceive(arr1,1));
        requests.push_back(Comm[3]->aReceive(arr2,1));
        Request::wait(requests);
        std::cout<<"arr1 arr2 "<<arr1[0]<<"..."<<arr2[0]<<std::endl;
    }
}
int main()
{
    //acceptConAsServer_requestConAsClient();
    //acceptCon_requestConTest();
    //IntraConnect();
    aSend_aRecvTest();
    return 0;
}