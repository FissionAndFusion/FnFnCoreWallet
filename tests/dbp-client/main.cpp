#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

#include <arpa/inet.h>

#include "client.h"
#include "dbp.pb.h"
#include "lws.pb.h"

void sendConnect(
 const std::string& session,
 const std::string& client,
 int version
) 
{
    boost::asio::io_service service;
    boost::asio::ip::tcp::socket socket(service);

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 
    6815);

    socket.connect(ep);

    // write connect msg

    dbp::Base connectMsgBase;
    connectMsgBase.set_msg(dbp::Msg::CONNECT);

    dbp::Connect connectMsg;
    connectMsg.set_session(session);
    connectMsg.set_client(client);
    connectMsg.set_version(version);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connectMsg);
    connectMsgBase.set_allocated_object(any);

    int sizeByte = connectMsgBase.ByteSize();
    char serilizedBuf[sizeByte];
    char sendBuf[1024 * 10 + 4] = {0};

    connectMsgBase.SerializeToArray(serilizedBuf,sizeByte);
    

    int networkOrder = htonl(sizeByte);
    std::memcpy(sendBuf,&networkOrder,4);
    std::memcpy(sendBuf + 4,serilizedBuf,sizeByte);


    boost::system::error_code ec;
    socket.write_some(boost::asio::buffer(sendBuf,sizeByte + 4),ec);

    if(ec != boost::system::errc::success)
    {
        std::cerr << "write_some failed" << std::endl;
        return;
    }

    // read connected or failed msg

    char recvBuf[1024*10 + 4] = {0};
    char len[4] = {0};
    socket.read_some(boost::asio::buffer(len,4),ec);
    if(ec != boost::system::errc::success)
    {
        std::cerr << "read_some failed" << std::endl;
        return;
    }

    int nLenHost,nLenNetWorkOrder;
    std::memcpy(&nLenNetWorkOrder,len,4);
    nLenHost = ntohl(nLenNetWorkOrder);

    socket.read_some(boost::asio::buffer(recvBuf,nLenHost),ec);
    if(ec != boost::system::errc::success)
    {
        std::cerr << "read_some failed" << std::endl;
        return;
    }

    dbp::Base msgBase;
    if(!msgBase.ParseFromArray(recvBuf,nLenHost))
    {
        std::cerr << "parse base msg false"
        << std::endl;
        return;
    }

    if(msgBase.msg() == dbp::Msg::CONNECTED)
    {
        if(msgBase.object().Is<dbp::Connected>())
        {
            dbp::Connected connectedMsg;
            msgBase.object().UnpackTo(&connectedMsg);

            std::cout << "connected session is:" <<
            connectedMsg.session() << std::endl; 
        }
    }
    else if(msgBase.msg() == dbp::Msg::FAILED)
    {
        if(msgBase.object().Is<dbp::Failed>())
        {
            dbp::Failed failedMsg;
            msgBase.object().UnpackTo(&failedMsg);

            std::cout << "server support version is:"
            << std::endl;
            for(const int& version : failedMsg.version())
            {
                std::cout << version << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "unknown msg type" << std::endl;
    }

    socket.close();

}

void help()
{
    std::cerr << "usage: dbpclient [msgtype]"
        << std::endl
        << "msgtype:" << std::endl
        << "connect [session] [client] [version]"
        << std::endl;
}

void run()
{
    try
    {
        Client cl;
        cl.Run();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    run();
    return 0;

    std::string msgType(argv[1]);
    
    if(msgType == "help")
    {
        help();
    }
    else if(msgType == "connect")
    {
        std::string session(argv[2]);
        std::string client(argv[3]);
        std::string version(argv[4]);
        sendConnect(session,client,
        std::atoi(version.c_str()));
    }
    else
    {
        std::cerr << "error: msg type invalid"
        << std::endl;
        help();
    }
    
    return 0;
}