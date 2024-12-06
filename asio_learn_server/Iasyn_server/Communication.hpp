#pragma once

#include <stddef.h>
#include <string>
#include <set>
#include <span>
#include <boost/range/irange.hpp>
#include "utils/assertion.hpp"
#include "SharedPointer.hpp"
#include "Request.hpp"

namespace com{

using Rank = int;
template<typename T>
struct AsRangeTag{};
class Communication{
public:
    Communication &operator=(Communication &&) = delete;//禁止com1=std::move(com0)这种操作

    virtual ~Communication(){}

    virtual bool isconnected()
    {
        return _connected;
    }

    virtual size_t getRemoteCommunicatorSize() = 0;

    auto remoteCommunicatorRanks()
    {
        return boost::irange<Rank>(0,static_cast<Rank>(getRemoteCommunicatorSize()));
    }
    /*
    *接受多个客户端的连接，每个客户端必须有不同的requesterRank，相同的requesterCommunicationSize
    *1、用于接收组间请求，另一组的进程号从0～N-1,总共N个进程必须都建立连接，也就是要求请求方的进程号有序。
    *2、用于接收组内请求，默认调用acceptConnection的主进程号为0,并接收剩余1～N-1从进程的连接请求，RankOffset
    *   设置为1。
    */
    virtual void acceptConnection(std::string const &acceptorName,
                                  std::string const &requesterName,
                                  std::string const &tag,
                                  int acceptorRank,
                                  int rankoffset = 0) = 0;
    //用于组间通信
    virtual void acceptConnectionAsServer(std::string const &acceptorName,
                                        std::string const &requesterName,
                                        std::string const &tag,
                                        int                acceptorRank,
                                        int                requesterCommunicatorSize) = 0;
            
    //请求连接至服务端，每一个SocketCommunicaton对象只能调用一次该方法。
    virtual void requestConnection(std::string const &acceptorName,
                                   std::string const &requesterName,
                                   std::string const &tag,
                                   int requesterRank,
                                   int requesterCommunicationSize) = 0;

    virtual void requestConnectionAsClient(std::string const &  acceptorName,
                                         std::string const &  requesterName,
                                         std::string const &  tag,
                                         std::set<int> const &acceptorRanks,
                                         int                  requesterRank) = 0;
                                        
    void IntraConnect(std::string const &participantName,
                            std::string const &tag,
                            int               rank,
                            int               size);
    
    
    ///发送字符串
    virtual void send(std::string const &itemstoSend,Rank rankReceiver) = 0;
    ///发送int数据
    virtual void send(const int &itemstoSend,Rank rankReceiver) = 0;
    ///异步发送int数据
    virtual RequestPtr aSend(const int &itemstoSend,Rank rankReceiver) = 0;
    ///发送double数据
    virtual void send(const double&itemstoSend,Rank rankReceiver) = 0;
    ///异步发送double数据
    virtual RequestPtr aSend(const double &itemstoSend,Rank rankReceiver) = 0;
    ///发送bool数据
    virtual void send(bool itemstoSend,Rank rankReceiver) = 0;
    ///异步发送bool数据
    virtual RequestPtr aSend(const bool &itemstoSend,Rank rankReceiver) = 0;
    ///发送一组int数据
    virtual void send(std::span<const int> itemstoSend,Rank rankReceiver) = 0;
    ///异步发送一组int数据
    virtual RequestPtr aSend(std::span<const int> itemstoSend,Rank rankReceiver) = 0;
    ///发送一组double数据
    virtual void send(std::span<const double> itemstoSend,Rank rankReceiver) = 0;
    ///异步发送一组double数据
    virtual RequestPtr aSend(std::span<const double> itemstoSend,Rank rankReceiver) = 0;
    
    
    ///接收字符串
    virtual void receive(std::string &itemstoReceive,Rank rankSender) = 0;
    ///接收int数据
    virtual void receive(int &itemstoReceive,Rank rankSender) = 0;
    ///异步接收int数据
    virtual RequestPtr aReceive(int &itemstoReceive,Rank rankSender) = 0;
    ///接收double数据
    virtual void receive(double &itemstoReceive,Rank rankSender) = 0;
    ///异步接收double数据
    virtual RequestPtr aReceive(double &itemstoReceive,Rank rankSender) = 0;
    ///接收bool数据
    virtual void receive(bool &itemstoReceive,Rank rankSender) = 0;
    ///异步接收bool数据
    virtual RequestPtr aReceive(bool &itemstoReceive,Rank rankSender) = 0;
    ///接收一组int数据
    virtual void receive(std::span<int> itemstoReceive,Rank rankSender) = 0;
    ///异步接收一组int数据
    virtual RequestPtr aReceive(std::span<int> itemstoReceive,Rank rankSender) = 0;
    ///接收一组double数据
    virtual void receive(std::span<double> itemstoReceive,Rank rankSender) = 0;
    ///异步接收一组double数据
    virtual RequestPtr aReceive(std::span<double> itemstoReceive,Rank rankSender) = 0;


