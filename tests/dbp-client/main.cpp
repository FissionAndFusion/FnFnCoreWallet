#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>

#include "dbp.pb.h"
#include "lws.pb.h"
#include "../../crypto/uint256.h"
#include "../../common/transaction.h"
#include "../../walleve/walleve/stream/stream.h"

using namespace walleve;

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

typedef struct ThreadCxt
{

    ThreadCxt() : ep(boost::asio::ip::address::from_string("127.0.0.1"), 6815),
                  g_socket(io_), session_id("")
    {
    }

    State state;

    boost::asio::io_service io_;
    boost::asio::ip::tcp::endpoint ep;

    boost::asio::ip::tcp::socket g_socket;
    std::string session_id;
} ThreadCxt;

static std::string formatString(const char *name)
{
    char buffer[1024] = {0};

    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    unsigned long long tid = std::stoull(stid);

    sprintf(buffer, "######### [%s] [thread: %lld] [timestamp: %ld] ############",
            name,
            tid,
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return std::string(buffer, 1024);
}

void connect_func(ThreadCxt &cxt)
{
    try
    {
        cxt.g_socket.connect(cxt.ep);
    }
    catch (const std::exception &e)
    {
        std::cout << "connect socket failed: " << e.what() << std::endl;
        cxt.state = CONNECT;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return;
    }

    cxt.state = CONNECT_SESSION;
}

static bool write_msg(ThreadCxt &cxt, dbp::Msg type, google::protobuf::Any *any)
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
    std::size_t size = boost::asio::write(cxt.g_socket, boost::asio::buffer(bytes), err);
    if (err)
    {
        cxt.state = CONNECT;
        cxt.g_socket.cancel();
        cxt.g_socket.close();
        std::cout
            << "wait to reconnect session" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return false;
    }
    return true;
}

static bool read_header(ThreadCxt &cxt, std::size_t &len)
{
    char rlen_buffer[4];
    boost::system::error_code err;
    std::size_t recv_size = boost::asio::read(cxt.g_socket, boost::asio::buffer(rlen_buffer), err);
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

static bool read_payload(ThreadCxt &cxt, dbp::Base &base, std::size_t len)
{
    std::vector<unsigned char> payloadBuf(len, 0);
    boost::system::error_code err;
    std::size_t size = boost::asio::read(cxt.g_socket, boost::asio::buffer(payloadBuf), err);
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
        exit(-1);
        return false;
    }

    return true;
}

static bool read_msg(ThreadCxt &cxt, dbp::Base &base)
{
    std::size_t len;
    if (!read_header(cxt, len))
    {
        std::cout << "read header failed. " << std::endl;
        return false;
    }

    if (!read_payload(cxt, base, len))
    {
        std::cout << "read payload failed. " << std::endl;
        return false;
    }

    return true;
}

void connect_session(ThreadCxt &cxt)
{
    std::cout << formatString("CONNECT") << std::endl;

    dbp::Connect connect;
    connect.set_session(cxt.session_id);
    connect.set_version(1);

    google::protobuf::Any anyFork;
    lws::ForkID forkidMsg;

    // f9b06c744e1629f5482759f3fbf92c333b72e58bb06a6097cef115bfcd0e98aa
    // f77d0b574fe7c567db23d2ad2e7174181db6d7b227a7c8c26abc48d6a9fc9030
    // 0bd9068dc9d2fee08866bace68b072a9f476acfb0173842843c22017ad60e074
   
   // forkidMsg.add_ids(std::string("c8f10736fb9b03a2d224c9d79b60ccc156b4bf9c28072fb332d0ea5fc104e085"));
   // forkidMsg.add_ids(std::string("f9b06c744e1629f5482759f3fbf92c333b72e58bb06a6097cef115bfcd0e98aa"));
   // forkidMsg.add_ids(std::string("f77d0b574fe7c567db23d2ad2e7174181db6d7b227a7c8c26abc48d6a9fc9030"));
    forkidMsg.add_ids(std::string("0bd9068dc9d2fee08866bace68b072a9f476acfb0173842843c22017ad60e074"));
    anyFork.PackFrom(forkidMsg);
    (*connect.mutable_udata())["forkid"] = anyFork;

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connect);

    if (!write_msg(cxt, dbp::CONNECT, any))
    {
        return;
    }

    std::cout << "[>]send connect session success. " << std::endl;

    dbp::Base base;
    if (!read_msg(cxt, base))
    {
        return;
    }

    if (base.msg() == dbp::Msg::CONNECTED)
    {
        dbp::Connected connected;
        base.object().UnpackTo(&connected);
        std::cout << "[<]connected session is: " << connected.session() << std::endl;
        cxt.session_id = connected.session();
        cxt.state = SUB;
        return;
    }

    if (base.msg() == dbp::Msg::FAILED)
    {
        dbp::Failed failed;
        base.object().UnpackTo(&failed);
        std::cout << "[<]connect session failed: " << failed.reason() << std::endl;

        if ("002" == failed.reason())
        {
            cxt.session_id = "";
            cxt.state = CONNECT_SESSION;
        }

        if ("001" == failed.reason())
        {

            std::cout << "support version: " << failed.version()[0] << std::endl;
            cxt.session_id = "";
            cxt.state = CONNECT_SESSION;
        }

        if ("003" == failed.reason())
        {
            cxt.session_id = "";
            cxt.state = CONNECT_SESSION;
        }
        return;
    }
}

