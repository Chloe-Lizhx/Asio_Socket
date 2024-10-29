#include <assert.h>
#include "SocketSendQueue.hpp"

SocketSendQueue::~SocketSendQueue()
{
    assert(!_sendqueue.empty());
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
                            [item,this](boost::system::error_code &err,size_t bytes)
                            {
                                if(err){return;}
                                item.callback();
                                this->sendcompleted();
                            });
}