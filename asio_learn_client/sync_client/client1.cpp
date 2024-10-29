#include <iostream>
#include <string>
#include <cstdlib>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <mutex>
#include <memory>

using namespace boost;
using namespace boost::asio;
using boost::system::error_code;

bool Connect(ip::tcp::endpoint &ep,ip::tcp::socket &sock)
{
    error_code err;
    sock.connect(ep,err);
    if(err){return false;}
    return true;
}
int main()
{
    io_service service;
    // 第1步：创建客户端的socket。
    ip::tcp::endpoint ep(ip::address::from_string("192.168.44.129"),5005);
    ip::tcp::socket sock(service);
    // 第2步：向服务器发起连接请求。 
    if(!Connect(ep,sock)){std::cerr<<"连接失败"<<std::endl;exit(1);}
    // 第3步：与服务端通讯，客户发送一个请求报文后等待服务端的回复，收到回复后，再发下一个请求报文。
    char buff[1024];
    for(int i=0;i<5;i++)
    {
        //发送
        memset(buff,0,sizeof(buff));
        sprintf(buff,"这是第%d个超级boy, 编号00%d",i,i);
        if(!asio::write(sock,buffer(buff,strlen(buff)))){std::cerr<<"发送失败"<<std::endl;exit(2);}
        std::cout << "发送：" << buff << std::endl;
        //接收
        memset(buff,0,sizeof(buff));
        //asio::read函数要求传入确定的接收数据的大小,因此使用sock.read_some更加友好
        //if(!asio::read(sock,buffer(buff,33))){std::cerr<<"接收失败"<<std::endl;exit(3);}
        if(!sock.read_some(buffer(buff,sizeof(buff)))){std::cerr<<"接收失败"<<std::endl;exit(3);};
        std::cout<<"接收："<<buff<<std::endl;
    }
    //第四步，关闭套接字
    sock.close();
    return 0;
}
