#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>

// struct Interface{
//     int index;
//     std::string Name;
//     std::string Address;
// };
// std::string getIpAddress()
// {
//     std::vector<Interface> interfaces;
//     struct if_nameindex *nameInterface = if_nameindex();
//     for(auto itnameInterface = nameInterface;itnameInterface->if_index!=0;itnameInterface++)
//     {
//         Interface interface;
//         interface.index=itnameInterface->if_index;
//         interface.Name=itnameInterface->if_name;
//         interfaces.push_back(std::move(interface));
//     }
//     if_freenameindex(nameInterface);
// }
std::string _netWorkName = "lo";
std::string getIpAddress() 
{
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
           if(ifAddrStruct1->ifa_name==_netWorkName)
           {
            address=std::string(std::move(addressBuffer));
            break;
            }
        } 
        ifAddrStruct1=ifAddrStruct1->ifa_next;
    }
    if(ifAddrStruct!=NULL)
    {
    freeifaddrs(ifAddrStruct);}
    return address;
}

int main (int argc, const char * argv[]) 
{
    std::string addr(std::move(getIpAddress()));
    std::cout<<addr<<std::endl;
    return 0;
}

