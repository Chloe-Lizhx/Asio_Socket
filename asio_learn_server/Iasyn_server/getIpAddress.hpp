#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include "SocketCommunication.hpp"
#include "utils/assertion.hpp"

namespace com{
    std::string SocketCommunication::getIpAddress()
    {
    #ifdef _WIN32
        return "127.0.0.1";
    #else
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;
    std::string address;
    getifaddrs(&ifAddrStruct);
    auto ifAddrStruct1 = ifAddrStruct;
    while (ifAddrStruct1!=NULL) 
    {
        if (ifAddrStruct1->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
           tmpAddrPtr=&((struct sockaddr_in*)ifAddrStruct1->ifa_addr)->sin_addr;
           char addressBuffer[INET_ADDRSTRLEN];
           inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);//数值to表达式
           if(ifAddrStruct1->ifa_name==_networkName)
           {
            address=std::string(std::move(addressBuffer));
            break;
            }
        } 
        ifAddrStruct1=ifAddrStruct1->ifa_next;
    }
    Assert(ifAddrStruct==NULL,"系统没有检测到任何网络接口信息");
    freeifaddrs(ifAddrStruct);
    return address;
    #endif
    }
}
