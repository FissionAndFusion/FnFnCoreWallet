#include "client.h"

Client::Client()
{
}

Client::Client(std::string ip, int port, int version, std::string client)
    : client_(client),
      version_(version),
      m_ep_(boost::asio::ip::address_v4::from_string(ip), port),
      m_buf_(4096, 0),
      is_connected_(false),
      session_("")
{
    Start();
}

Client::~Client()
{
}

void Client::SockConnect()
{
    boost::system::error_code ec;
    do
    {
        sock_.reset(new boost::asio::ip::tcp::socket(m_io_));
        sock_->connect(m_ep_, ec);
        std::cerr << "socket connection: " << ec.message() << std::endl;

        if (0 != ec)
        {
            sleep(1);
        }
    } while (ec != 0);

    is_connected_ = true;
    SendConnect(session_);
    sock_->async_read_some(boost::asio::buffer(m_buf_), boost::bind(&Client::ReadHandler, this, boost::asio::placeholders::error, sock_));
}

void Client::Start()
{
    // sock_.reset(new boost::asio::ip::tcp::socket(m_io_));
    // sock_->async_connect(m_ep_, boost::bind(&Client::ConnHandler, this, boost::asio::placeholders::error, sock_));
    SockConnect();
}

dbp::Base Client::CreateMsg(dbp::Msg msg, google::protobuf::Message &obj)
{
    dbp::Base base;
    base.set_msg(msg);
    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(obj);
    base.set_allocated_object(any);
    return base;
}

std::vector<char> Client::Serialize(dbp::Base base)
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

void Client::Send(std::vector<char> buf, std::string explain)
{
    char *b = buf.data();
    int size_buf = ntohl(*((uint32_t *)b));
    // std::cout << "size of buf:" << size_buf << std::endl;

    dbp::Base base;
    if(!base.ParseFromArray(&b[4], size_buf))
    {
        std::cerr << "[send]parse base msg false" << std::endl;
        return;
    }

    boost::shared_ptr<std::string> pstr(new std::string(explain));
    // sock_->async_write_some(boost::asio::buffer(buf, buf.size()), boost::bind(&Client::WriteHandler, this, pstr, _1, _2));
    boost::asio::async_write(*sock_, boost::asio::buffer(buf), boost::bind(&Client::WriteHandler, this, pstr, _1, _2));
}

void Client::SendConnect(std::string session)
{
    dbp::Connect connect;
    connect.set_session(session);
    if("" == session)
    {
        connect.set_client(client_);
        connect.set_version(version_);
    }

    dbp::Base connect_msg = CreateMsg(dbp::Msg::CONNECT, connect);
    std::vector<char> ret = Serialize(connect_msg);
    Send(ret, "connect" + session);
}

uint Client::Random()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long seed = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    boost::random::mt19937 rng(seed);
    int ret = rng();
    return ret;
}

std::string Client::SendPing()
{
    std::string id(std::to_string(Random()));

    dbp::Ping ping;
    ping.set_id(id);
    dbp::Base ping_msg = CreateMsg(dbp::Msg::PING, ping);
    std::vector<char> ret = Serialize(ping_msg);
    Send(ret, "ping" + id);
    return id;
}

void Client::SendPong(std::string id)
{
    dbp::Pong pong;
    pong.set_id(id);
    dbp::Base pong_msg = CreateMsg(dbp::Msg::PONG, pong);
    std::vector<char> ret = Serialize(pong_msg);
    Send(ret, "pong" + id);
}

std::string Client::SendSub(std::string name, HandlePair hp)
{
    std::string id(std::to_string(Random()));

    dbp::Sub sub;
    sub.set_name(name);
    sub.set_id(id);
    dbp::Base sub_msg = CreateMsg(dbp::Msg::SUB, sub);
    std::vector<char> ret = Serialize(sub_msg);
    Send(ret, "sub" + id);

    hp.name = name;
    hp.id = id;
    hp.type = "sub";
    sub_map_.insert({id, hp});

    return id;
}

void Client::SendUnsub(std::string id)
{
    dbp::Unsub unsub;
    unsub.set_id(id);
    dbp::Base unsubn_msg = CreateMsg(dbp::Msg::UNSUB, unsub);
    std::vector<char> ret = Serialize(unsubn_msg);
    Send(ret, "unsub");
    sub_map_.erase(id);
}

std::string Client::SendMethod(std::string method, HandlePair hp)
{
    std::string id(std::to_string(Random()));

    dbp::Method obj;
    obj.set_method(method);
    obj.set_id(id);
    dbp::Base obj_msg = CreateMsg(dbp::Msg::METHOD, obj);
    std::vector<char> ret = Serialize(obj_msg);
    Send(ret, "method" + id);

    hp.name = method;
    hp.id = id;
    hp.type = "method";
    method_map_.insert({id, hp});

    return id;
}

