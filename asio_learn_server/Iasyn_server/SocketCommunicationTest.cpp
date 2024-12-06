#include <iostream>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <memory>
#include <chrono>
#include <set>
#include <thread>
#include <random>
#include "CommunicationFactory.hpp"
#include "SocketCommunicationFactory.hpp"
#include "SocketCommunication.hpp"
#include "Communication.hpp"
#include "SharedPointer.hpp"
#include "Request.hpp"
#include "SocketRequest.hpp"
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
        comm->requestConnection(acceptorName,requesterName,tag,requesterRank,requesterCommunicationSzie);
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

void reducesum_broadcast(unsigned short port = 0,
                bool reuseAddress = false,
                std::string networkName = "lo",
                std::string addressDirectory = ".")
{
    std::shared_ptr<CommunicationFactory> communicator =std::make_shared<SocketCommunicationFactory>();
    std::vector<CommunicationPtr> communicators;
    for(int i=0;i<4;i++)
    {
        communicators.push_back(communicator->newCommunication());
    }
    pid_t pid1 = fork();
    pid_t pid2 = fork();
    if(pid1!=0&&pid2!=0)
    {
        int rank=0;
        communicators[0]->IntraConnect("lzx","88",rank,4);
        std::vector<int> v={1,4,7,10,13};
        std::vector<int> v1;
        communicators[0]->AllreduceSumForPrimaryRank(v,v1);
        communicators[0]->broadcastForPrimaryRank(v);
        std::cout<<"rank0's sum: ";
        for(auto i :v1)
        {
            std::cout<<i<<",";
        }
        std::cout<<""<<std::endl;
    }
    else if(pid1!=0&&pid2==0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int rank=1;
        communicators[1]->IntraConnect("lzx","88",rank,4);
        std::vector<int> v={2,5,8,11,14};
        std::vector<int> v1;
        communicators[1]->AllreduceSumForSecondaryRank(v,v1,0);
        communicators[1]->broadcastForSecondaryRank(v,0);
        std::cout<<"rank1's sum: ";
        for(auto i :v1)
        {
            std::cout<<i<<",";
        }
        std::cout<<""<<std::endl;
    }
    else if(pid1==0&&pid2!=0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int rank=2;
        communicators[2]->IntraConnect("lzx","88",rank,4);
        std::vector<int> v={3,6,9,12,15};
        std::vector<int> v1;
        communicators[2]->AllreduceSumForSecondaryRank(v,v1,0);
        communicators[2]->broadcastForSecondaryRank(v,0);
        std::cout<<"rank2's sum: ";
        for(auto i :v1)
        {
            std::cout<<i<<",";
        }
        std::cout<<""<<std::endl;
    }
    else if(pid1==0&&pid2==0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int rank=3;
        communicators[3]->IntraConnect("lzx","88",rank,4);
        std::vector<int> v={4,7,10,13,16};
        std::vector<int> v1;
        communicators[3]->AllreduceSumForSecondaryRank(v,v1,0);
        communicators[3]->broadcastForSecondaryRank(v,0);
        std::cout<<"rank3's sum: ";
        for(auto i :v1)
        {
            std::cout<<i<<",";
        }
        std::cout<<""<<std::endl;
    }
}
//同步和异步的测试结果：
//1、异步程序大概是同步的两倍
//2、当数据够大和事件执行时间够长时，由于发送队列SendQueue造成的延时可忽略
//3、异步程序有很好的调度算法，让并发线程并行运行
void Sync(unsigned short port = 0,
                bool reuseAddress = false,
                std::string networkName = "lo",
                std::string addressDirectory = ".")
{
    std::shared_ptr<CommunicationFactory> communicator(new SocketCommunicationFactory());
    int vectorNum = 1000;
    pid_t pid1 = fork();
    if(pid1!=0)//0进程，同步收发
    {
        std::vector<std::vector<int>> matrix(vectorNum, std::vector<int>(vectorNum));
        // 创建一个随机数生成器
        std::default_random_engine rng;
        // 创建一个均匀分布，范围从1到100
        std::uniform_int_distribution<int> dist(1, 1000);
        // 遍历矩阵并随机填充每个元素
        for (int i = 0; i < vectorNum; ++i) 
        {
            for (int j = 0; j < vectorNum; ++j) 
            {
                matrix[i][j] = dist(rng);
            }
        }
        Rank rank=0;
        Rank remoteRank = 1;
        //连接至进程1
        CommunicationPtr Comm(communicator->newCommunication());
        Comm->acceptConnectionAsServer("lzx","xzl","SyncTest",rank,1);
        auto start = std::chrono::high_resolution_clock::now();
        //遍历发送矩阵
        for(int i=0;i<vectorNum;i++)
        {
            Comm->send(matrix[i],remoteRank);
        }
                //延时200ms，作为计算事件的执行时间
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        auto stop = std::chrono::high_resolution_clock::now();  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count(); 
        std::cout<<"同步进程0发送时间:"<<duration<<std::endl;
    }
    else if(pid1==0)//1进程，同步收发
    {
        std::vector<std::vector<int>> matrix(vectorNum, std::vector<int>(vectorNum,0));
        Rank rank=1;
        Rank remoteRank = 0;
        CommunicationPtr Comm(communicator->newCommunication());
        //连接
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        Comm->requestConnectionAsClient("lzx","xzl","SyncTest",{remoteRank},rank);
        auto start = std::chrono::high_resolution_clock::now();
        //遍历发送矩阵
        for(int i=0;i<vectorNum;i++)
        {
            Comm->receive(matrix[i],remoteRank);
        }
                //延时200ms，作为计算事件的执行时间
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        auto stop = std::chrono::high_resolution_clock::now();  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count(); 
        std::cout<<"同步进程1接收时间:"<<duration<<std::endl;
        exit(0);
    }
}

