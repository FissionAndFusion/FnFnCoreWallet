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

    HandlePair method_hp;
    std::string mehtod_id = cl->SendMethod("getblocks", method_hp);
    usleep(100 * 10);
    std::string mehtod_id1 = cl->SendMethod("gettransaction", method_hp);
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