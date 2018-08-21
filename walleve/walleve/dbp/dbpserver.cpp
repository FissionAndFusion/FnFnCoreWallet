#include "dbpserver.h"

#include <openssl/rand.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace walleve;

 CDbpClient::CDbpClient(CDbpServer *pServerIn,CDbpProfile *pProfileIn,
                CIOClient* pClientIn)
{

}

CDbpClient::~CDbpClient()
{

}

CDbpProfile *GetProfile()
{

}

bool CDbpClient::IsEventStream()
{

}

void CDbpClient::SetEventStream()
{

}

void CDbpClient::Activate()
{

}

void CDbpClient::SendResponse(std::string& strResponse)
{

}

void CDbpClient::StartReadHeader()
{

}

void CDbpClient::StartReadPayload(std::size_t nLength)
{

}

void CDbpClient::HandleReadHeader(std::size_t nTransferred)
{

}

void CDbpClient::HandleReadPayload(std::size_t nTransferred)
{

}

void CDbpClient::HandleReadCompleted()
{

}

void CDbpClient::HandleWritenResponse(std::size_t nTransferred)
{

}

CDbpServer::CDbpServer()
: CIOProc("dbpserver")
{

}

CDbpServer::~CDbpServer()
{

}

CIOClient* CDbpServer::CreateIOClient(CIOContainer *pContainer)
{

}

void CDbpServer::HandleClientRecv(CDbpClient *pDbpClient,
                          CWalleveBufStream& ssPayload)
{

}

void CDbpServer::HandleClientSent(CDbpClient *pDbpClient)
{

}

void CDbpServer::HandleClientError(CDbpClient *pDbpClient)
{

}

void CDbpServer::AddNewHost(const CDbpHostConfig& confHost)
{

}

bool CDbpServer::WalleveHandleInitialize()
{

}

void CDbpServer::WalleveHandleDeinitialize()
{

}

void CDbpServer::EnterLoop()
{

}

void CDbpServer::LeaveLoop()
{

}

bool CDbpServer::ClientAccepted(const boost::asio::ip::tcp::endpoint& epService,CIOClient *pClient)
{

}

bool CDbpServer::CreateProfile(const CDbpHostConfig& confHost)
{

}

CDbpClient* CDbpServer::AddNewClient(CIOClient *pClient,CDbpProfile *pDbpProfile)
{

}

void CDbpServer::RemoveClient(CDbpClient *pDbpClient)
{

}

void CDbpServer::RespondError(CDbpClient *pDbpClient,int nStatusCode,const std::string& strError)
{

}

bool CDbpServer::HandleEvent(CWalleveEventDbpRespond& eventRsp)
{

}