#include "handlepair.h"


HandlePair::HandlePair() : enable(false)
{
}

HandlePair::~HandlePair()
{
}

std::string HandlePair::GetHex(std::string data)
{
    int n = 2 * data.length() + 1;
    std::string ret;
    const char c_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    ret.reserve(n);
    for(const unsigned char& c : data)
    {
        ret.push_back(c_map[c >> 4]);
        ret.push_back(c_map[c & 15]);
    }

    return ret;
}

std::string HandlePair::SetHex(std::string data)
{
    int len = data.length();
    std::string ret;

    std::map<char, int> c_map = {
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9},
        {'a', 10},
        {'b', 11},
        {'c', 12},
        {'d', 13},
        {'e', 14},
        {'f', 15}
    };

    unsigned char temp;
    for(int i = 0; i < len; i++)
    {

        unsigned char c = data[i];
        unsigned char t = c_map[c];

        if(0 == i % 2)
        {
            temp = 0xff;
            t = t << 4;
            temp = temp & t;
        }

        if(1 == i % 2)
        {
            temp = t | temp;
            ret.push_back(temp);
        }
    }

    return ret;
}

void HandlePair::AddBlock(lws::Block &block)
{
    PrintBlock(block);
}

void HandlePair::PrintBlock(lws::Block &block)
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

void HandlePair::PrintTx(lws::Transaction &tx)
{
    std::string hash(tx.hash());
    reverse(hash.begin(), hash.end());

    std::string sig(tx.vchsig());
    reverse(sig.begin(), sig.end());

    std::cout << "[<]recived transaction" << std::endl;
    std::cout << "   hash:" << GetHex(hash) << std::endl;
    std::cout << "   sig:" << GetHex(sig) << std::endl;
}

void HandlePair::AddTx(lws::Transaction &tx)
{
    PrintTx(tx);
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

void HandlePair::MethodHandler(dbp::Result &result)
{
    if(!result.error().empty())
    {
        std::cout << "[-]method error:" << result.error() << std::endl;
    }

    if ("getblocks" == name)
    {
        
        std::cout << "[<]: getblocks result" << std::endl;
        
        int size = result.result_size();
        for(int i = 0; i < size; i++)
        {
            google::protobuf::Any any = result.result(i); 
            lws::Block block;
            any.UnpackTo(&block);
            PrintBlock(block);
        }
    }

    if("gettransaction" == name)
    {
        int size = result.result_size();
        for(int i = 0; i < size; i++)
        {
            google::protobuf::Any any = result.result(i); 
            lws::Transaction tx;
            any.UnpackTo(&tx);
            PrintTx(tx);
        }
    }

    if("sendtransaction" == name)
    {
        int size = result.result_size();
        for(int i = 0; i < size; i++)
        {
            google::protobuf::Any any = result.result(i); 
            lws::SendTxRet ret;
            any.UnpackTo(&ret);
            std::cout << "[<]sendtransaction txid: " << ret.hash() << std::endl;
            std::cout << "[<]sendtransaction result: " << ret.result() << std::endl;
            std::cout << "[<]sendtransaction reason: " << ret.reason() << std::endl;
        }
    }
}
