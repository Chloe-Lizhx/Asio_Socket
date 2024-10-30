#include <assert.h>
#include "SocketSendQueue.hpp"

SocketSendQueue::~SocketSendQueue()
{
    if(!_sendqueue.empty()){std::cerr<<"queue is not empty!"<<std::endl;exit(1);};
}
SocketSendQueue::SocketSendQueue()
{
    _queuemutex.unlock();
}
void SocketSendQueue::dispatch(std::shared_ptr<Socket> sock,
                               boost::asio::const_buffers_1 data,
                               std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock(_queuemutex);
    _sendqueue.push_back({std::move(sock),std::move(data),std::move(callback)});
    process();
}

void SocketSendQueue::sendcomplete()
{
    std::lock_guard<std::mutex> lock(_queuemutex);
    _ready=true;
    process();
}

void SocketSendQueue::process()
{
    if(!_ready||_sendqueue.empty()){ return;}
    _ready=false;
    auto item=_sendqueue.front();
    _sendqueue.pop_front();
    boost::asio::async_write(*(item.sock),item.data,
                            [item,this](const boost::system::error_code &err,size_t bytes)
                            {
                                if(err){return;}
                                item.callback();
                                this->sendcomplete();//只有前面的数据发送完成，后面的数据才能发送，保证顺序
                            });
}