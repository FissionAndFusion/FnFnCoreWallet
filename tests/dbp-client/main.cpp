#include <iostream>
#include <vector>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/random.hpp>

#include <arpa/inet.h>

#include "client.h"
#include "handlepair.h"
#include "dbp.pb.h"
#include "lws.pb.h"
#include "../../crypto/uint256.h"
#include "../../common/transaction.h"
#include "../../walleve/walleve/stream/stream.h"

using namespace walleve;

void callback(Client *cl)
{
    // HandlePair sub_tx_hp;
    // std::string id_all_tx = cl->SendSub("all-tx", sub_tx_hp);

    //usleep(100 * 10);

    HandlePair sub_hp;
    std::string id_all_block = cl->SendSub("all-block", sub_hp);

    // cl->SendUnsub(id);

    //HandlePair tx_hp;
    // lws::GetTxArg tx_arg;
    // google::protobuf::Any *tx_any = new google::protobuf::Any();
    // tx_any->PackFrom(block_arg);
    // std::string mehtod_id1 = cl->SendMethod("gettransaction", tx_any, tx_hp);
}

void callback1(Client *cl)
{
    HandlePair block_hp;
    lws::GetBlocksArg block_arg;
    uint256 hash;
    hash.SetHex("a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0");
    block_arg.set_hash(std::string(hash.begin(), hash.end()));
    block_arg.set_number(5);
    google::protobuf::Any *block_any = new google::protobuf::Any();
    block_any->PackFrom(block_arg);
    std::string mehtod_id = cl->SendMethod("getblocks", block_any, block_hp);

    ///////////////////////////////////////////////////////////////////////

    /*  HandlePair sendtx_hp;
    lws::SendTxArg sendtx_arg;
  
    CTransaction tx;
    tx.SetNull();
    tx.nAmount = 120;
    tx.nVersion = 1;
    tx.nTxFee = 1;
    
    walleve::CWalleveBufStream ss;
    ss << tx;
    
    std::string data(ss.GetData(),ss.GetSize());
    sendtx_arg.set_data(data);
    google::protobuf::Any *tx_any = new google::protobuf::Any();
    tx_any->PackFrom(sendtx_arg);
    std::string mehtod_id = cl->SendMethod("sendtransaction", tx_any, sendtx_hp);*/
}

