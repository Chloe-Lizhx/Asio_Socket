#include <mutex>
#include <thread>
#include <boost/thread/thread.hpp>
#include <boost/asio/strand.hpp>
#include "server.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
std::mutex m;

void multhread(io_service &service)
{
    service.run();  
}

void show(int i)
{
    lock_guard<mutex> lk(m);
    std::cout<<"[" << std::this_thread::get_id() << "] "<<"这是第 "<<i<<" 个消息"<<std::endl;
}
int main()
{
    std::shared_ptr<io_service> service(new io_service);
    std::shared_ptr<io_service::work> work(new io_service::work(*service));
    boost::asio::io_service::strand strand(*service);
    std::shared_ptr<Server> server(new Server(*service,5005));//只有shared_ptr<Server>定义的对象才能使用shared_from_this()函数
    server->do_repeat();
    boost::thread_group work_threads;
    for(int i=0;i<3;i++)
    {
        work_threads.create_thread(boost::bind(multhread,ref(*service)));
    }
    // for(int i=0;i<10;i++)
    // {
    //     //下面这句可以保证show函数在同一个线程中按顺序执行，但不能保证只在一个线程中执行
    //     //service->post(strand.wrap(boost::bind(show,i)));
    //     strand.dispatch(boost::bind(show,i));//这一句可以保证只在一个线程中顺序执行
    // }
    work.reset();
    work_threads.join_all();
    return 0;
}