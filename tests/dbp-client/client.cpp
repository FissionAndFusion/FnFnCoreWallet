#include "client.h"

client::client() : m_buf(100, 0),
                   m_ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 6815),
                   m_timer(m_io, std::chrono::seconds{30}),
                   sock(new boost::asio::ip::tcp::socket(m_io)),
                   is_connected(false),
                   session("")
{
    start();
}

client::~client()
{
}

void client::start()
{
    // std::shared_ptr<boost::asio::ip::tcp::socket> sock(new boost::asio::ip::tcp::socket(m_io));
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
    // char array[1024 * 10] = {'\0'};
    char *array = new char[len + 4];
    std::memcpy(array, &nl, 4);
    base.SerializeToArray(array + 4, len);
    std::vector<char> ret(array, array + (len + 4));
    delete [] array;
    return ret;
}

void client::send(std::vector<char> buf, std::string explain)
{
    boost::shared_ptr<std::string> pstr(new std::string(explain));
    sock->async_write_some(boost::asio::buffer(buf, buf.size()), boost::bind(&client::write_handler, this, pstr, _1, _2));
}

void client::send_connect(std::string session)
{
    dbp::Connect connect;
    connect.set_session(session);
    connect.set_client("lws-test");
    connect.set_version(1);

    dbp::Base connect_msg = create_msg(dbp::Msg::CONNECT, connect);
    std::vector<char> ret = serialize(connect_msg);
    send(ret, "connect");
}

std::string client::send_ping()
{
    srand(time(NULL));
    int secret = rand();
    char s[20] = {'\0'};
    sprintf(s, "%d", secret);
    std::string id(s);

    dbp::Ping ping;
    ping.set_id(id);
    dbp::Base ping_msg = create_msg(dbp::Msg::PING, ping);
    std::vector<char> ret = serialize(ping_msg);

    char explain[30] = {'\0'};
    sprintf(explain, "ping%d", secret);
    send(ret, explain);
    return id;
}

void client::send_pong(std::string id)
{
    dbp::Pong pong;
    pong.set_id(id);
    dbp::Base pong_msg = create_msg(dbp::Msg::PONG, pong);
    std::vector<char> ret = serialize(pong_msg);
    send(ret, "pong");
}

std::string client::send_sub(std::string name)
{
    srand (time(NULL));
    int secret = rand();
    char s[20] = {'\0'};
    sprintf(s, "%d", secret);
    std::string id(s);

    dbp::Sub sub;
    sub.set_name(name);
    sub.set_id(id);
    dbp::Base sub_msg = create_msg(dbp::Msg::SUB, sub);
    std::vector<char> ret = serialize(sub_msg);

    char explain[30] = {'\0'};
    sprintf(explain, "sub%d", secret);
    send(ret, explain);
    return id;
}

void client::send_unsub(std::string id)
{
    dbp::Unsub unsub;
    unsub.set_id(id);
    dbp::Base unsubn_msg = create_msg(dbp::Msg::UNSUB, unsub);
    std::vector<char> ret = serialize(unsubn_msg);
    send(ret, "unsub");
}

std::string client::send_method(std::string method)
{
    srand(time(NULL));
    int secret = rand();
    char s[20] = {'\0'};
    sprintf(s, "%d", secret);
    std::string id(s);

    dbp::Method obj;
    obj.set_method(method);
    obj.set_id(id);
    dbp::Base obj_msg = create_msg(dbp::Msg::METHOD, obj);
    std::vector<char> ret = serialize(obj_msg);

    char explain[30] = {'\0'};
    sprintf(explain, "method%d", secret);
    send(ret, explain);
    
    return id;
}

void client::conn_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if (ec)
    {
        return;
    }

    send_connect("");
    std::cout << "[conn_handler]recive from " << sock->remote_endpoint().address() << std::endl;
    sock->async_read_some(boost::asio::buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
}

void client::test()
{
    std::string id = send_sub("all-block");
    send_unsub(id);

    std::string mehtod_id = send_method("getblocks");
}

void client::read_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if (ec)
    {
        return;
    }

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
            is_connected = true;
            session = connected.session();

            m_timer.async_wait(boost::bind(&client::timer_handler, this, boost::asio::placeholders::error, sock));
        }

        test();
    }

    if(base.msg() == dbp::Msg::FAILED)
    {
    }

    if(base.msg() == dbp::Msg::PING)
    {
        dbp::Ping ping;
        base.object().UnpackTo(&ping);
        send_pong(ping.id());
    }

    if(base.msg() == dbp::Msg::PONG)
    {
        dbp::Pong pong;
        base.object().UnpackTo(&pong);
    }

    if(base.msg() == dbp::Msg::NOSUB)
    {
        std::cout << "[read_handler]nosub recv" << std::endl;
    }
     
    if(base.msg() == dbp::Msg::READY)
    {
        std::cout << "[read_handler]ready recv" << std::endl;
    }

    if(base.msg() == dbp::Msg::ADDED)
    {
    }

    if(base.msg() == dbp::Msg::CHANGED)
    {
    }

    if(base.msg() == dbp::Msg::REMOVED)
    {
    }

    if(base.msg() == dbp::Msg::RESULT)
    {
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
    std::string id = send_ping();

    m_timer.expires_from_now(std::chrono::seconds{30});
    m_timer.async_wait(boost::bind(&client::timer_handler, this, boost::asio::placeholders::error, sock));
}

void client::run()
{
    m_io.run();
}