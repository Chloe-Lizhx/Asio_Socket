#include "SocketCommunication.hpp"
#include "utils/assertion.hpp"
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
        for(auto iter=_sockets.begin();iter!=_sockets.end();iter++)
        {
            iter->second->close();
        }
    }    

    void SocketCommunication::acceptConnection(std::string const &acceptorName,
                                               std::string const &requesterName,
                                               std::string const &tag,
                                               Rank acceptorRank,
                                               int rankOffset = 0
                                               ) 
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
            acceptor.listen();
            acceptor.set_option(tcp::acceptor::reuse_address(_reuseAddress));
            //获取实际系统分配的端口号
            _portNumber=acceptor.local_endpoint().port();
            address = ipAddress + ":" + std::to_string(_portNumber);
            //创建连接信息的文件
            conInfoWriter conInfo(acceptorName,requesterName,tag,acceptorRank,_addressDirectory);
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
            _sockets[0] = std::move(socket);
            _sockets[0]->write_some(boost::asio::buffer(&requesterRank,sizeof(int)));
            int acceptorRank = -1;
            _sockets[0]->read_some(boost::asio::buffer(&acceptorRank,sizeof(int)));
            _sockets[0]->write_some(boost::asio::buffer(&requesterCommunicationSize,sizeof(int)));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"requestConnection请求连接时出错");
        }
        
        _work = std::make_shared<IO_service::work>(*_service);
        _thread = std::thread([this]{_service->run();});
    }
                           

    void SocketCommunication::send(int itemstoSend,Rank rankReceiver)
    {
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

    void SocketCommunication::send(std::string const &itemstoSend,Rank rankReceiver)
    {
        rankReceiver = adjustRank(rankReceiver);
        try
        {
            _sockets[rankReceiver]->write_some(boost::asio::buffer(&itemstoSend,itemstoSend.length()));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"发送string数据出错");
        } 
    }

    void SocketCommunication::receive(int &itemstoReceive,Rank rankSender)
    {
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

    void SocketCommunication::receive(std::string &itemstoReceive,Rank rankSender)
    {
        rankSender = adjustRank(rankSender);
        try
        {
            _sockets[rankSender]->read_some(boost::asio::buffer(&itemstoReceive,itemstoReceive.length()));
        }
        catch(const std::exception& e)
        {
            Assert(e.what(),"接收string数据出错");
        }
    }

}