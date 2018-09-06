#include <iostream>
#include <algorithm>
#include "dbp.pb.h"
#include "lws.pb.h"

#ifndef  HANDLEPAIR_H
#define  HANDLEPAIR_H
class HandlePair
{
    public:
      std::string name;
      std::string id;
      std::string type;
      bool enable;

      HandlePair();
      ~HandlePair();
      std::string GetHex(std::string data);
      void AddBlock(lws::Block &block);
      void AddTx(lws::Transaction &tx);
      void SubHandler(std::string type, std::string name, google::protobuf::Any object);
      void MethodHandler();

    private:
    protected:
};
#endif