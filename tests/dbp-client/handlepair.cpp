#include "handlepair.h"


HandlePair::HandlePair() : enable(false)
{
}

HandlePair::~HandlePair()
{
}

void HandlePair::AddBlock()
{
}

void HandlePair::AddTx()
{
}

void HandlePair::SubHandler(std::string type, std::string name, google::protobuf::Any object)
{
    if("all-block" == name)
    {
        AddBlock();
    }

    if("all-tx" == name)
    {
        AddTx();
    }
}

void HandlePair::MethodHandler()
{
}
