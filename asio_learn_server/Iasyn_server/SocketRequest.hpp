#pragma once

#include <condition_variable>
#include <mutex>
#include "Request.hpp"

namespace com{
    class SocketRequest:public Request
    {
        public:
        void complete();

        bool test() override;

        void wait() override;

        virtual ~SocketRequest();

        private:
        bool _complete{false};
        std::mutex _completeMutex;
        std::condition_variable _completeCondition;

    };
}