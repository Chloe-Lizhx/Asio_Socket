#include <iostream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <mutex>
#include <memory>

using namespace boost;
using namespace boost::asio;
using boost::system::error_code;
typedef std::shared_ptr<ip::tcp::socket> socket_ptr;
/// 正常的收发
void send_recv()
{
    io_service service;//调用系统输入/输出服务
    ip::tcp::endpoint ep(ip::address_v4(),5005);//创建端点，ip::address_v4()表示可以使用本机任意ipv4地址的网卡
    ip::tcp::acceptor acc(service,ep);//服务器端的接收器
    acc.listen();
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
}
/// 测试asio::write和asio::tcp::write_some的通信效率
void test_write()
{
    io_service service;//调用系统输入/输出服务
    ip::tcp::endpoint ep(ip::address_v4(),5005);//创建端点，ip::address_v4()表示可以使用本机任意ipv4地址的网卡
    ip::tcp::acceptor acc(service,ep);//服务器端的接收器
    acc.listen();
    error_code err;
    while(true)
    {
        socket_ptr clientsock(new ip::tcp::socket(service));
         //第一步：监听客户端,如果有客户端连上来，返回绑定ip和port的套接字
        acc.accept(*clientsock,err);
        char buff[1024];
        //第二步：与客户端通信
        int64_t count=0;
        for(int j=0;j<10;j++)
        {auto start = std::chrono::high_resolution_clock::now(); 
        for(int i=0;i<1000;i++)
        {
            memset(buff,0,sizeof(buff));
            sprintf(buff,"这是第%d个超级女生，编号%03d。",i+1,i+1); 
            clientsock->write_some(buffer(buff,strlen(buff)),err);
            //asio::write(*clientsock,buffer(buff,strlen(buff)));
            if(err){std::cout<<"退出"<<std::endl;break;}
        }
        auto stop = std::chrono::high_resolution_clock::now();  
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();  
        count = count + duration;
        
        }std::cout<<"写入时间: "<<count/10<<" ns"<<std::endl;
        //第三步：关闭套接字，释放内存、IP、Port
        clientsock->close();
    }
}
/// 测试asio::read和asio::tcp::read_some的通信效率
void test_read()
{

}

int main()
{
    test_write();
    return 0;
}