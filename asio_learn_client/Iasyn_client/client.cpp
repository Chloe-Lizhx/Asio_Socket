#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/shared_ptr.hpp> 

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

class Session:public std::enable_shared_from_this<Session>
{
    private:
        tcp::socket socket_;
        enum {max_packet_len=1024};
        char read_buff[max_packet_len];
        char write_buff[max_packet_len];
        int i=0;

        void do_test()
        {
            do_write();
            do_read();
        }
        void do_read()
        {
            std::shared_ptr<Session> self(shared_from_this());//self是std::shared_ptr<Session>的一个对象，引用计数加1
            //async_read_some函数每次buffer(read_buff,max_packet_len)，都会将受到的n个字节数据对read_buff的前n位进行覆盖，而不是将read_buff清空之后在覆盖
            //因此出现   读入数据：40个字节,内容为：这是第5个超级女生，编号005。
            //         读入数据：2个字节,内容为：OK�是第5个超级女生，编号005。这样两次收到的内容
            memset(read_buff,0,max_packet_len);//客户端关闭也会执行异步读写的回调函数
            socket_.async_read_some(buffer(read_buff,max_packet_len),
            [this,self](const boost::system::error_code &err,size_t transfered)
            {
                if(err) {do_close();return;} 
                cout<<"读入数据："<<transfered<<"个字节,"<<"内容为："<<read_buff<<endl;
                //if(read_buff=="OK"){do_close();return;} 
                if(i==5){do_close();return;}
                do_write();
            }
            );
        }//do_read()函数返回时Session智能指针销毁，引用计数减1
        void do_write()
        {
            std::shared_ptr<Session> self(shared_from_this());
            memset(write_buff,0,max_packet_len);
            // std::string msg;
            // std::copy(msg.begin(), msg.end(), write_buff);
            sprintf(write_buff,"这是第%d个超级boy，编号%03d。",i+1,i+1);
            socket_.async_write_some(buffer(write_buff,strlen(write_buff)),
            [this,self](const boost::system::error_code &err,size_t transfered)
            {
                if(err) {do_close();return;} 
                cout<<"写入数据："<<transfered<<"个字节,"<<"内容为："<<write_buff<<endl;
                i=i+1;
                do_read();
            }
            );
        }
    public:
        Session(tcp::socket sock):socket_(std::move(sock)){}
        void start()
        {
            do_test();
        }
        void do_close()
        {
            socket_.close();
        }
};

class Client:public std::enable_shared_from_this<Client>
{
    private:
        tcp::socket socket_;
        tcp::endpoint ep;
    public:
        Client(io_service &service,tcp::endpoint &ep):socket_(service),ep(ep){}

        void start_connect()
        {
            auto self=shared_from_this();
            socket_.async_connect(ep,[this,self](const boost::system::error_code &err)
            {
                if(err){do_close();return;}
                std::shared_ptr<Session> session(new Session(std::move(socket_)));
                session->start();
            });
        }
        void do_close()
        {
            std::cout<<"连接失败"<<std::endl;
            socket_.close();
        }
};
int main()
{
    io_service service;
    tcp::endpoint ep(ip::address::from_string("192.168.44.129"),5005);
    std::shared_ptr<Client> client=std::make_shared<Client>(service,ep);
    client->start_connect();
    service.run();
}