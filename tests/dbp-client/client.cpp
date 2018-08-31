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

dbp::Base client::create_msg(dbp::Msg msg, google::protobuf::Message &obj)
{
    dbp::Base base;
    base.set_msg(msg);
    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(obj);
    base.set_allocated_object(any);
    return base;
}

std::vector<char> client::serialize(dbp::Base base)
{
    uint32_t len = base.ByteSize();
    int nl = htonl(len);
    char array[1024 * 10] = {'\0'};
    std::memcpy(array, &nl, 4);
    base.SerializeToArray(array + 4, len);
    // std::vector<char> ret(array, array + sizeof(array)/sizeof(array[0]));
    std::vector<char> ret(array, array + (len + 4));
    return ret;
}

void client::conn_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if(ec)
    {
        return;
    }

    dbp::Connect connect;
    connect.set_session("");
    connect.set_client("lws-test");
    connect.set_version(1);

    dbp::Base connect_msg = create_msg(dbp::Msg::CONNECT, connect);
    std::vector<char> ret = serialize(connect_msg);

    boost::system::error_code ec1;
    sock->write_some(boost::asio::buffer(ret),ec1);

    if(ec1 != boost::system::errc::success)
    {
        std::cerr << "[conn_handler]write_some failed" << std::endl;
        return;
    }

    std::cout << "[conn_handler]recive from " << sock->remote_endpoint().address() << std::endl;
    sock->async_read_some(boost::asio::buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
}

void client::read_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if(ec)
    {
        return;
    }

    // std::cerr << "[read_handler]msg recved" << std::endl;
    uint32_t b;
    std::memcpy(&b, &m_buf[0], 4);
    uint32_t len = ntohl(b);

    dbp::Base base;
    if(!base.ParseFromArray(&m_buf[4], len))
    {
        std::cerr << "[read_handler]parse base msg false" << std::endl;
        return;
    }

    if(base.msg() == dbp::Msg::CONNECTED)
    {
        if(base.object().Is<dbp::Connected>())
        {
            dbp::Connected connected;
            base.object().UnpackTo(&connected);

            std::cout << "[read_handler]connected session is:" << connected.session() << std::endl; 

            m_timer.async_wait(boost::bind(&client::timer_handler, this, boost::asio::placeholders::error, sock));
        }

        dbp::Sub sub;
        sub.set_name("all-block");
        sub.set_id("123456");

        dbp::Base sub_msg = create_msg(dbp::Msg::SUB, sub);
        std::vector<char> ret = serialize(sub_msg);
        boost::shared_ptr<std::string> pstr(new std::string("sub"));
        sock->async_write_some(boost::asio::buffer(ret), boost::bind(&client::write_handler, this, pstr, _1, _2));
    }

    if(base.msg() == dbp::Msg::PONG)
    {
        std::cout << "[read_handler]pong recv" << std::endl;
    }

    if(base.msg() == dbp::Msg::NOSUB)
    {
        std::cout << "[read_handler]nosub recv" << std::endl;
    }
     
    if(base.msg() == dbp::Msg::READY)
    {
        std::cout << "[read_handler]ready recv" << std::endl;
    }

    sock->async_read_some(boost::asio::buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
}

void client::write_handler(boost::shared_ptr<std::string> pstr, const boost::system::error_code &ec, size_t bytes_transferred)
{
  if (ec)
  {
      std::cerr << "[write_handler]" << *pstr << " msg write error:" << ec << ". len:" << bytes_transferred << std::endl;
      return;
  }
  std::cout << "[write_handler]" << *pstr << " msg write succeed. len:" << bytes_transferred << std::endl;
}

void client::timer_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    dbp::Ping ping;
    ping.set_id("");
    dbp::Base ping_msg = create_msg(dbp::Msg::PING, ping);
    std::vector<char> ret = serialize(ping_msg);
    boost::shared_ptr<std::string> pstr(new std::string("ping"));
    sock->async_write_some(boost::asio::buffer(ret), boost::bind(&client::write_handler, this, pstr, _1, _2));

    m_timer.expires_from_now(std::chrono::seconds{30});
    m_timer.async_wait(boost::bind(&client::timer_handler, this, boost::asio::placeholders::error, sock));
}

void client::run()
{
    m_io.run();
}