// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpclient.h"
#include "dbputils.h"
#include "walleve/netio/netio.h"


static int MSG_HEADER_LEN = 4;

#define DBPCLIENT_CONNECT_TIMEOUT 10 
namespace multiverse
{

CMvDbpClientSocket::CMvDbpClientSocket(const std::string& strIOModuleIn,const uint64 nNonceIn,
                   CMvDbpClient* pDbpClientIn,CIOClient* pClientIn)
: strIOModule(strIOModuleIn),
  nNonce(nNonceIn),
  pDbpClient(pDbpClientIn),
  pClient(pClientIn)
{

}
    
CMvDbpClientSocket::~CMvDbpClientSocket()
{

}

const std::string& CMvDbpClientSocket::GetIOModule()
{
    return strIOModule;
}
    
uint64 CMvDbpClientSocket::GetNonce()
{
    return nNonce;
}
    
CNetHost CMvDbpClientSocket::GetHost()
{
    return CNetHost(pClient->GetRemote());
}
    
void CMvDbpClientSocket::Activate()
{
    pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1));
}

void CMvDbpClientSocket::StartReadHeader()
{
    pClient->Read(ssRecv, MSG_HEADER_LEN,
                  boost::bind(&CMvDbpClientSocket::HandleReadHeader, this, _1));
}

void CMvDbpClientSocket::StartReadPayload(std::size_t nLength)
{
     pClient->Read(ssRecv, nLength,
                  boost::bind(&CMvDbpClientSocket::HandleReadPayload, this, _1, nLength));
}

void CMvDbpClientSocket::HandleWritenRequest(std::size_t nTransferred)
{
    if(ssSend.GetSize() != 0)
    {
        pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1));
        return;
    }

    if(nTransferred != 0)
    {
        StartReadHeader();
    }
    else
    {
        pDbpClient->HandleClientSocketError(this);
    }
}
    
void CMvDbpClientSocket::HandleReadHeader(std::size_t nTransferred)
{
    if (nTransferred == MSG_HEADER_LEN)
    {
        std::string lenBuffer(MSG_HEADER_LEN, 0);
        ssRecv.Read(&lenBuffer[0], MSG_HEADER_LEN);

        uint32_t nMsgHeaderLen = CDbpUtils::ParseLenFromMsgHeader(&lenBuffer[0], MSG_HEADER_LEN);
        if (nMsgHeaderLen == 0)
        {
            std::cerr << "Msg Base header length is 0 [dbpclient]" << std::endl;
            pDbpClient->HandleClientSocketError(this);
            return;
        }

        StartReadPayload(nMsgHeaderLen);
    }
    else
    {
        std::cerr << "Msg Base header length is not 4 [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
    }
}
    
void CMvDbpClientSocket::HandleReadPayload(std::size_t nTransferred,uint32_t len)
{
    if (nTransferred == len)
    {
        HandleReadCompleted(len);
    }
    else
    {
        std::cerr << "pay load is not len. [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
    }
}

void CMvDbpClientSocket::HandleReadCompleted(uint32_t len)
{

}


CMvDbpClient::CMvDbpClient()
  : walleve::CIOProc("dbpclient")
{
}

CMvDbpClient::~CMvDbpClient(){}


void CMvDbpClient::HandleClientSocketError(CMvDbpClientSocket* pClientSocket)
{

}

void CMvDbpClient::AddNewClient(const CDbpClientConfig& confClient)
{
    vecClientConfig.push_back(confClient);
}

bool CMvDbpClient::WalleveHandleInitialize()
{
    // init client config
    for(const auto & confClient : vecClientConfig)
    {
        if(!CreateProfile(confClient))
        {
            return false;
        }
    }

    return true;
}

void CMvDbpClient::WalleveHandleDeinitialize()
{
    // delete client config
}

void CMvDbpClient::EnterLoop()
{
    // start resource
    WalleveLog("Dbp Client starting:\n");
    
    for(std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        bool fEnableSSL = (*it).second.optSSL.fEnable;
        if(!StartConnection(it->first,DBPCLIENT_CONNECT_TIMEOUT,fEnableSSL,it->second.optSSL))
        {
            WalleveLog("Start to connect parent node %s failed,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
        }
        else
        {
            WalleveLog("Start to connect parent node %s success,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
        }  
    }
}

void CMvDbpClient::LeaveLoop()
{
    WalleveLog("Dbp Client stop\n");
    // destory resource
}

bool CMvDbpClient::ClientConnected(CIOClient* pClient)
{
    auto it = mapProfile.find(pClient->GetRemote());
    if(it == mapProfile.end())
    {
        return false;
    }

    WalleveLog("Connect parent node %s success,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
    
    std::cerr << "Connect parent node" << 
        (*it).first.address().to_string() << "success, " 
        << "port " << (*it).first.port() << std::endl;

    return true;
}

void CMvDbpClient::ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote)
{
    WalleveLog("Connect parent node %s failed,  port = %d\n reconnectting",
                       epRemote.address().to_string().c_str(),
                       epRemote.port());

    std::cerr << "Connect parent node" << 
        epRemote.address().to_string() << "failed, " 
        << "port " << epRemote.port() << std::endl;
}
    
void CMvDbpClient::Timeout(uint64 nNonce,uint32 nTimerId)
{
    std::cerr << "time out" << std::endl;
}

bool CMvDbpClient::CreateProfile(const CDbpClientConfig& confClient)
{
    CDbpClientProfile profile;
    if(!WalleveGetObject(confClient.strIOModule,profile.pIOModule))
    {
        WalleveLog("Failed to request %s\n", confClient.strIOModule.c_str());
        return false;
    }

    if(confClient.optSSL.fEnable)
        profile.optSSL = confClient.optSSL;
    
    profile.vSupportForks = confClient.vSupportForks;
    
    mapProfile[confClient.epParentHost] = profile;

    return true;
}

bool CMvDbpClient::StartConnection(const boost::asio::ip::tcp::endpoint& epRemote, int64 nTimeout,bool fEnableSSL,
    const CIOSSLOption& optSSL)
{
    if(fEnableSSL)
    {
        return Connect(epRemote,nTimeout) ? true : false;
    }
    else
    {
        return SSLConnect(epRemote,nTimeout,optSSL) ? true : false;
    }
}


} // namespace multiverse