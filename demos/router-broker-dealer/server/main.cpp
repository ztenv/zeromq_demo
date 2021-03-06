#include <ctime>

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>

#include <zmqpp/context.hpp>
#include <zmqpp/loop.hpp>
#include <zmqpp/message.hpp>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>
#include <zmqpp/socket_options.hpp>
#include <zmqpp/socket_types.hpp>
#include <zmqpp/zmqpp.hpp>

#include <gflags/gflags.h>


DEFINE_string(id,"server","app id");
DEFINE_string(endpoint,"tcp://127.0.0.1:3333","broker backend endpoint");
DEFINE_int32(send_interval,1000,"send request interval,ms");


using namespace std;

bool send_msg(zmqpp::socket *socket);
bool recv_msg(zmqpp::socket *socket);

int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("usage:");
    gflags::ParseCommandLineFlags(&argc,&argv,true);

    zmqpp::context context;

    zmqpp::socket dealer_socket(context,zmqpp::socket_type::dealer);
    dealer_socket.set(zmqpp::socket_option::identity,FLAGS_id);
    dealer_socket.connect(FLAGS_endpoint);

    zmqpp::loop looper;
    looper.add(std::chrono::milliseconds(FLAGS_send_interval),0,std::bind(send_msg,&dealer_socket));
    looper.add(dealer_socket,std::bind(recv_msg,&dealer_socket),zmqpp::poller::poll_in|zmqpp::poller::poll_error);
    looper.start();
    
    return 0;
}

bool send_msg(zmqpp::socket *socket)
{
    auto time=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&time),"%Y-%m-%d %H:%M:%S");
    string msg(FLAGS_id);
    msg.append(":");
    msg.append(ss.str());

    zmqpp::message mmsg;
    mmsg.add("client0");
    mmsg.add(msg);

    bool res=socket->send(mmsg);
    cout<<"send:["<<msg<<"],res="<<res<<endl;

    return true;
}

bool recv_msg(zmqpp::socket *socket)
{
    zmqpp::message msg;
    bool res=socket->receive(msg);
    string data;
    msg.get(data,1);

    cout<<"recv:["<<data<<"],res="<<res<<endl;
    return true;
}
