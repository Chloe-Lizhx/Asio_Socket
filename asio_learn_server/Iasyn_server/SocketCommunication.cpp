#include <vector>
#include "SocketCommunication.hpp"
#include "SocketRequest.hpp"
#include "utils/assertion.hpp"
#include "utils/print.hpp"
#include "getIpAddress.hpp"
#include "connectionInfo.hpp"

namespace com
{
    SocketCommunication::SocketCommunication(unsigned short portNumber ,
                                            bool reuseAddress ,
                                            std::string networkName ,
                                            std::string addressDirectory )
        :_portNumber(portNumber),
         _reuseAddress(reuseAddress),               
         _networkName(std::move(networkName)),               
         _addressDirectory(std::move(addressDirectory)),               
         _service(new IO_service)                              
    {
        if(_addressDirectory.empty())
        {
            _addressDirectory = ".";
        }
    }

    SocketCommunication::~SocketCommunication()
    {
        closeConnection();
    }
    //关闭所有套接字
    void SocketCommunication::closeConnection()
    {
        if(_thread.joinable())//如果还有异步事件未完成
        {//如果service没有异步工作内容了，_work.reset()解除对service的控制，service按需析构
        _work.reset();
        _service->stop();
        //主线程等待_thread结束,即使_thread已经完成分配的函数
        _thread.join();
        }
        for(auto iter=_sockets.begin();iter!=_sockets.end();iter++)
        {
            iter->second->shutdown(Socket::shutdown_send);//将发送缓冲区的内容发送到对端，同时该套接字不可再写入新消息，但是不影响读对端发送过来的数据的过程
            iter->second->close();//关闭套接字资源。如果只有close,没有shutdown,在close后就不能正常读取消息了，socket两端的数据收发就不完整。
        }
        _connected = false;
    }    

    void SocketCommunication::acceptConnection(std::string const &acceptorName,
                                               std::string const &requesterName,
                                               std::string const &tag,
                                               int acceptorRank,
                                               int rankOffset) //C++最新要求：声明时带有默认参数，实现时没有默认值，或者反过来
    {
        Assert(_connected,"已经连接");
        SetRankOffset(rankOffset);
        std::string address;
        try
        {
            std::string ipAddress = getIpAddress();
            using boost::asio::ip::tcp;
            tcp::endpoint endpoint(boost::asio::ip::address::from_string(ipAddress),_portNumber);
            tcp::acceptor acceptor(*_service,endpoint);
            //如果不提前进入listen,后面每次执行acceptor.accept()都会先进入listen状态，再accept
            acceptor.set_option(tcp::acceptor::reuse_address(_reuseAddress));
            acceptor.listen();
            //获取实际系统分配的端口号
            _portNumber=acceptor.local_endpoint().port();
            address = ipAddress + ":" + std::to_string(_portNumber);
            //创建连接信息的文件
            conInfoWriter conInfo(acceptorName,requesterName,tag,_addressDirectory);
            conInfo.write(address);
            int peercurrent = 0;
            int peercount = -1;
            int requesterCommunicationSize = -1;
            
            do{
                std::shared_ptr<Socket> socket = std::make_shared<Socket>(*_service);
                acceptor.accept(*socket);
                _connected = true;
                int requesterRank = -1;
                socket->read_some(boost::asio::buffer(&requesterRank,sizeof(int)));
                _sockets[requesterRank] = std::move(socket);

                auto adjustRequesterRank = requesterRank + rankOffset;
                send(acceptorRank,adjustRequesterRank);
                receive(requesterCommunicationSize,adjustRequesterRank);
                //初始化
                if(peercurrent==0)
                {peercount = requesterCommunicationSize;}
                Assert((peercount!=requesterCommunicationSize),"从requesterRank接收到的requesterCommunicationSize错误");
                print(requesterRank," accept成功 ");
            }while(++peercurrent<requesterCommunicationSize);
            acceptor.close();
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"acceptConnection接受连接时出错");
        }

