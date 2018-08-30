#include "dbp.pb.h"
#include "client.h"

client::client() : m_buf(100, 0),
                   m_ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 6815),
                   m_timer(m_io, std::chrono::seconds{30})
{
    start();
}

client::~client()
{
}

void client::start()
{
    std::shared_ptr<boost::asio::ip::tcp::socket> sock(new boost::asio::ip::tcp::socket(m_io));
    sock->async_connect(m_ep, boost::bind(&client::conn_handler, this, boost::asio::placeholders::error, sock));
}

void client::conn_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if(ec)
    {
        return;
    }

    dbp::Base connectMsgBase;
    connectMsgBase.set_msg(dbp::Msg::CONNECT);

    dbp::Connect connectMsg;
    connectMsg.set_session("");
    connectMsg.set_client("lws-test");
    connectMsg.set_version(1);

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


    boost::system::error_code ec1;
    sock->write_some(boost::asio::buffer(sendBuf,sizeByte + 4),ec1);

    if(ec1 != boost::system::errc::success)
    {
        std::cerr << "write_some failed" << std::endl;
        return;
    }


    std::cout << "recive from " << sock->remote_endpoint().address() << std::endl;
    sock->async_read_some(boost::asio::buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
}

void client::read_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if(ec)
    {
        return;
    }

    uint32_t b;
    std::memcpy(&b, &m_buf[0], 4);
    uint32_t len = ntohl(b);

    // std::cout << &m_buf[0] << std::endl;
    // std::cout << len << std::endl;

    dbp::Base msgBase;
    if(!msgBase.ParseFromArray(&m_buf[4], len))
    {
        std::cerr << "parse base msg false" << std::endl;
        return;
    }

    if(msgBase.msg() == dbp::Msg::CONNECTED)
    {
        if(msgBase.object().Is<dbp::Connected>())
        {
            dbp::Connected connectedMsg;
            msgBase.object().UnpackTo(&connectedMsg);

            std::cout << "connected session is:" << connectedMsg.session() << std::endl; 

            // m_timer.async_wait([](const boost::system::error_code &ec) { std::cout << "3 sec\n"; });
            m_timer.async_wait(boost::bind(&client::timer_handler, this, boost::asio::placeholders::error));
        }
    }

    sock->async_read_some(boost::asio::buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
}

void client::timer_handler(const boost::system::error_code &ec)
{
    std::cout << "30 sec send ping msg" << std::endl;
    m_timer.expires_from_now(std::chrono::seconds{30});
    m_timer.async_wait(boost::bind(&client::timer_handler, this, boost::asio::placeholders::error));
}

void client::run()
{
    m_io.run();
}