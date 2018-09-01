#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include "dbp.pb.h"

class Client
{
  public:
    Client();
    ~Client();
    void Run();

    void SendConnect(std::string session);
    std::string SendPing();
    void SendPong(std::string id);
    std::string SendSub(std::string name);
    void SendUnsub(std::string id);
    std::string SendMethod(std::string method);
    void Close();

  private:
    void Start();
    void ConnHandler(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void ReadHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void TimerHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void WriteHandler(boost::shared_ptr<std::string> pstr, const boost::system::error_code &ec, size_t bytes_transferred);
    dbp::Base CreateMsg(dbp::Msg msg, google::protobuf::Message &obj);
    std::vector<char> Serialize(dbp::Base base);
    void Send(std::vector<char> buf, std::string explain);
    void Test();

  private:
    boost::asio::io_service m_io_;
    std::vector<char> m_buf_;
    boost::asio::ip::tcp::endpoint m_ep_;
    boost::asio::steady_timer m_timer_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
    bool is_connected_;
    std::string session_;

  protected:
};