void asyn(unsigned short port = 0,
                bool reuseAddress = false,
                std::string networkName = "lo",
                std::string addressDirectory = ".")
{
     std::shared_ptr<CommunicationFactory> communicator(new SocketCommunicationFactory(port,reuseAddress,networkName,addressDirectory));
    int vectorNum = 1000;
    pid_t pid1 = fork();
    if(pid1!=0)//0进程，同步收发
    {
        std::vector<std::vector<int>> matrix(vectorNum, std::vector<int>(vectorNum));
        // 创建一个随机数生成器
        std::default_random_engine rng;
        // 创建一个均匀分布，范围从1到100
        std::uniform_int_distribution<int> dist(1, 1000);
        // 遍历矩阵并随机填充每个元素
        for (int i = 0; i < vectorNum; ++i) 
        {
            for (int j = 0; j < vectorNum; ++j) 
            {
                matrix[i][j] = dist(rng);
            }
        }
        Rank rank=0;
        Rank remoteRank = 1;
        //连接至进程1
        std::vector<RequestPtr> requests;
        CommunicationPtr Comm(communicator->newCommunication());
        Comm->acceptConnectionAsServer("lzx","xzl","AsynTest",rank,1);
        auto start = std::chrono::high_resolution_clock::now();
        //遍历发送矩阵
        for(int i=0;i<vectorNum;i++)
        {
            requests.push_back(Comm->aSend(matrix[i],remoteRank));
        }
        //延时200ms，作为计算事件的执行时间
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        Request::wait(requests);
        auto stop = std::chrono::high_resolution_clock::now();  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count(); 
        std::cout<<"异步进程0发送时间:"<<duration<<std::endl;
    }
    else if(pid1==0)//1进程，同步收发
    {
        std::vector<std::vector<int>> matrix(vectorNum, std::vector<int>(vectorNum,0));
        Rank rank=1;
        Rank remoteRank = 0;
        CommunicationPtr Comm(communicator->newCommunication());
        //连接
        std::vector<RequestPtr> requests;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Comm->requestConnectionAsClient("lzx","xzl","AsynTest",{remoteRank},rank);
        auto start = std::chrono::high_resolution_clock::now();
        //遍历发送矩阵
        for(int i=0;i<vectorNum;i++)
        {
            requests.push_back(Comm->aReceive(matrix[i],remoteRank));
        }
        //延时200ms，作为计算事件的执行时间
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        Request::wait(requests);
        auto stop = std::chrono::high_resolution_clock::now();  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count(); 
        std::cout<<"异步进程1接收时间:"<<duration<<std::endl;
    }
}

