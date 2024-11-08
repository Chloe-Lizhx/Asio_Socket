#include <boost/asio.hpp>
#include <cstring>
#include <iostream>
#include <memory>
#include "SocketSendQueue.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
namespace com
{
class Session:public std::enable_shared_from_this<Session>
{
    private:
        std::shared_ptr<tcp::socket> socket_;
        SocketSendQueue sendqueue;
        std::shared_ptr<std::string> msg;
        enum {max_packet_len=1024};
        char read_buff[max_packet_len];
        char write_buff[max_packet_len];
    public:
        Session(tcp::socket sock):socket_(std::make_shared<tcp::socket>(std::move(sock)))//tcp::socket不允许拷贝构造。
        {
            std::cout<<"客户端地址:"<<socket_->remote_endpoint()<<std::endl;
        }  
        ~Session(){std::cout<<"与客户端连接关闭"<<std::endl;}
        void Start()
        {
            do_read();
        }
        void Repeat()//回显给客户端所发送的内容
        {
            std::shared_ptr<Session> self(shared_from_this());
            memset(read_buff,0,max_packet_len);//客户端关闭也会执行异步读写的回调函数
            socket_->async_read_some(buffer(read_buff,max_packet_len),
            [this,self](const boost::system::error_code &err,size_t bytes)
            {
                if(err) {do_close();return;} 
                //cout<<"[" << std::this_thread::get_id()<<"]"<<"读入："<<bytes<<"个字节,"<<"内容："<<read_buff<<endl;
                std::cout<<"接收："<<read_buff<<std::endl;;
                //if(read_buff=="OK"){do_close();return;} 
                do_write(std::move(read_buff));
            }
            );
        }
        void do_read()
        {
            std::shared_ptr<Session> self(shared_from_this());//self是std::shared_ptr<Session>的一个对象，引用计数加1
            //async_read_some函数每次buffer(read_buff,max_packet_len)，都会将受到的n个字节数据对read_buff的前n位进行覆盖，而不是将read_buff清空之后在覆盖
            //因此出现   读入数据：40个字节,内容为：这是第5个超级boy，编号005。
            //         读入数据：2个字节,内容为：OK�是第5个超级boy，编号005。这样两次收到的内容
            memset(read_buff,0,max_packet_len);//客户端关闭也会执行异步读写的回调函数
            socket_->async_read_some(buffer(read_buff,max_packet_len),
            [this,self](const boost::system::error_code &err,size_t bytes)
            {
                if(err) {do_close();return;} 
                cout<<"读入："<<bytes<<"个字节,"<<"内容："<<read_buff<<endl;
                //if(read_buff=="OK"){do_close();return;} 
                do_write();
            }
            );
        }//do_read()函数返回时Session智能指针销毁，引用计数减1
        void do_write(char message[])
        {
            std::shared_ptr<Session> self(shared_from_this());
            memset(write_buff,0,max_packet_len);
            msg.reset();
            msg=std::make_shared<std::string>(std::move(message));
            sendqueue.dispatch(socket_,buffer(msg->c_str(),msg->size()),
            [this,self]()
            {
                //std::cout<<"写入: "<<msg->size()<<"个字节,内容： "<<msg->c_str()<<std::endl;
                std::cout<<"发送："<<msg->c_str()<<std::endl;
                Repeat();
            }
            );
        }
        void do_write()
        {
            std::shared_ptr<Session> self(shared_from_this());
            memset(write_buff,0,max_packet_len);
            const std::string &msg="OK";
            std::copy(msg.begin(), msg.end(), write_buff);
            socket_->async_write_some(buffer(write_buff,msg.size()),
            [this,self](const boost::system::error_code &err,size_t bytes)
            {
                if(err) {do_close();return;} 
                std::cout<<"[" << std::this_thread::get_id() << "]"<<"写入："<<bytes<<"个字节,"<<"内容："<<write_buff<<std::endl;
                do_read();
            }
            );
        }
        void do_close()
        {
            std::cout<<"连接关闭"<<std::endl;
            socket_->close();
        }
};

class Server:public std::enable_shared_from_this<Server>
{
    private:
        tcp::acceptor acceptor_;
    public:
    Server(io_service &Service,short port):acceptor_(Service,tcp::endpoint(tcp::v4(),port)){}
    void do_accept()
        {
            auto self=shared_from_this();
            acceptor_.async_accept([this,self](const boost::system::error_code &err,tcp::socket sock)
            {
                if(!err)
                {
                std::shared_ptr<Session> session=std::make_shared<Session>(std::move(sock));//使用std::move(sock)进行移动，不能简单拷贝
                session->Start();
                self->do_accept();
                }
            });
        }
    void do_repeat()
    {
        auto self=shared_from_this();
            acceptor_.async_accept([this,self](const boost::system::error_code &err,tcp::socket sock)
            {
                if(!err)
                {
                std::shared_ptr<Session> session=std::make_shared<Session>(std::move(sock));//使用std::move(sock)进行移动，不能简单拷贝
                session->Repeat();
                //self->do_repeat();
                }
            });
    }
};
}