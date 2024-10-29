#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <string>

class SocketSendQueue
{
public:
    using Socket=boost::asio::ip::tcp::socket;
    SocketSendQueue() = default;
    ~SocketSendQueue();

    SocketSendQueue(const SocketSendQueue &)=delete;//禁止拷贝构造
    SocketSendQueue &operator=(const SocketSendQueue &)=delete;//禁止拷贝赋值

    void dispatch(std::shared_ptr<Socket> sock,boost::asio::const_buffers_1 data,std::function<void()> callback);
    void sendcompleted();
private:
    void process();
    struct itemtosend
    {
        std::shared_ptr<Socket> sock;
        boost::asio::const_buffers_1 data;
        std::function<void()> callback;
    };

    bool _ready;
    std::deque<itemtosend> _sendqueue;
    std::mutex _queuemutex;
};