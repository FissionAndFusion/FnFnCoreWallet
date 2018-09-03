#include <iostream>
#include "dbp.pb.h"

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
      void AddBlock();
      void AddTx();
      void SubHandler(std::string type, std::string name, google::protobuf::Any object);
      void MethodHandler();

    private:
    protected:
};
#endif