void sub_func(ThreadCxt &cxt)
{
    std::cout << formatString("SUB") << std::endl;

    dbp::Sub sub;
    sub.set_name("all-block");
    std::string id(std::to_string(time(NULL)) + "blk");
    sub.set_id(id);
    google::protobuf::Any *any_blk = new google::protobuf::Any();
    any_blk->PackFrom(sub);

    if (!write_msg(cxt, dbp::Msg::SUB, any_blk))
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

    if (!write_msg(cxt, dbp::Msg::SUB, any_tx))
    {
        return;
    }

    std::cout << "[>]sub all-tx success. "
              << std::endl;

    cxt.state = METHOD;
}

void ping_func(ThreadCxt &cxt)
{
    std::cout << formatString("PING") << std::endl;

    dbp::Ping ping;
    std::string id(std::to_string(time(NULL)));
    ping.set_id(id);
    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(ping);

    if (!write_msg(cxt, dbp::Msg::PING, any))
    {
        return;
    }

    std::cout << "[>]PING success:   " << id
              << std::endl;

    cxt.state = RECV;
}

void method_func(ThreadCxt &cxt)
{
    std::cout << formatString("METHOD") << std::endl;

    lws::GetBlocksArg block_arg;
    uint256 hash;
    hash.SetHex("93cb5e2c6e73d52cf755cf4fd050682d5744980e7a52de8148ff01860012d7cc");
    block_arg.set_hash(std::string(hash.begin(), hash.end()));
    block_arg.set_number(77);
    google::protobuf::Any *block_any = new google::protobuf::Any();
    block_any->PackFrom(block_arg);

    dbp::Method method;
    method.set_method("getblocks");
    std::string id(std::to_string(time(NULL)));
    method.set_id(id);
    method.set_allocated_params(block_any);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(method);

    if (!write_msg(cxt, dbp::Msg::METHOD, any))
    {
        return;
    }

    std::cout << "[>]Method success:  " << id
              << std::endl;

    cxt.state = PING;
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

    /*std::cout << "vtx size: " << block.vtx_size() << std::endl;
    std::cout << "vtx v input size: " << block.vtx(0).vinput_size() << std::endl;
    for (int i = 0; i < block.vtx(0).vinput_size(); ++i)
    {
        std::string txhash(block.vtx(0).vinput(i).hash());
        reverse(txhash.begin(), txhash.end());
        std::cout << "InputTxHash: " << GetHex(txhash) << std::endl;
        std::cout << "InputTx n: " << block.vtx(0).vinput(i).n() << std::endl;
    }*/
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

void recv_func(ThreadCxt &cxt)
{
    std::cout << formatString("RECV") << std::endl;

    dbp::Base base;
    if (!read_msg(cxt, base))
    {
        cxt.state = METHOD;
        return;
    }

    if (base.msg() == dbp::Msg::READY)
    {
        dbp::Ready ready;
        base.object().UnpackTo(&ready);
        std::cout << "[<]ready: " << ready.id() << std::endl;
        return;
    }

    if (base.msg() == dbp::Msg::ERROR)
    {
        dbp::Error error;
        base.object().UnpackTo(&error);
        std::cout << "[<]error: " << error.reason() << "  " << error.explain() << std::endl;
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
            cxt.state = METHOD;
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
        if (write_msg(cxt, dbp::Msg::PONG, any))
        {
            std::cout << "[>]pong: " << ping.id() << std::endl;
            cxt.state = PING;
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

void run_state_machine(ThreadCxt &cxt)
{
    switch (cxt.state)
    {
    case START:
        cxt.state = CONNECT;
        break;
    case CONNECT:
        connect_func(cxt);
        break;
    case CONNECT_SESSION:
        connect_session(cxt);
        break;
    case SUB:
        sub_func(cxt);
        break;
    case PING:
        ping_func(cxt);
        break;
    case METHOD:
        method_func(cxt);
        break;
    case RECV:
        recv_func(cxt);
        break;
    }
}

void test_client(ThreadCxt &cxt)
{
    cxt.state = START;
    while (true)
    {
        run_state_machine(cxt);
    }
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cout << "usage: dbpclient <test_thread_num>" << std::endl;
        exit(-1);
    }

    const int num = std::atoi(argv[1]);

    std::cout << "starting " << num << " threads to test."
              << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::thread threads[num];
    ThreadCxt cxts[num];

    for (int i = 0; i < num; ++i)
    {
        threads[i] = std::move(std::thread(test_client, std::ref(cxts[i])));
    }

    for (int i = 0; i < num; ++i)
    {
        threads[i].join();
    }

    return 0;
}