void run(std::string ip, int port)
{
    try
    {
        Client cl(ip, port, 1, "lws-test");
        cl.SetCallBackFn(callback);
        cl.SetMethodCallBackFn(callback1);
        cl.Run();
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void test_stream()
{
    DbpStream ss;
    std::cout << ss.get_size() << std::endl; // 0

    std::string str("sssss");
    ss.write(std::vector<unsigned char>(str.begin(), str.end()));
    std::cout << ss.get_size() << std::endl; // 5
    ss.clear();
    std::cout << ss.get_size() << std::endl; // 0

    std::vector<unsigned char> v(13000000, 255);
    ss.write(v);
    std::cout << ss.get_size() << std::endl; // 13000000
    ss.clear();
    std::cout << ss.get_size() << std::endl; // 0
    ss.write(v);
    std::cout << ss.get_size() << std::endl; // 13000000
    std::vector<unsigned char> rbuf;
    ss.read(rbuf, 3000000);
    std::cout << ss.get_size() << std::endl; //  10000000
    ss.write(v);
    std::cout << ss.get_size() << std::endl; //  23000000
    std::cout << rbuf.size() << std::endl;   //  3000000
    ss.read(rbuf, 2000);
    std::cout << rbuf.size() << std::endl;   //  2000
    std::cout << ss.get_size() << std::endl; //  22998000

    ss.read(rbuf, 22998000 + 10);

    std::cout << ss.get_size() << std::endl; //  22998000
}

void test_client(int argc, char *argv[])
{
    std::string ip("127.0.0.1");
    int port(6815);

    if (argc > 1)
    {
        ip = std::string(argv[1]);
    }

    if (argc > 2)
    {
        port = atoi(argv[2]);
    }

    std::cout << "[-]core wallet:" << ip << ":" << port << std::endl;

    run(ip, port);
}

enum State
{
    START,
    CONNECT,
    CONNECT_SESSION,
    RECONNECT_SESSION,
    PING,
    PONG,
    SUB,
    METHOD,
    RECV,
    ERROR,
    TERMINAL
};

State state;

boost::asio::io_service io_;
boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 6815);

boost::asio::ip::tcp::socket g_socket(io_);

void connect_func()
{
    try
    {
        g_socket.connect(ep);
    }
    catch (const std::exception &e)
    {
        std::cout << "connect socket failed: " << e.what() << std::endl;
        state = CONNECT;
        sleep(5);
        return;
    }

    state = CONNECT_SESSION;
}

static bool write_msg(dbp::Msg type, google::protobuf::Any *any)
{
    dbp::Base base;
    base.set_msg(type);
    base.set_allocated_object(any);

    std::string bytes;
    base.SerializeToString(&bytes);

    uint32_t len;
    char len_buffer[4];
    len = bytes.size();
    len = htonl(len);
    std::memcpy(&len_buffer[0], &len, 4);
    bytes.insert(0, len_buffer, 4);

    boost::system::error_code err;
    std::size_t size = boost::asio::write(g_socket, boost::asio::buffer(bytes), err);
    if (err)
    {
        state = CONNECT;
        g_socket.cancel();
        g_socket.close();
        std::cout
            << "wait to reconnect session" << std::endl;
        sleep(5);
        return false;
    }
    return true;
}

static bool read_header(std::size_t &len)
{
    char rlen_buffer[4];
    boost::system::error_code err;
    std::size_t recv_size = boost::asio::read(g_socket, boost::asio::buffer(rlen_buffer), err);
    if (err)
    {
        std::cout << "read header failed. " << std::endl;
        return false;
    }

    uint32_t rlen;
    std::memcpy(&rlen, rlen_buffer, 4);
    rlen = ntohl(rlen);
    if (rlen == 0)
    {
        std::cout << "len is 0" << std::endl;
        return false;
    }
    len = rlen;
    return true;
}

static bool read_payload(dbp::Base &base, std::size_t len)
{
    std::vector<unsigned char> payloadBuf(len, 0);
    boost::system::error_code err;
    std::size_t size = boost::asio::read(g_socket, boost::asio::buffer(payloadBuf), err);
    if (err)
    {
        std::cout << "read payload failed. " << std::endl;
        return false;
    }
    if (size != len)
    {
        std::cout << "recv size is not len" << std::endl;
        return false;
    }

    if (!base.ParseFromString(std::string(payloadBuf.begin(), payloadBuf.end())))
    {
        std::cout << "Msg Base parse error. " << std::endl;
        return false;
    }

    return true;
}

static bool read_msg(dbp::Base &base)
{
    std::size_t len;
    if (!read_header(len))
    {
        std::cout << "read header failed. " << std::endl;
        return false;
    }

    if (!read_payload(base, len))
    {
        std::cout << "read payload failed. " << std::endl;
        return false;
    }

    return true;
}

static std::string session_id("");

void connect_session()
{
    std::cout << "################# CONNECT SESSION ##################" << std::endl;

    dbp::Connect connect;
    connect.set_session(session_id);
    connect.set_version(1);
    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connect);

    if (!write_msg(dbp::CONNECT, any))
    {
        return;
    }

    std::cout << "[>]send connect session success. " << std::endl;

    dbp::Base base;
    if (!read_msg(base))
    {
        return;
    }

    if (base.msg() == dbp::Msg::CONNECTED)
    {
        dbp::Connected connected;
        base.object().UnpackTo(&connected);
        std::cout << "[<]connected session is: " << connected.session() << std::endl;
        session_id = connected.session();
        state = SUB;
        return;
    }

    if (base.msg() == dbp::Msg::FAILED)
    {
        dbp::Failed failed;
        base.object().UnpackTo(&failed);
        std::cout << "[<]connect session failed: " << failed.reason() << std::endl;

        if ("002" == failed.reason())
        {
            session_id = "";
            state = CONNECT_SESSION;
        }

        if ("001" == failed.reason())
        {

            std::cout << "support version: " << failed.version()[0] << std::endl;
            session_id = "";
            state = CONNECT_SESSION;
        }

        if ("003" == failed.reason())
        {
            session_id = "";
            state = CONNECT_SESSION;
        }
        return;
    }
}

void sub_func()
{
    std::cout << "################# SUB ##################" << std::endl;

    dbp::Sub sub;
    sub.set_name("all-block");
    std::string id(std::to_string(time(NULL)) + "blk");
    sub.set_id(id);
    google::protobuf::Any *any_blk = new google::protobuf::Any();
    any_blk->PackFrom(sub);

    if (!write_msg(dbp::Msg::SUB, any_blk))
    {
        return;
    }

    std::cout << "[>]sub all-block success. "
              << std::endl;

    sub.set_name("all-tx");
    id = std::to_string(time(NULL)) + "tx";
    sub.set_id(id);
    google::protobuf::Any *any_tx = new google::protobuf::Any();
    any_tx->PackFrom(sub);

    if (!write_msg(dbp::Msg::SUB, any_tx))
    {
        return;
    }

    std::cout << "[>]sub all-tx success. "
              << std::endl;

    state = METHOD;
}

void ping_func()
{
    std::cout << "################# PING ##################" << std::endl;

    dbp::Ping ping;
    std::string id(std::to_string(time(NULL)));
    ping.set_id(id);
    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(ping);

    if (!write_msg(dbp::Msg::PING, any))
    {
        return;
    }

    std::cout << "[>]PING success:   " << id
              << std::endl;

    state = RECV;
}

