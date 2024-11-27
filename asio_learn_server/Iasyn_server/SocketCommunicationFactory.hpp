#pragma once

#include "CommunicationFactory.hpp"
#include "SharedPointer.hpp"

namespace com{
    class SocketCommunicationFactory:public CommunicationFactory
    {
    public:
        SocketCommunicationFactory( unsigned short portNumber       = 0,
                                    bool           reuseAddress     = false,
                                    std::string    networkName      = "lo",
                                    std::string    addressDirectory = ".");

        explicit SocketCommunicationFactory(std::string const &addressDirectory);//explicit关键字要求创建对象时必须显式创建，
  //常量引用的作用是为了避免重新复制原始数据（占内存）

        virtual CommunicationPtr newCommunication() override;//override的作用是显式说明继承自基类的虚函数需要重写，防止派生类忘记重写虚函数而使用基类的默认虚函数的定义。

        virtual std::string AddressDirectory() override;

    private:
        unsigned short portNumber ;
        bool           reuseAddress ;
        std::string    networkName ;
        std::string    addressDirectory ;
    };
}