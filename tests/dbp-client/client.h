#include <iostream>
#include <vector>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/random.hpp>
#include "handlepair.h"
#include "dbp.pb.h"

class DbpStream
{
public:
  DbpStream() {}
  ~DbpStream() {}

  void write(const std::vector<unsigned char> &buf);

  void read(std::vector<unsigned char> &buf, std::size_t n);

  void clear();

  boost::asio::streambuf &get_data();

  std::size_t get_size() const;

private:
  //std::vector<unsigned char> buffer_;
  boost::asio::streambuf buffer_;
  std::mutex mutex_;
};

class Client
{
public:
  Client();
  Client(std::string ip, int port, int version, std::string client);
  ~Client();
  void Run();

  typedef void (*CallBackFn)(Client *cl);
  void SetCallBackFn(CallBackFn cb);
  void SetMethodCallBackFn(CallBackFn cb);
  void SendConnect(std::string session);
  std::string SendPing();
  void SendPong(std::string id);
  std::string SendSub(std::string name, HandlePair hp);
  void SendUnsub(std::string id);
  std::string SendMethod(std::string method, google::protobuf::Any *params, HandlePair hp);
  void Close();

private:
  void Start();
  void SockConnect();
  void ConnHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
  void ReadHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock, std::size_t bytes_size, int len);
  void ReadHeaderHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock, std::size_t bytes_size);
  void TimerHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
  void MethodTimerHandler(const boost::system::error_code &ec, std::shared_ptr<boost::asio::ip::tcp::socket> sock);
  void WriteHandler(boost::shared_ptr<std::string> pstr, const boost::system::error_code &ec, size_t bytes_transferred);
  void ErrorHandler();
  void TestHandle(Client *cl);
  dbp::Base CreateMsg(dbp::Msg msg, google::protobuf::Message &obj);
  std::vector<char> Serialize(dbp::Base base);
  void Send(std::vector<char> buf, std::string explain);
  uint Random();

private:
  boost::asio::io_service m_io_;

  std::vector<char> m_buf_;
  DbpStream ssRecv;
  DbpStream ssSend;

  boost::asio::ip::tcp::endpoint m_ep_;
  std::shared_ptr<boost::asio::steady_timer> m_timer_;
  int timer_expires_;
  std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
  bool is_connected_;
  std::string session_;
  std::string client_;
  int version_;
  CallBackFn cb_;
  CallBackFn method_cb_;
  std::map<std::string, HandlePair> sub_map_;
  std::map<std::string, HandlePair> method_map_;
  std::shared_ptr<boost::asio::steady_timer> test_timer_;

protected:
};