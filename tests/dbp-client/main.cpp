#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

#include <arpa/inet.h>

#include "client.h"
#include "handlepair.h"
#include "dbp.pb.h"
#include "lws.pb.h"

void callback(Client *cl)
{
    HandlePair sub_hp;
    std::string id_all_block = cl->SendSub("all-block", sub_hp);
    usleep(100 * 10);
    std::string id_all_tx = cl->SendSub("all-tx", sub_hp);
    // cl->SendUnsub(id);

    HandlePair block_hp;
    lws::GetBlocksArg block_arg;
    block_arg.set_hash("5b1456237a0fe7a1129875b71bd038b8b0ff4d2e2c69f725dda4f501ec8ef9a3");
    block_arg.set_number(5);
    google::protobuf::Any *block_any = new google::protobuf::Any();
    block_any->PackFrom(block_arg);
    std::string mehtod_id = cl->SendMethod("getblocks", block_any, block_hp);
    usleep(100 * 10);

    //HandlePair tx_hp;
   // lws::GetTxArg tx_arg;
   // google::protobuf::Any *tx_any = new google::protobuf::Any();
   // tx_any->PackFrom(block_arg);
   // std::string mehtod_id1 = cl->SendMethod("gettransaction", tx_any, tx_hp);
}

void run(std::string ip, int port)
{
    try
    {
        Client cl(ip, port, 1, "lws-test");
        cl.SetCallBackFn(callback);
        cl.Run();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    std::string ip("127.0.0.1");
    int port(6815);

    if(argc > 1)
    {
        ip = std::string(argv[1]);
    }

    if(argc > 2)
    {
        port = atoi(argv[2]);
    }

    std::cout << "[-]core wallet:" << ip << ":" << port << std::endl;

    run(ip, port);
    return 0;
}