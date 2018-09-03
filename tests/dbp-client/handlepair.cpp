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

void HandlePair::AddTx()
{
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
        AddTx();
    }
}

void HandlePair::MethodHandler()
{
}
