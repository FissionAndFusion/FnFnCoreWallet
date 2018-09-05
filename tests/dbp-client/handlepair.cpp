#include "handlepair.h"


HandlePair::HandlePair() : enable(false)
{
}

HandlePair::~HandlePair()
{
}

void HandlePair::AddBlock(lws::Block &block)
{
    std::cout << "recived block, nVersion:" << block.nversion() << "nType:" << block.ntype() << std::endl;
}

void HandlePair::AddTx(lws::Transaction &tx)
{
    std::cout << "recived transaction, nVersion:" << tx.nversion() << "nType:" << tx.ntype() << std::endl;
}

void HandlePair::SubHandler(std::string type, std::string name, google::protobuf::Any object)
{
    if("all-block" == name)
    {
        lws::Block block;
        object.UnpackTo(&block);
        AddBlock(block);
    }

    if("all-tx" == name)
    {
        lws::Transaction tx;
        object.UnpackTo(&tx);
        AddTx(tx);
    }
}

void HandlePair::MethodHandler()
{
}