    ///同步发送double序列，大小+内容
    void sendRange(std::span<const double> itemstoSend, Rank rankReceiver);
    ///同步发送int序列，大小+内容
    void sendRange(std::span<const int> itemstoSend, Rank rankReceiver);
    /// 同步接收int序列，并返回vector
    std::vector<int> receiveRange(Rank rankSender, AsRangeTag<int>);
    /// 同步接收double序列，并返回vector
    std::vector<double> receiveRange(Rank rankSender, AsRangeTag<double>);


    /// 主进程调用,从各从进程接收数据并与主进程数据汇总
    virtual void reduceSumForPrimaryRank(int itemstoSend,int &itemstoreceive);
    /// 从进程调用，发送各自的数据到主进程
    virtual void reduceSumForSecondaryRank(int &itemstoSend,Rank PrimaryRank);
    /// 主进程调用，从各从进程接收数据并汇总，再将汇总后的数据发送至各从进程
    virtual void AllreduceSumForPrimaryRank(int itemstoSend,int &itemstoReceive);
    /// 从进程调用，发送各自的数据到主进程，并从主进程接收数据
    virtual void AllreduceSumForSecondaryRank(int &itemstoSend,int &itemstoReceive,Rank PrimaryRank);
   
    /// 主进程调用,从各从进程接收span<int>数据并与主进程数据汇总
    virtual void reduceSumForPrimaryRank(std::span<const int> itemstoSend,std::span<int> itemstoReceive);
    /// 从进程调用，发送各自的span<int>数据到主进程
    virtual void reduceSumForSecondaryRank(std::span<const int> itemstoSend,Rank PrimaryRank);
    /// 主进程调用，从各从进程接收span<int>数据并汇总，再将汇总后的span<int>数据发送至各从进程
    virtual void AllreduceSumForPrimaryRank(std::span<const int> itemstoSend,std::span<int> itemstoReceive);
    /// 从进程调用，发送各自的span<int>数据到主进程，并从主进程接收span<int>数据
    virtual void AllreduceSumForSecondaryRank(std::span<const int> itemstoSend,std::span<int> itemstoReceive,Rank PrimaryRank);
    
    /// 主进程调用,从各从进程接收数据并与主进程数据汇总
    virtual void reduceSumForPrimaryRank(double itemstoSend,double &itemstoreceive);
    /// 从进程调用，发送各自的数据到主进程
    virtual void reduceSumForSecondaryRank(double &itemstoSend,Rank PrimaryRank);
    /// 主进程调用，从各从进程接收数据并汇总，再将汇总后的数据发送至各从进程
    virtual void AllreduceSumForPrimaryRank(double itemstoSend,double &itemstoReceive);
    /// 从进程调用，发送各自的数据到主进程，并从主进程接收数据
    virtual void AllreduceSumForSecondaryRank(double &itemstoSend,double &itemstoReceive,Rank PrimaryRank);

