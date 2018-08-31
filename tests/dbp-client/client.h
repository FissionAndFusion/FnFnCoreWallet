#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include "dbp.pb.h"

class client
{
  public:
    client();
    ~client();
    void run();

    void send_connect(std::string session);
    std::string send_ping();
    void send_pong(std::string id);
    std::string send_sub(std::string name);
    void send_unsub(std::string id);
    std::string send_method(std::string method);

  private:
    void start();
    void conn_handler(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void read_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void timer_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void write_handler(boost::shared_ptr<std::string> pstr, const boost::system::error_code &ec, size_t bytes_transferred);
    dbp::Base create_msg(dbp::Msg msg, google::protobuf::Message &obj);
    std::vector<char> serialize(dbp::Base base);
    void send(std::vector<char> buf, std::string explain);
    void test();

  private:
    boost::asio::io_service m_io;
    std::vector<char> m_buf;
    boost::asio::ip::tcp::endpoint m_ep;
    boost::asio::steady_timer m_timer;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock;
    bool is_connected;
    std::string session;

  protected:
};