#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

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

  private:
    boost::asio::io_service m_io;
    std::vector<char> m_buf;
    boost::asio::ip::tcp::endpoint m_ep;

  protected:
};