    /// 主进程调用,从各从进程接收span<double>数据并与主进程数据汇总
    virtual void reduceSumForPrimaryRank(std::span<const double> itemstoSend,std::span<double> itemstoReceive);
    /// 从进程调用，发送各自的span<double>数据到主进程
    virtual void reduceSumForSecondaryRank(std::span<const double> itemstoSend,Rank PrimaryRank);
    /// 主进程调用，从各从进程接收span<double>数据并汇总，再将汇总后的span<int>数据发送至各从进程
    virtual void AllreduceSumForPrimaryRank(std::span<const double> itemstoSend,std::span<double> itemstoReceive);
    /// 从进程调用，发送各自的span<double>数据到主进程，并从主进程接收span<int>数据
    virtual void AllreduceSumForSecondaryRank(std::span<const double> itemstoSend,std::span<double> itemstoReceive,Rank PrimaryRank);


    /// 用作接收的对象为std::vector<double>,该参数不需要设置大小
    virtual void AllreduceSumForPrimaryRank(std::span<const double> itemstoSend,std::vector<double> &itemstoReceive);
    /// 用作接收的对象为std::vector<double>,该参数不需要设置大小
    virtual void AllreduceSumForSecondaryRank(std::span<const double> itemstoSend,std::vector<double> &itemstoReceive,Rank PrimaryRank);
    /// 用作接收的对象为std::vector<int>,该参数不需要设置大小
    virtual void AllreduceSumForPrimaryRank(std::span<const int> itemstoSend,std::vector<int> &itemstoReceive);
    /// 用作接收的对象为std::vector<int>,该参数不需要设置大小
    virtual void AllreduceSumForSecondaryRank(std::span<const int> itemstoSend,std::vector<int> &itemstoReceive,Rank PrimaryRank);



    virtual void broadcastForPrimaryRank(int itemstoSend);
    virtual void broadcastForSecondaryRank(int &itemstoReceive, Rank rankBroadcaster);

    virtual void broadcastForPrimaryRank(std::span<const int> itemstoSend);
    virtual void broadcastForSecondaryRank(std::span<int> itemstoReceive, Rank rankBroadcaster);

    virtual void broadcastForPrimaryRank(double itemstoSend);
    virtual void broadcastForSecondaryRank(double &itemstoReceive, Rank rankBroadcaster);

    virtual void broadcastForPrimaryRank(std::span<const double> itemstoSend);
    virtual void broadcastForSecondaryRank(std::span<double> itemstoReceive, Rank rankBroadcaster);

    virtual void broadcastForPrimaryRank(bool itemstoSend);
    virtual void broadcastForSecondaryRank(bool &itemstoReceive, Rank rankBroadcaster);

    virtual void broadcastForPrimaryRank(std::vector<int> const &v);
    virtual void broadcastForSecondaryRank(std::vector<int> &v, Rank rankBroadcaster);

    virtual void broadcastForPrimaryRank(std::vector<double> const &v);
    virtual void broadcastForSecondaryRank(std::vector<double> &v, Rank rankBroadcaster);



    virtual void closeConnection() = 0;

    virtual void prepareEstablishment(std::string const &acceptorName,std::string const &requesterName) = 0;
    virtual void cleanupEstablishment(std::string const &acceptorName,std::string const &requesterName) = 0;

    void SetRankOffset(Rank rankOffset)
    {
        _RankOffset = rankOffset;
    }

protected:
    int _RankOffset = 0;
    bool _connected = false;
    virtual int adjustRank(Rank rank) const;

};

    void connectSeries(
    std::string const & participantName,
    std::string const & addressDirectory,
    std::string const & tag,
    int                 rank,
    int                 size,
    CommunicationPtr Comm);
}