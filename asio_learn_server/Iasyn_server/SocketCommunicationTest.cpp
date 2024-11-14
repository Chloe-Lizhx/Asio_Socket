#include <iostream>
#include <unistd.h>
#include <string>
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
    //用两个进程分别处理两个套接字通信
    pid_t pid = fork();
    //子进程
    if(pid==0)
    {
        comm1->acceptConnection("lzx","xzl",0,-1,0);
    }else {//父进程
        sleep(1);
        comm->requsetConnection("lzx","xzl",0,0,1);
    }
    return 0;
}