void Client::ConnHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if (ec)
    {
        std::cerr << "connection failed: " << ec.message() << std::endl;
        return;
    }

    SendConnect("");
    std::cout << "[conn_handler]recive from " << sock->remote_endpoint().address() << std::endl;
    sock_->async_read_some(boost::asio::buffer(m_buf_), boost::bind(&Client::ReadHandler, this, boost::asio::placeholders::error, sock));
}

void Client::ErrorHandler()
{
    m_timer_->cancel();
    SockConnect();
}

void Client::SetCallBackFn(CallBackFn cb)
{
    cb_ = cb;
}

void Client::TestHandle(Client *cl)
{
    cb_(cl);
}

void Client::ReadHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if (ec)
    {

        std::cerr << "read connection: " << ec.message() << ",code:" << ec << std::endl;
        is_connected_ = false;
        m_io_.post(boost::bind(&Client::ErrorHandler, this));
        return;
    }

    uint32_t b;
    std::memcpy(&b, &m_buf_[0], 4);
    uint32_t len = ntohl(b);

    dbp::Base base;
    if(!base.ParseFromArray(&m_buf_[4], len))
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
            is_connected_ = true;
            session_ = connected.session();

            m_timer_.reset(new boost::asio::steady_timer(m_io_, std::chrono::seconds{1}));
            m_timer_->async_wait(boost::bind(&Client::TimerHandler, this, boost::asio::placeholders::error, sock));
        }

        m_io_.post(boost::bind(&Client::TestHandle, this, this));
    }

    if(base.msg() == dbp::Msg::FAILED)
    {
        dbp::Failed failed;
        base.object().UnpackTo(&failed);
        if("002" == failed.reason())
        {
            session_ = "";
            SendConnect(session_);
        }

        if("001" == failed.reason())
        {
            session_ = "";
            version_ = failed.version()[0];
            SendConnect(session_);
        }

        if("003" == failed.reason())
        {
            session_ = "";
            SendConnect(session_);
        }
    }

    if(base.msg() == dbp::Msg::PING)
    {
        std::cout << "[read_handler]ping recv" << std::endl;
        dbp::Ping ping;
        base.object().UnpackTo(&ping);
        SendPong(ping.id());
    }

    if(base.msg() == dbp::Msg::PONG)
    {
        std::cout << "[read_handler]pong recv" << std::endl;
        dbp::Pong pong;
        base.object().UnpackTo(&pong);
    }

    if(base.msg() == dbp::Msg::NOSUB)
    {
        std::cout << "[read_handler]nosub recv" << std::endl;
        dbp::Nosub nosub;
        base.object().UnpackTo(&nosub);
        sub_map_.erase(nosub.id());
    }
     
    if(base.msg() == dbp::Msg::READY)
    {
        std::cout << "[read_handler]ready recv" << std::endl;
        dbp::Ready ready;
        base.object().UnpackTo(&ready);
        sub_map_[ready.id()].enable = true;
    }

    if(base.msg() == dbp::Msg::ADDED)
    {
        dbp::Added added;
        base.object().UnpackTo(&added);
        sub_map_[added.id()].SubHandler("added", added.name(), added.object());
    }

    if(base.msg() == dbp::Msg::CHANGED)
    {
    }

    if(base.msg() == dbp::Msg::REMOVED)
    {
    }

    if(base.msg() == dbp::Msg::RESULT)
    {
        dbp::Result result;
        base.object().UnpackTo(&result);
        method_map_[result.id()].MethodHandler();
        method_map_.erase(result.id());
    }

    if(base.msg() == dbp::Msg::ERROR)
    {
        dbp::Error error;
        base.object().UnpackTo(&error);
        std::cout << "[read_handler]error reason:" << error.reason() << ", explain:" << error.explain() << std::endl;
    }

    sock->async_read_some(boost::asio::buffer(m_buf_), boost::bind(&Client::ReadHandler, this, boost::asio::placeholders::error, sock));
}

void Client::WriteHandler(boost::shared_ptr<std::string> pstr, const boost::system::error_code &ec, size_t bytes_transferred)
{
  if (ec)
  {
      std::cerr << "[write_handler]" << *pstr << " msg write error:" << ec.message() << ". len:" << bytes_transferred << std::endl;
      is_connected_ = false;
      m_io_.post(boost::bind(&Client::ErrorHandler, this));
      return;
  }
  std::cout << "[write_handler]" << *pstr << " msg write succeed. len:" << bytes_transferred << std::endl;
}

void Client::TimerHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if(ec)
    {
        std::cerr << "cancel timer" << std::endl;
        return;
    }

    if(is_connected_)
    {
        std::string id = SendPing();
    }

    m_timer_->expires_from_now(std::chrono::seconds{1});
    m_timer_->async_wait(boost::bind(&Client::TimerHandler, this, boost::asio::placeholders::error, sock));
}

void Client::Run()
{
    m_io_.run();
}

void Client::Close()
{
    boost::system::error_code ec;
    sock_->shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    sock_->close(ec);
}