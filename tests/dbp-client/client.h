#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>

class client
{
  public:
    client();
    ~client();
    void run();

  private:
    void start();
    void conn_handler(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void read_handler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
    void timer_handler(const boost::system::error_code &ec);

  private:
    boost::asio::io_service m_io;
    std::vector<char> m_buf;
    boost::asio::ip::tcp::endpoint m_ep;
    boost::asio::steady_timer m_timer;

  protected:
};