        _work = std::make_shared<Work>(*_service);
        _thread = std::thread([this]{_service->run();});
    } 

    void SocketCommunication::acceptConnectionAsServer(std::string const &acceptorName,
                                                       std::string const &requesterName,
                                                       std::string const &tag,
                                                       int                acceptorRank,
                                                       int                requesterCommunicatorSize) 
    {
        Assert(_connected,"已经连接");
        if(requesterCommunicatorSize==0)
        {
            print("","不接受任何连接");
            _connected = true;
            return;
        }
        std::string address;
        try
        {
            using boost::asio::ip::tcp;
            std::string ipAddress = getIpAddress();
            tcp::endpoint endpoint(boost::asio::ip::address::from_string(ipAddress),_portNumber);
            tcp::acceptor acceptor(*_service,endpoint);
            acceptor.set_option(tcp::acceptor::reuse_address(_reuseAddress));
            acceptor.listen();
            //得到实际分配的端口
            _portNumber = acceptor.local_endpoint().port();
            address = ipAddress + ':' + std::to_string(_portNumber);
            conInfoWriter conInfo(acceptorName,requesterName,tag,acceptorRank,_addressDirectory);
            conInfo.write(address);

            for(int peercurrent = 0;peercurrent < requesterCommunicatorSize;peercurrent++)
            {
                std::shared_ptr<Socket> socket = std::make_shared<Socket>(*_service);
                acceptor.accept(*socket);
                _connected = true;
                int requesterRank = -1;
                socket->read_some(boost::asio::buffer(&requesterRank,sizeof(int)));
                _sockets[requesterRank] = std::move(socket);
            }
            acceptor.close();
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"acceptAsServer出错");
        }
        
        _work = std::make_shared<Work>(*_service);
        _thread = std::thread([this]{_service->run();});
    }

    void SocketCommunication::requsetConnection(std::string const &acceptorName,
                                                std::string const &requesterName,
                                                std::string const &tag,
                                                int             requesterRank,
                                                int requesterCommunicationSize)
    {
        Assert(_connected,"已经连接");
        std::string address;
        conInfoReader conInfo(acceptorName,requesterName,tag,_addressDirectory);
        //读取IP+Port
        address = conInfo.read();
        //根据":“分隔符对ip和port进行分解
        auto const sep = address.find(':');
        std::string const ipAddress = address.substr(0,sep);
        std::string const portNum = address.substr(sep + 1);
        _portNumber = static_cast<unsigned short>(std::stoul(portNum));
        try
        {
            using boost::asio::ip::tcp;
            //服务端的ip和port组成的端点
            tcp::endpoint endpoint(boost::asio::ip::address::from_string(ipAddress),_portNumber);
            std::shared_ptr<Socket> socket = std::make_shared<Socket>(*_service);
            boost::system::error_code err = boost::asio::error::host_not_found;
            socket->connect(endpoint,err);
            _connected = !err;
            Assert(!isconnected(),"客户端未连接成功");
            //在客户端只有一个套接字
            socket->write_some(boost::asio::buffer(&requesterRank,sizeof(int)));
            int acceptorRank = -1;
            socket->read_some(boost::asio::buffer(&acceptorRank,sizeof(int)));
            _sockets[acceptorRank] = std::move(socket);
            _sockets[acceptorRank]->write_some(boost::asio::buffer(&requesterCommunicationSize,sizeof(int)));
            print(acceptorRank," request成功 ");
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"requestConnection请求连接时出错");
        }
        
        _work = std::make_shared<IO_service::work>(*_service);
        _thread = std::thread([this]{_service->run();});
    }
                           
    void SocketCommunication::requestConnectionAsClient(std::string const &acceptorName,
                                                        std::string const &requesterName,
                                                        std::string const &tag,
                                                        std::set<int> const &acceptorRanks,
                                                        int          requesterRank)
    {
        Assert(_connected,"已经连接");
        for(auto const &acceptorRank : acceptorRanks)
        {
            std::string address;
            conInfoReader conInfo(acceptorName,requesterName,tag,acceptorRank,_addressDirectory);
            address = conInfo.read();
            auto const sep = address.find(':');
            std::string ipAddress = address.substr(0,sep);
            std::string portNum = address.substr(sep+1);
            _portNumber = std::stoul(portNum);

            try
            {
                using boost::asio::ip::tcp;
                tcp::endpoint endpoint(boost::asio::ip::address::from_string(ipAddress),_portNumber);
                std::shared_ptr<Socket> socket = std::make_shared<Socket>(*_service);
                boost::system::error_code err;
                socket->connect(endpoint,err);
                Assert(err,"connectAsClient的connect错误");
                _connected = !err;
                socket->write_some(boost::asio::buffer(&requesterRank,sizeof(int)));
                _sockets[acceptorRank] = std::move(socket);
            }
            catch(const std::exception& e)
            {
                Assert(e.what(),"requestAsClient出错");
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        _work = std::make_shared<Work>(*_service);
        _thread = std::thread([this]{_service->run();});
    }

    void SocketCommunication::send(std::string const &itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        size_t size = itemstoSend.size() + 1;
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(&size,sizeof(size_t)));
            //C++中很多系统需要字符串最后加一个"\0",c_str()返回的字符串最后一位是“\0”
            _sockets[rankReceiver]->write_some(boost::asio::buffer(itemstoSend.c_str(),size));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送string数据出错");
        } 
    }

    void SocketCommunication::send(const int &itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(&itemstoSend,sizeof(int)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送int数据出错");
        }
    }

    RequestPtr SocketCommunication::aSend(const int &itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        RequestPtr request(new SocketRequest);
        _queue.dispatch(_sockets[rankReceiver],
                        boost::asio::buffer(&itemstoSend,sizeof(int)),
                        [request]{std::static_pointer_cast<SocketRequest>(request)->complete();});
        return request;
    }

    //  RequestPtr SocketCommunication::aSend(const int &itemstoSend,Rank rankReceiver)
    // {
    //     Assert(!_connected,"未连接");
    //     rankReceiver = adjustRank(rankReceiver);
    //     RequestPtr request(new SocketRequest);
    //     _sockets[rankReceiver]->async_write_some(boost::asio::buffer(&itemstoSend,sizeof(int)),
    //                     [request](const boost::system::error_code &err,size_t bytes)
    //                     {std::static_pointer_cast<SocketRequest>(request)->complete();});
    //     return request;
    // }

    void SocketCommunication::send(const double &itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(&itemstoSend,sizeof(double)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送double数据错误");
        }
    }

    RequestPtr SocketCommunication::aSend(const double &itemstoSend,Rank rankReceiver)
    {

        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        RequestPtr request(new SocketRequest);
        _queue.dispatch(_sockets[rankReceiver],
                        boost::asio::buffer(&itemstoSend,sizeof(double)),
                        [request]{std::static_pointer_cast<SocketRequest>(request)->complete();});
        return request;
    }

    // RequestPtr SocketCommunication::aSend(const double &itemstoSend,Rank rankReceiver)
    // {

    //     Assert(!_connected,"未连接");
    //     rankReceiver = adjustRank(rankReceiver);
    //     RequestPtr request(new SocketRequest);
    //     _sockets[rankReceiver]->async_write_some(boost::asio::buffer(&itemstoSend,sizeof(double)),
    //                     [request](const boost::system::error_code &err,size_t bytes)
    //                     {std::static_pointer_cast<SocketRequest>(request)->complete();});
    //     return request;
    // }


    void SocketCommunication::send(bool itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(&itemstoSend,sizeof(bool)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送bool数据错误");
        }
    }

    RequestPtr SocketCommunication::aSend(const bool &itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        RequestPtr request(new SocketRequest);
        _queue.dispatch(_sockets[rankReceiver],
                        boost::asio::buffer(&itemstoSend,sizeof(bool)),
                        [request]{std::static_pointer_cast<SocketRequest>(request)->complete();});
        return request;
    }

    //     RequestPtr SocketCommunication::aSend(const bool &itemstoSend,Rank rankReceiver)
    // {
    //     Assert(!_connected,"未连接");
    //     rankReceiver = adjustRank(rankReceiver);
    //     RequestPtr request(new SocketRequest);
    //     _sockets[rankReceiver]->async_write_some(boost::asio::buffer(&itemstoSend,sizeof(bool)),
    //                     [request](const boost::system::error_code &err,size_t bytes)
    //                     {std::static_pointer_cast<SocketRequest>(request)->complete();});
    //     return request;
    // }

    void SocketCommunication::send(std::span<const int> itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(itemstoSend.data(),itemstoSend.size()*sizeof(int)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送std::span<const int>数据错误");
        }
    }

    RequestPtr SocketCommunication::aSend(std::span<const int> itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        RequestPtr request(new SocketRequest);
        _queue.dispatch(_sockets[rankReceiver],
                        boost::asio::buffer(itemstoSend.data(),itemstoSend.size()*sizeof(int)),
                        [request]{std::static_pointer_cast<SocketRequest>(request)->complete();});
        return request;
    }

    //     RequestPtr SocketCommunication::aSend(std::span<const int> itemstoSend,Rank rankReceiver)
    // {
    //     Assert(!_connected,"未连接");
    //     rankReceiver = adjustRank(rankReceiver);
    //     RequestPtr request(new SocketRequest);
    //     _sockets[rankReceiver]->async_write_some(boost::asio::buffer(itemstoSend.data(),itemstoSend.size()*sizeof(int)),
    //                     [request](const boost::system::error_code &err,size_t bytes)
    //                     {std::static_pointer_cast<SocketRequest>(request)->complete();});
    //     return request;
    // }

    void SocketCommunication::send(std::span<const double> itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(itemstoSend.data(),itemstoSend.size()*sizeof(double)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送std::span<const double>数据错误");
        }
    }

     RequestPtr SocketCommunication::aSend(std::span<const double> itemstoSend,Rank rankReceiver)
    {
        Assert(!_connected,"未连接");
        rankReceiver = adjustRank(rankReceiver);
        RequestPtr request(new SocketRequest);
        _queue.dispatch(_sockets[rankReceiver],
                        boost::asio::buffer(itemstoSend.data(),itemstoSend.size()*sizeof(double)),
                        [request]{std::static_pointer_cast<SocketRequest>(request)->complete();});
        return request;
    }
    

    //      RequestPtr SocketCommunication::aSend(std::span<const double> itemstoSend,Rank rankReceiver)
    // {
    //     Assert(!_connected,"未连接");
    //     rankReceiver = adjustRank(rankReceiver);
    //     RequestPtr request(new SocketRequest);
    //     _sockets[rankReceiver]->async_write_some(boost::asio::buffer(itemstoSend.data(),itemstoSend.size()*sizeof(double)),
    //                     [request](const boost::system::error_code &err,size_t bytes)
    //                     {std::static_pointer_cast<SocketRequest>(request)->complete();});
    //     return request;
    // }
    
    void SocketCommunication::receive(std::string &itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        size_t size = -1;
        try
        {//itemstoReceive为空时，buffer不能使用
            _sockets[rankSender]->read_some(boost::asio::buffer(&size,sizeof(size_t)));
            //std::vector是在堆上创建内存，而char []是在栈上创建内存，直接使用msg.data()可以省去边界检测，提高效率
            std::vector<char> msg(size);
            _sockets[rankSender]->read_some(boost::asio::buffer(msg.data(),size));
            itemstoReceive = msg.data();
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收string数据出错");
        }
    }

    void SocketCommunication::receive(int &itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        try
        {
            _sockets[rankSender]->read_some(boost::asio::buffer(&itemstoReceive,sizeof(int)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收int数据出错");
        }
    }

    RequestPtr SocketCommunication::aReceive(int &itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        RequestPtr request(new SocketRequest);
        try
        {
            _sockets[rankSender]->async_read_some(boost::asio::buffer(&itemstoReceive,sizeof(int)),
                                                  [request](const boost::system::error_code &err,size_t bytes)
                                                  {std::static_pointer_cast<SocketRequest>(request)->complete();});
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"异步接收int数据出错");
        }   
        return request;
    }

    void SocketCommunication::receive(double &itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        try
        {
            _sockets[rankSender]->read_some(boost::asio::buffer(&itemstoReceive,sizeof(double)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收double数据错误");
        }
    }

    RequestPtr SocketCommunication::aReceive(double &itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        RequestPtr request(new SocketRequest);
        try
        {
            _sockets[rankSender]->async_read_some(boost::asio::buffer(&itemstoReceive,sizeof(double)),
                                                  [request](const boost::system::error_code &err,size_t bytes)
                                                  {std::static_pointer_cast<SocketRequest>(request)->complete();});
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"异步接收double数据出错");
        }
        return request;
    }

    void SocketCommunication::receive(bool &itemstoReceive,Rank rankSender) 
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        try
        {
            _sockets[rankSender]->read_some(boost::asio::buffer(&itemstoReceive,sizeof(bool)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收bool数据错误");
        }
    }

    RequestPtr SocketCommunication::aReceive(bool &itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        RequestPtr request(new SocketRequest);
        try
        {
            _sockets[rankSender]->async_read_some(boost::asio::buffer(&itemstoReceive,sizeof(bool)),
                                                  [request](const boost::system::error_code &err,size_t bytes)
                                                  {std::static_pointer_cast<SocketRequest>(request)->complete();});
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"异步接收bool数据出错");
        }
        return request;
    }

    void SocketCommunication::receive(std::span<int> itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        try
        {
            _sockets[rankSender]->read_some(boost::asio::buffer(itemstoReceive.data(),itemstoReceive.size()*sizeof(int)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收std::span<int>数据错误");
        }
    }

    RequestPtr SocketCommunication::aReceive(std::span<int> itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        RequestPtr request(new SocketRequest);
        try
        {
            _sockets[rankSender]->async_read_some(boost::asio::buffer(itemstoReceive.data(),itemstoReceive.size()*sizeof(int)),
                                                  [request](const boost::system::error_code &err,size_t bytes)
                                                  {std::static_pointer_cast<SocketRequest>(request)->complete();});
        }
        catch(const std::exception& e)
        {
           Assert(e.what(),"异步接收std::span<int>数据错误");
        }
        return request;
    }

    void SocketCommunication::receive(std::span<double> itemstoReceive,Rank rankSender) 
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        try
        {
            _sockets[rankSender]->read_some(boost::asio::buffer(itemstoReceive.data(),itemstoReceive.size()*sizeof(double)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收std::span<double>数据出错");
        }
    }

    RequestPtr SocketCommunication::aReceive(std::span<double> itemstoReceive,Rank rankSender)
    {
        Assert(!_connected,"未连接");
        rankSender = adjustRank(rankSender);
        RequestPtr request(new SocketRequest);
        try
        {
            _sockets[rankSender]->async_read_some(boost::asio::buffer(itemstoReceive.data(),itemstoReceive.size()*sizeof(double)),
                                                  [request](const boost::system::error_code &err,size_t bytes)
                                                  {std::static_pointer_cast<SocketRequest>(request)->complete();});
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"异步发送std::span<double>数据错误");
        }
        return request;
    }

    size_t SocketCommunication::getRemoteCommunicatorSize() {return _sockets.size();}

    void SocketCommunication::prepareEstablishment(std::string const &acceptorName,std::string const &requesterName) 
    {
        fs::path path = impl::getLocalDirectory(acceptorName,requesterName,_addressDirectory);
        try
        {
            fs::create_directory(path);
        }
        catch(const fs::filesystem_error& e)
        {
            Assert(e.what(),"prepareEstablishment出错");
        } 
    }
    void SocketCommunication::cleanupEstablishment(std::string const &acceptorName,std::string const &requesterName) 
    {
        fs::path path = impl::getLocalDirectory(acceptorName,requesterName,_addressDirectory);
        try
        {
            fs::remove_all(path);
        }
        catch(const fs::filesystem_error& e)
        {
            Assert(e.what(),"cleanupEstablishment出错");
        }
    }


}