void method_func()
{

    std::cout << "################# METHOD ##################" << std::endl;

    lws::GetBlocksArg block_arg;
    uint256 hash;
    hash.SetHex("a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0");
    block_arg.set_hash(std::string(hash.begin(), hash.end()));
    block_arg.set_number(1000);
    google::protobuf::Any *block_any = new google::protobuf::Any();
    block_any->PackFrom(block_arg);

    dbp::Method method;
    method.set_method("getblocks");
    std::string id(std::to_string(time(NULL)));
    method.set_id(id);
    method.set_allocated_params(block_any);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(method);

    if (!write_msg(dbp::Msg::METHOD, any))
    {
        return;
    }

    std::cout << "[>]Method success:  " << id
              << std::endl;

    state = PING;
}

static std::string GetHex(std::string data)
{
    int n = 2 * data.length() + 1;
    std::string ret;
    const char c_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    ret.reserve(n);
    for (const unsigned char &c : data)
    {
        ret.push_back(c_map[c >> 4]);
        ret.push_back(c_map[c & 15]);
    }

    return ret;
}

static void print_block(lws::Block &block)
{
    std::string hash(block.hash());
    reverse(hash.begin(), hash.end());

    std::string prev_hash(block.hashprev());
    reverse(prev_hash.begin(), prev_hash.end());
    std::cout << "[<]recived block" << std::endl;
    std::cout << "   hash:" << GetHex(hash) << std::endl;
    std::cout << "   height:" << block.nheight() << std::endl;
    std::cout << "   prev hash:" << GetHex(prev_hash) << std::endl;
}

static void print_tx(lws::Transaction &tx)
{
    std::string hash(tx.hash());
    reverse(hash.begin(), hash.end());

    std::string sig(tx.vchsig());
    reverse(sig.begin(), sig.end());

    std::cout << "[<]recived transaction" << std::endl;
    std::cout << "   hash:" << GetHex(hash) << std::endl;
    std::cout << "   sig:" << GetHex(sig) << std::endl;
}

void recv_func()
{
    std::cout
        << "################# RECV" + std::to_string(time(NULL)) + "##################" << std::endl;

    dbp::Base base;
    if (!read_msg(base))
    {
        state = METHOD;
        return;
    }

    if (base.msg() == dbp::Msg::READY)
    {
        dbp::Ready ready;
        base.object().UnpackTo(&ready);
        std::cout << "[<]ready: " << ready.id() << std::endl;
        return;
    }

    if (base.msg() == dbp::Msg::NOSUB)
    {
        dbp::Nosub nosub;
        base.object().UnpackTo(&nosub);
        std::cout << "[<]nosub: " << nosub.id() << "errorcode: " << nosub.error() << std::endl;
        return;
    }

    if (base.msg() == dbp::Msg::PONG)
    {
        dbp::Pong pong;
        base.object().UnpackTo(&pong);
        std::cout << "[<]pong: " << pong.id() << std::endl;

        static int pong_count = 0;
        if (pong_count > 10)
        {
            state = METHOD;
            pong_count = 0;
        }
        pong_count++;

        return;
    }

    if (base.msg() == dbp::Msg::PING)
    {
        dbp::Ping ping;
        base.object().UnpackTo(&ping);
        std::cout << "[<]ping: " << ping.id() << std::endl;

        dbp::Pong pong;
        pong.set_id(ping.id());
        google::protobuf::Any *any = new google::protobuf::Any();
        any->PackFrom(pong);
        if (write_msg(dbp::Msg::PONG, any))
        {
            std::cout << "[>]pong: " << ping.id() << std::endl;
            state = PING;
        }

        return;
    }

    if (base.msg() == dbp::Msg::ADDED)
    {
        std::cout << "[<]added: " << std::endl;
        dbp::Added added;
        base.object().UnpackTo(&added);

        if (added.name() == "all-block")
        {
            lws::Block block;
            added.object().UnpackTo(&block);
            print_block(block);
        }

        if (added.name() == "all-tx")
        {
            lws::Transaction tx;
            added.object().UnpackTo(&tx);
            print_tx(tx);
        }

        return;
    }

    if (base.msg() == dbp::Msg::RESULT)
    {

        std::cout << "[<]result: " << std::endl;
        dbp::Result result;
        base.object().UnpackTo(&result);

        if (!result.error().empty())
        {
            std::cout << "[-]method error:" << result.error() << std::endl;
        }

        std::cout << "[<]: getblocks result: " << std::endl;

        int size = result.result_size();
        for (int i = 0; i < size; i++)
        {
            google::protobuf::Any any = result.result(i);
            lws::Block block;
            any.UnpackTo(&block);
            print_block(block);
        }
    }

    return;
}

void run_state_machine()
{
    switch (state)
    {
    case START:
        state = CONNECT;
        break;
    case CONNECT:
        connect_func();
        break;
    case CONNECT_SESSION:
        connect_session();
        break;
    case SUB:
        sub_func();
        break;
    case PING:
        ping_func();
        break;
    case METHOD:
        method_func();
        break;
    case RECV:
        recv_func();
        break;
    }
}

void test_client2()
{
    state = START;
    while (true)
    {
        run_state_machine();
    }
}

int main(int argc, char *argv[])
{
    //test_stream();
    // test_client(argc, argv);
    test_client2();
    return 0;
}