void Ajacobi(unsigned short port = 0,
                bool reuseAddress = false,
                std::string networkName = "lo",
                std::string addressDirectory = ".")
{
    std::shared_ptr<CommunicationFactory> communicator(new SocketCommunicationFactory());

    std::vector<CommunicationPtr> communicators(4);
    //创建共享数据
    int totalsize = 16;
    int mysize = totalsize / 4;
    int steps = 10;
    int world_size = 4;
    std::string tag = "88";
    //创建4个进程
    pid_t pid1 = fork();
    pid_t pid2 = fork();
    //定义进程号
    int rank=-1;
    //为每个进程分配唯一的进程号
    if(pid1!=0&&pid2!=0)
    {rank = 0;}
    else if(pid1!=0&&pid2==0)
    {rank = 1;}
    else if(pid1==0&&pid2!=0)
    {rank = 2;}
    else if(pid1==0&&pid2==0)
    {rank = 3;}
    //相邻进程连接
    communicators[rank] = communicator->newCommunication();
    com::connectSeries("lzx",addressDirectory,tag,rank,world_size,communicators[rank]);
    //声明私有数据
    int begin_col =-1,end_col=-1;
    int left = -1,right = -1;
    //定义数据块
    std::vector<std::vector<double>> a(totalsize, std::vector<double>(mysize + 2, 0.0));
    std::vector<std::vector<double>> b(totalsize, std::vector<double>(mysize + 2, 0.0));
    //中间变量，这里的4是因为有4个收发数据操作
    std::vector<std::vector<double>> temp(4,std::vector<double>(totalsize,0.0));
    std::vector<RequestPtr> requests;
    if(rank==0||rank==world_size-1)
    {requests.reserve(2);}
    else{requests.reserve(4);}
    //原始数据赋值
    for(int i=0;i<mysize+2;i++)
    {
        for(int j=0;j<totalsize;j++)
        {
            a[j][i] = 0.0;
        }
    }
    for(int i=0;i<totalsize;i++)
    {
        a[i][0] = 8.0;
        a[i][mysize+1] = 8.0;
    }
    if (rank == 0) {
        for (int i = 0; i < totalsize; i++) 
        {
            a[i][1] = 8.0;
        }
    } else if (rank == 3) {
        for (int i = 0; i < totalsize; i++) 
        {
            a[i][mysize] = 8.0;
        }
    }
    for (int i = 0; i < mysize+2; i++) {
        a[0][i] = 8.0;
        a[totalsize-1][i] = 8.0;
    }
    //根据rank判断left,right,begin_col,end_col
    if(rank>0)
    {left = rank-1;}

    if(rank<world_size-1)
    {right = rank+1;}

    begin_col = 1;
    end_col = mysize;
    if (rank == 0)
    {
        begin_col = 2;
    }
    if (rank == world_size - 1) 
    {
        end_col = mysize-1;
    }
    //时间
    int64_t duration = 0;
    for (int n = 0; n <steps; n++) 
    {
        auto start = std::chrono::high_resolution_clock::now();
        //先计算需要通信的边界数据
        for(int i=1;i<totalsize-1;i++)
        {
            b[i][begin_col] = (a[i][begin_col+1]+a[i][begin_col-1]+a[i+1][begin_col]+a[i-1][begin_col])*0.25;
            b[i][end_col] = (a[i][end_col+1]+a[i][end_col-1]+a[i+1][end_col]+a[i-1][end_col])*0.25;
        }
        //执行非阻塞通信 将下一次计算需要的数据首先进行通信
        for(int i=0;i<totalsize;i++)
        {
            temp[0][i] = b[i][end_col];
        }
        if(rank!=world_size-1)
        {requests.push_back(communicators[rank]->aSend(temp[0],right));}


        for(int i=0;i<totalsize;i++)
        {
            temp[1][i] = b[i][begin_col];
        }
        if(rank!=0)
        {requests.push_back(communicators[rank]->aSend(temp[1],left));}

        if(rank!=0)
        {requests.push_back(communicators[rank]->aReceive(temp[2],left));}
        
        if(rank!=world_size-1)
        {requests.push_back(communicators[rank]->aReceive(temp[3],right));}

        
        //计算剩余的部分
        for(int j = begin_col+1;j<=end_col-1;j++)
        {
            for(int i=1;i<=totalsize-2;i++)
            {
                b[i][j] = (a[i][j+1]+a[i][j-1]+a[i+1][j]+a[i-1][j])*0.25;
            }
        }
        //更新数组
        for (int j = begin_col; j <= end_col; j++) 
        {
            for (int i = 1; i < totalsize - 1; i++) 
            {
                a[i][j] = b[i][j];
            }
        }
        //等待数据收发完成
        Request::wait(requests);
        auto stop = std::chrono::high_resolution_clock::now();  
        duration = duration + std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        //数据收发完成后，更新边界值；
        for(int i=0;i<totalsize;i++)
        {
            a[i][0] = temp[2][i];
            a[i][mysize+1] = temp[3][i];
        }

        for(int i=0;i<requests.size();i++)
        {
            requests[i].reset();
        }
        requests.clear();
        
    }

    // for (int i = 1; i < totalsize - 1; i++) 
    // {
    //     for (int j = begin_col; j <= end_col; j++) 
    //     {
    //         std::cout << rank << ", " << i << ", " << j << ", " << a[i][j] << std::endl;
    //     }
    // }
    std::cout<<"rank:"<<rank<<",duration:"<<duration<<std::endl;

}
int main()
{
    //acceptConAsServer_requestConAsClient();
    //acceptCon_requestConTest();
    //IntraConnect();
    //aSend_aRecvTest();
    //reducesum_broadcast();
    // Sync();
    // asyn();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Ajacobi();
    return 0;
}