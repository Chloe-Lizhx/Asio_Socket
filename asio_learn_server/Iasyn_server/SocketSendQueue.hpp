#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <string>

namespace com{
class SocketSendQueue
{
public:
    using Socket=boost::asio::ip::tcp::socket;
    SocketSendQueue() ;
    ~SocketSendQueue();

    SocketSendQueue(const SocketSendQueue &)=delete;//禁止拷贝构造
    SocketSendQueue &operator=(const SocketSendQueue &)=delete;//禁止拷贝赋值

    void dispatch(std::shared_ptr<Socket> sock,boost::asio::const_buffers_1 data,std::function<void()> callback);
    void sendcomplete();
private:
    void process();
    struct itemtosend
    {
        std::shared_ptr<Socket> sock;
        boost::asio::const_buffers_1 data;
        std::function<void()> callback;
    };

    bool _ready=true;
    std::deque<itemtosend> _sendqueue;
    std::mutex _queuemutex;
};
}