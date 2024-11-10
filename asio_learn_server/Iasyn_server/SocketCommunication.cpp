#include "SocketCommunication.hpp"
#include "utils/assertion.hpp"
#include "getIpAddress.hpp"
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
        Assert(_connected);
        SetRankOffset(rankOffset);
        std::string address;
        try
        {
            std::string ipAddress = getIpAddress();
            using boost::asio::ip::tcp;
            tcp::endpoint endpoint(tcp::v4(),_portNumber);
            tcp::acceptor acceptor(*_service,endpoint);
            acceptor.set_option(tcp::acceptor::reuse_address(_reuseAddress));

            _portNumber=acceptor.local_endpoint().port();//获取实际系统分配的端口号
            address = ipAddress + ":" + std::to_string(_portNumber);

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    } 


}