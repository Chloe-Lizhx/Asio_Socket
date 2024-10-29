#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <boost/asio.hpp>
#include <mutex>
#include <memory>

using namespace boost;
using namespace boost::asio;
using boost::system::error_code;
typedef std::shared_ptr<ip::tcp::socket> socket_ptr;


int main()
{
    io_service service;//调用系统输入/输出服务
    ip::tcp::endpoint ep(ip::address_v4(),5005);//创建端点，ip::address_v4()表示可以使用本机任意ipv4地址的网卡
    ip::tcp::acceptor acc(service,ep);//服务器端的接收器
    error_code err;
    while(true)
    {
        socket_ptr clientsock(new ip::tcp::socket(service));
        acc.set_option(ip::tcp::acceptor::reuse_address(true));
         //第一步：监听客户端,如果有客户端连上来，返回绑定ip和port的套接字
        acc.accept(*clientsock,err);
        char buff[1024];
        //第二步：与客户端通信
        while(true)
        {
            memset(buff,0,sizeof(buff));
            //调用socket.read_some()函数，阻塞接收消息
            clientsock->read_some(buffer(buff,sizeof(buff)),err);
            if(err){std::cout<<"退出"<<std::endl;break;}
            std::cout<<"接收："<<buff<<std::endl;

            memset(buff,0,sizeof(buff));
            strcpy(buff,"OK");
            //调用socket.write_some()函数，阻塞发送消息
            clientsock->write_some(buffer(buff,strlen(buff)),err);
            if(err){std::cout<<"退出"<<std::endl;break;}
            std::cout<<"发送："<<buff<<std::endl;
        }
        //第三步：关闭套接字，释放内存、IP、Port
        clientsock->close();
    }
    return 0;
}