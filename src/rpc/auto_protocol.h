// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

# ifndef MULTIVERSE_RPC_AUTO_PROTOCOL_H
# define MULTIVERSE_RPC_AUTO_PROTOCOL_H

# include <cfloat>
# include <climits>

# include "json/json_spirit_utils.h"
# include "walleve/type.h"

# include "mode/basic_config.h"
# include "rpc/rpc_type.h"
# include "rpc/rpc_req.h"
# include "rpc/rpc_resp.h"

namespace multiverse
{
namespace rpc
{

# ifdef __required__
# pragma push_macro("__required__")
# define PUSHED_MACRO_REQUIRED
# undef __required__
# endif
# define __required__

# ifdef __optional__
# pragma push_macro("__optional__")
# define PUSHED_MACRO_OPTIONAL
# undef __optional__
# endif
# define __optional__

class CTemplatePubKey;
class CTemplatePubKeyWeight;
class CTemplateRequest;
class CTemplateResponse;
class CTransactionData;
class CWalletTxData;
class CBlockData;

// class CTemplatePubKey
class CTemplatePubKey
{
public:
	__required__ CRPCString strKey;
	__required__ CRPCString strAddress;
public:
	CTemplatePubKey();
	CTemplatePubKey(const CRPCString& strKey, const CRPCString& strAddress);
	CTemplatePubKey(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CTemplatePubKey& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

// class CTemplatePubKeyWeight
class CTemplatePubKeyWeight
{
public:
	__required__ CRPCString strKey;
	__required__ CRPCInt64 nWeight;
public:
	CTemplatePubKeyWeight();
	CTemplatePubKeyWeight(const CRPCString& strKey, const CRPCInt64& nWeight);
	CTemplatePubKeyWeight(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CTemplatePubKeyWeight& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

// class CTemplateRequest
class CTemplateRequest
{
public:
	// class CMultisig
	class CMultisig
	{
	public:
		__required__ CRPCInt64 nRequired;
		__required__ CRPCVector<std::string> vecPubkeys = RPCValid;
	public:
		CMultisig();
		CMultisig(const CRPCInt64& nRequired, const CRPCVector<std::string>& vecPubkeys);
		CMultisig(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CMultisig& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CFork
	class CFork
	{
	public:
		__required__ CRPCString strRedeem;
		__required__ CRPCString strFork;
	public:
		CFork();
		CFork(const CRPCString& strRedeem, const CRPCString& strFork);
		CFork(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CFork& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CDelegate
	class CDelegate
	{
	public:
		__required__ CRPCString strDelegate;
		__required__ CRPCString strOwner;
	public:
		CDelegate();
		CDelegate(const CRPCString& strDelegate, const CRPCString& strOwner);
		CDelegate(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CDelegate& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CMint
	class CMint
	{
	public:
		__required__ CRPCString strMint;
		__required__ CRPCString strSpent;
	public:
		CMint();
		CMint(const CRPCString& strMint, const CRPCString& strSpent);
		CMint(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CMint& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CWeighted
	class CWeighted
	{
	public:
		__required__ CRPCInt64 nRequired;
		__required__ CRPCVector<CTemplatePubKeyWeight> vecPubkeys = RPCValid;
	public:
		CWeighted();
		CWeighted(const CRPCInt64& nRequired, const CRPCVector<CTemplatePubKeyWeight>& vecPubkeys);
		CWeighted(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CWeighted& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCString strType;
	__required__ CDelegate delegate;
	__required__ CFork fork;
	__required__ CMint mint;
	__required__ CMultisig multisig;
	__required__ CWeighted weighted;
public:
	CTemplateRequest();
	CTemplateRequest(const CRPCString& strType, const CDelegate& delegate, const CFork& fork, const CMint& mint, const CMultisig& multisig, const CWeighted& weighted);
	CTemplateRequest(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CTemplateRequest& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

// class CTemplateResponse
class CTemplateResponse
{
public:
	// class CFork
	class CFork
	{
	public:
		__required__ CRPCString strRedeem;
		__required__ CRPCString strFork;
	public:
		CFork();
		CFork(const CRPCString& strRedeem, const CRPCString& strFork);
		CFork(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CFork& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CWeighted
	class CWeighted
	{
	public:
		__required__ CRPCInt64 nSigsrequired;
		__required__ CRPCVector<CTemplatePubKeyWeight> vecAddresses = RPCValid;
	public:
		CWeighted();
		CWeighted(const CRPCInt64& nSigsrequired, const CRPCVector<CTemplatePubKeyWeight>& vecAddresses);
		CWeighted(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CWeighted& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CDelegate
	class CDelegate
	{
	public:
		__required__ CRPCString strDelegate;
		__required__ CRPCString strOwner;
	public:
		CDelegate();
		CDelegate(const CRPCString& strDelegate, const CRPCString& strOwner);
		CDelegate(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CDelegate& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CMint
	class CMint
	{
	public:
		__required__ CRPCString strMint;
		__required__ CRPCString strSpent;
	public:
		CMint();
		CMint(const CRPCString& strMint, const CRPCString& strSpent);
		CMint(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CMint& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CMultisig
	class CMultisig
	{
	public:
		__required__ CRPCInt64 nSigsrequired;
		__required__ CRPCVector<std::string> vecAddresses = RPCValid;
	public:
		CMultisig();
		CMultisig(const CRPCInt64& nSigsrequired, const CRPCVector<std::string>& vecAddresses);
		CMultisig(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CMultisig& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCString strType;
	__required__ CRPCString strHex;
	__required__ CDelegate delegate;
	__required__ CFork fork;
	__required__ CMint mint;
	__required__ CMultisig multisig;
	__required__ CWeighted weighted;
public:
	CTemplateResponse();
	CTemplateResponse(const CRPCString& strType, const CRPCString& strHex, const CDelegate& delegate, const CFork& fork, const CMint& mint, const CMultisig& multisig, const CWeighted& weighted);
	CTemplateResponse(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CTemplateResponse& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

// class CTransactionData
class CTransactionData
{
public:
	// class CVin
	class CVin
	{
	public:
		__required__ CRPCString strTxid;
		__required__ CRPCUint64 nVout;
	public:
		CVin();
		CVin(const CRPCString& strTxid, const CRPCUint64& nVout);
		CVin(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CVin& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCString strTxid;
	__required__ CRPCUint64 nVersion;
	__required__ CRPCString strType;
	__required__ CRPCUint64 nLockuntil;
	__required__ CRPCString strAnchor;
	__required__ CRPCVector<CVin> vecVin = RPCValid;
	__required__ CRPCString strSendto;
	__required__ CRPCDouble fAmount;
	__required__ CRPCDouble fTxfee;
	__required__ CRPCString strData;
	__required__ CRPCString strSig;
	__required__ CRPCString strFork;
	__optional__ CRPCInt64 nConfirmations;
public:
	CTransactionData();
	CTransactionData(const CRPCString& strTxid, const CRPCUint64& nVersion, const CRPCString& strType, const CRPCUint64& nLockuntil, const CRPCString& strAnchor, const CRPCVector<CVin>& vecVin, const CRPCString& strSendto, const CRPCDouble& fAmount, const CRPCDouble& fTxfee, const CRPCString& strData, const CRPCString& strSig, const CRPCString& strFork, const CRPCInt64& nConfirmations);
	CTransactionData(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CTransactionData& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

// class CWalletTxData
class CWalletTxData
{
public:
	__required__ CRPCString strTxid;
	__required__ CRPCString strFork;
	__required__ CRPCString strType;
	__required__ CRPCBool fSend;
	__required__ CRPCString strTo;
	__required__ CRPCDouble fAmount;
	__required__ CRPCDouble fFee;
	__required__ CRPCUint64 nLockuntil;
	__optional__ CRPCInt64 nBlockheight;
	__optional__ CRPCString strFrom;
public:
	CWalletTxData();
	CWalletTxData(const CRPCString& strTxid, const CRPCString& strFork, const CRPCString& strType, const CRPCBool& fSend, const CRPCString& strTo, const CRPCDouble& fAmount, const CRPCDouble& fFee, const CRPCUint64& nLockuntil, const CRPCInt64& nBlockheight, const CRPCString& strFrom);
	CWalletTxData(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CWalletTxData& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

// class CBlockData
class CBlockData
{
public:
	__required__ CRPCString strHash;
	__required__ CRPCUint64 nVersion;
	__required__ CRPCString strType;
	__required__ CRPCUint64 nTime;
	__required__ CRPCString strFork;
	__required__ CRPCUint64 nHeight;
	__required__ CRPCString strTxmint;
	__required__ CRPCVector<std::string> vecTx = RPCValid;
	__optional__ CRPCString strPrev;
public:
	CBlockData();
	CBlockData(const CRPCString& strHash, const CRPCUint64& nVersion, const CRPCString& strType, const CRPCUint64& nTime, const CRPCString& strFork, const CRPCUint64& nHeight, const CRPCString& strTxmint, const CRPCVector<std::string>& vecTx, const CRPCString& strPrev);
	CBlockData(const CRPCType& type);
	json_spirit::Value ToJSON() const;
	CBlockData& FromJSON(const json_spirit::Value& v);
	bool IsValid() const;
};

/////////////////////////////////////////////////////
// help

// CHelpParam
class CHelpParam : public CRPCParam
{
public:
	__optional__ CRPCString strCommand;
public:
	CHelpParam();
	CHelpParam(const CRPCString& strCommand);
	virtual json_spirit::Value ToJSON() const;
	virtual CHelpParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CHelpParam> MakeCHelpParamPtr(Args&&... args)
{
	return std::make_shared<CHelpParam>(std::forward<Args>(args)...);
}

// CHelpResult
class CHelpResult : public CRPCResult
{
public:
	__required__ CRPCString strHelp;
public:
	CHelpResult();
	CHelpResult(const CRPCString& strHelp);
	virtual json_spirit::Value ToJSON() const;
	virtual CHelpResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CHelpResult> MakeCHelpResultPtr(Args&&... args)
{
	return std::make_shared<CHelpResult>(std::forward<Args>(args)...);
}

// CHelpConfig
class CHelpConfig : virtual public CMvBasicConfig, public CHelpParam
{
public:
	CHelpConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// stop

// CStopParam
class CStopParam : public CRPCParam
{
public:
	CStopParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CStopParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CStopParam> MakeCStopParamPtr(Args&&... args)
{
	return std::make_shared<CStopParam>(std::forward<Args>(args)...);
}

// CStopResult
class CStopResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CStopResult();
	CStopResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CStopResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CStopResult> MakeCStopResultPtr(Args&&... args)
{
	return std::make_shared<CStopResult>(std::forward<Args>(args)...);
}

// CStopConfig
class CStopConfig : virtual public CMvBasicConfig, public CStopParam
{
public:
	CStopConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getpeercount

// CGetPeerCountParam
class CGetPeerCountParam : public CRPCParam
{
public:
	CGetPeerCountParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CGetPeerCountParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetPeerCountParam> MakeCGetPeerCountParamPtr(Args&&... args)
{
	return std::make_shared<CGetPeerCountParam>(std::forward<Args>(args)...);
}

// CGetPeerCountResult
class CGetPeerCountResult : public CRPCResult
{
public:
	__required__ CRPCInt64 nCount;
public:
	CGetPeerCountResult();
	CGetPeerCountResult(const CRPCInt64& nCount);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetPeerCountResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetPeerCountResult> MakeCGetPeerCountResultPtr(Args&&... args)
{
	return std::make_shared<CGetPeerCountResult>(std::forward<Args>(args)...);
}

// CGetPeerCountConfig
class CGetPeerCountConfig : virtual public CMvBasicConfig, public CGetPeerCountParam
{
public:
	CGetPeerCountConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// listpeer

// CListPeerParam
class CListPeerParam : public CRPCParam
{
public:
	CListPeerParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CListPeerParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListPeerParam> MakeCListPeerParamPtr(Args&&... args)
{
	return std::make_shared<CListPeerParam>(std::forward<Args>(args)...);
}

// CListPeerResult
class CListPeerResult : public CRPCResult
{
public:
	// class CPeer
	class CPeer
	{
	public:
		__required__ CRPCString strAddress;
		__required__ CRPCString strServices;
		__required__ CRPCInt64 nLastsend;
		__required__ CRPCInt64 nLastrecv;
		__required__ CRPCInt64 nConntime;
		__required__ CRPCString strVersion;
		__required__ CRPCString strSubver;
		__required__ CRPCBool fInbound;
		__required__ CRPCInt64 nHeight;
		__required__ CRPCBool fBanscore;
	public:
		CPeer();
		CPeer(const CRPCString& strAddress, const CRPCString& strServices, const CRPCInt64& nLastsend, const CRPCInt64& nLastrecv, const CRPCInt64& nConntime, const CRPCString& strVersion, const CRPCString& strSubver, const CRPCBool& fInbound, const CRPCInt64& nHeight, const CRPCBool& fBanscore);
		CPeer(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CPeer& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCVector<CPeer> vecPeer = RPCValid;
public:
	CListPeerResult();
	CListPeerResult(const CRPCVector<CPeer>& vecPeer);
	virtual json_spirit::Value ToJSON() const;
	virtual CListPeerResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListPeerResult> MakeCListPeerResultPtr(Args&&... args)
{
	return std::make_shared<CListPeerResult>(std::forward<Args>(args)...);
}

// CListPeerConfig
class CListPeerConfig : virtual public CMvBasicConfig, public CListPeerParam
{
public:
	CListPeerConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// addnode

// CAddNodeParam
class CAddNodeParam : public CRPCParam
{
public:
	__required__ CRPCString strNode;
public:
	CAddNodeParam();
	CAddNodeParam(const CRPCString& strNode);
	virtual json_spirit::Value ToJSON() const;
	virtual CAddNodeParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CAddNodeParam> MakeCAddNodeParamPtr(Args&&... args)
{
	return std::make_shared<CAddNodeParam>(std::forward<Args>(args)...);
}

// CAddNodeResult
class CAddNodeResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CAddNodeResult();
	CAddNodeResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CAddNodeResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CAddNodeResult> MakeCAddNodeResultPtr(Args&&... args)
{
	return std::make_shared<CAddNodeResult>(std::forward<Args>(args)...);
}

// CAddNodeConfig
class CAddNodeConfig : virtual public CMvBasicConfig, public CAddNodeParam
{
public:
	CAddNodeConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// removenode

// CRemoveNodeParam
class CRemoveNodeParam : public CRPCParam
{
public:
	__required__ CRPCString strNode;
public:
	CRemoveNodeParam();
	CRemoveNodeParam(const CRPCString& strNode);
	virtual json_spirit::Value ToJSON() const;
	virtual CRemoveNodeParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CRemoveNodeParam> MakeCRemoveNodeParamPtr(Args&&... args)
{
	return std::make_shared<CRemoveNodeParam>(std::forward<Args>(args)...);
}

// CRemoveNodeResult
class CRemoveNodeResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CRemoveNodeResult();
	CRemoveNodeResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CRemoveNodeResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CRemoveNodeResult> MakeCRemoveNodeResultPtr(Args&&... args)
{
	return std::make_shared<CRemoveNodeResult>(std::forward<Args>(args)...);
}

// CRemoveNodeConfig
class CRemoveNodeConfig : virtual public CMvBasicConfig, public CRemoveNodeParam
{
public:
	CRemoveNodeConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getforkcount

// CGetForkCountParam
class CGetForkCountParam : public CRPCParam
{
public:
	CGetForkCountParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CGetForkCountParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetForkCountParam> MakeCGetForkCountParamPtr(Args&&... args)
{
	return std::make_shared<CGetForkCountParam>(std::forward<Args>(args)...);
}

// CGetForkCountResult
class CGetForkCountResult : public CRPCResult
{
public:
	__required__ CRPCInt64 nCount;
public:
	CGetForkCountResult();
	CGetForkCountResult(const CRPCInt64& nCount);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetForkCountResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetForkCountResult> MakeCGetForkCountResultPtr(Args&&... args)
{
	return std::make_shared<CGetForkCountResult>(std::forward<Args>(args)...);
}

// CGetForkCountConfig
class CGetForkCountConfig : virtual public CMvBasicConfig, public CGetForkCountParam
{
public:
	CGetForkCountConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// listfork

// CListForkParam
class CListForkParam : public CRPCParam
{
public:
	CListForkParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CListForkParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListForkParam> MakeCListForkParamPtr(Args&&... args)
{
	return std::make_shared<CListForkParam>(std::forward<Args>(args)...);
}

// CListForkResult
class CListForkResult : public CRPCResult
{
public:
	// class CProfile
	class CProfile
	{
	public:
		__required__ CRPCString strFork;
		__required__ CRPCString strName;
		__required__ CRPCString strSymbol;
		__required__ CRPCBool fIsolated;
		__required__ CRPCBool fPrivate;
		__required__ CRPCBool fEnclosed;
		__required__ CRPCString strOwner;
	public:
		CProfile();
		CProfile(const CRPCString& strFork, const CRPCString& strName, const CRPCString& strSymbol, const CRPCBool& fIsolated, const CRPCBool& fPrivate, const CRPCBool& fEnclosed, const CRPCString& strOwner);
		CProfile(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CProfile& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCVector<CProfile> vecProfile = RPCValid;
public:
	CListForkResult();
	CListForkResult(const CRPCVector<CProfile>& vecProfile);
	virtual json_spirit::Value ToJSON() const;
	virtual CListForkResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListForkResult> MakeCListForkResultPtr(Args&&... args)
{
	return std::make_shared<CListForkResult>(std::forward<Args>(args)...);
}

// CListForkConfig
class CListForkConfig : virtual public CMvBasicConfig, public CListForkParam
{
public:
	CListForkConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getgenealogy

// CGetGenealogyParam
class CGetGenealogyParam : public CRPCParam
{
public:
	__optional__ CRPCString strFork;
public:
	CGetGenealogyParam();
	CGetGenealogyParam(const CRPCString& strFork);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetGenealogyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetGenealogyParam> MakeCGetGenealogyParamPtr(Args&&... args)
{
	return std::make_shared<CGetGenealogyParam>(std::forward<Args>(args)...);
}

// CGetGenealogyResult
class CGetGenealogyResult : public CRPCResult
{
public:
	// class CAncestry
	class CAncestry
	{
	public:
		__required__ CRPCString strParent;
		__required__ CRPCInt64 nHeight;
	public:
		CAncestry();
		CAncestry(const CRPCString& strParent, const CRPCInt64& nHeight);
		CAncestry(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CAncestry& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
	// class CSubline
	class CSubline
	{
	public:
		__required__ CRPCString strSub;
		__required__ CRPCInt64 nHeight;
	public:
		CSubline();
		CSubline(const CRPCString& strSub, const CRPCInt64& nHeight);
		CSubline(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CSubline& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCVector<CAncestry> vecAncestry = RPCValid;
	__required__ CRPCVector<CSubline> vecSubline = RPCValid;
public:
	CGetGenealogyResult();
	CGetGenealogyResult(const CRPCVector<CAncestry>& vecAncestry, const CRPCVector<CSubline>& vecSubline);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetGenealogyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetGenealogyResult> MakeCGetGenealogyResultPtr(Args&&... args)
{
	return std::make_shared<CGetGenealogyResult>(std::forward<Args>(args)...);
}

// CGetGenealogyConfig
class CGetGenealogyConfig : virtual public CMvBasicConfig, public CGetGenealogyParam
{
public:
	CGetGenealogyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getblocklocation

// CGetBlockLocationParam
class CGetBlockLocationParam : public CRPCParam
{
public:
	__required__ CRPCString strBlock;
public:
	CGetBlockLocationParam();
	CGetBlockLocationParam(const CRPCString& strBlock);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockLocationParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockLocationParam> MakeCGetBlockLocationParamPtr(Args&&... args)
{
	return std::make_shared<CGetBlockLocationParam>(std::forward<Args>(args)...);
}

// CGetBlockLocationResult
class CGetBlockLocationResult : public CRPCResult
{
public:
	__required__ CRPCString strFork;
	__required__ CRPCInt64 nHeight;
public:
	CGetBlockLocationResult();
	CGetBlockLocationResult(const CRPCString& strFork, const CRPCInt64& nHeight);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockLocationResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockLocationResult> MakeCGetBlockLocationResultPtr(Args&&... args)
{
	return std::make_shared<CGetBlockLocationResult>(std::forward<Args>(args)...);
}

// CGetBlockLocationConfig
class CGetBlockLocationConfig : virtual public CMvBasicConfig, public CGetBlockLocationParam
{
public:
	CGetBlockLocationConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getblockcount

// CGetBlockCountParam
class CGetBlockCountParam : public CRPCParam
{
public:
	__optional__ CRPCString strFork;
public:
	CGetBlockCountParam();
	CGetBlockCountParam(const CRPCString& strFork);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockCountParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockCountParam> MakeCGetBlockCountParamPtr(Args&&... args)
{
	return std::make_shared<CGetBlockCountParam>(std::forward<Args>(args)...);
}

// CGetBlockCountResult
class CGetBlockCountResult : public CRPCResult
{
public:
	__required__ CRPCInt64 nCount;
public:
	CGetBlockCountResult();
	CGetBlockCountResult(const CRPCInt64& nCount);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockCountResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockCountResult> MakeCGetBlockCountResultPtr(Args&&... args)
{
	return std::make_shared<CGetBlockCountResult>(std::forward<Args>(args)...);
}

// CGetBlockCountConfig
class CGetBlockCountConfig : virtual public CMvBasicConfig, public CGetBlockCountParam
{
public:
	CGetBlockCountConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getblockhash

// CGetBlockHashParam
class CGetBlockHashParam : public CRPCParam
{
public:
	__required__ CRPCInt64 nHeight;
	__optional__ CRPCString strFork;
public:
	CGetBlockHashParam();
	CGetBlockHashParam(const CRPCInt64& nHeight, const CRPCString& strFork);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockHashParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockHashParam> MakeCGetBlockHashParamPtr(Args&&... args)
{
	return std::make_shared<CGetBlockHashParam>(std::forward<Args>(args)...);
}

// CGetBlockHashResult
class CGetBlockHashResult : public CRPCResult
{
public:
	__required__ CRPCString strHash;
public:
	CGetBlockHashResult();
	CGetBlockHashResult(const CRPCString& strHash);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockHashResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockHashResult> MakeCGetBlockHashResultPtr(Args&&... args)
{
	return std::make_shared<CGetBlockHashResult>(std::forward<Args>(args)...);
}

// CGetBlockHashConfig
class CGetBlockHashConfig : virtual public CMvBasicConfig, public CGetBlockHashParam
{
public:
	CGetBlockHashConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getblock

// CGetBlockParam
class CGetBlockParam : public CRPCParam
{
public:
	__required__ CRPCString strBlock;
public:
	CGetBlockParam();
	CGetBlockParam(const CRPCString& strBlock);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockParam> MakeCGetBlockParamPtr(Args&&... args)
{
	return std::make_shared<CGetBlockParam>(std::forward<Args>(args)...);
}

// CGetBlockResult
class CGetBlockResult : public CRPCResult
{
public:
	__required__ CBlockData block;
public:
	CGetBlockResult();
	CGetBlockResult(const CBlockData& block);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBlockResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBlockResult> MakeCGetBlockResultPtr(Args&&... args)
{
	return std::make_shared<CGetBlockResult>(std::forward<Args>(args)...);
}

// CGetBlockConfig
class CGetBlockConfig : virtual public CMvBasicConfig, public CGetBlockParam
{
public:
	CGetBlockConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// gettxpool

// CGetTxPoolParam
class CGetTxPoolParam : public CRPCParam
{
public:
	__optional__ CRPCString strFork;
	__optional__ CRPCBool fDetail = false;
public:
	CGetTxPoolParam();
	CGetTxPoolParam(const CRPCString& strFork, const CRPCBool& fDetail);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetTxPoolParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetTxPoolParam> MakeCGetTxPoolParamPtr(Args&&... args)
{
	return std::make_shared<CGetTxPoolParam>(std::forward<Args>(args)...);
}

// CGetTxPoolResult
class CGetTxPoolResult : public CRPCResult
{
public:
	// class CList
	class CList
	{
	public:
		__required__ CRPCString strHex;
		__required__ CRPCUint64 nSize;
	public:
		CList();
		CList(const CRPCString& strHex, const CRPCUint64& nSize);
		CList(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CList& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__optional__ CRPCUint64 nCount;
	__optional__ CRPCUint64 nSize;
	__optional__ CRPCVector<CList> vecList;
public:
	CGetTxPoolResult();
	CGetTxPoolResult(const CRPCUint64& nCount, const CRPCUint64& nSize, const CRPCVector<CList>& vecList);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetTxPoolResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetTxPoolResult> MakeCGetTxPoolResultPtr(Args&&... args)
{
	return std::make_shared<CGetTxPoolResult>(std::forward<Args>(args)...);
}

// CGetTxPoolConfig
class CGetTxPoolConfig : virtual public CMvBasicConfig, public CGetTxPoolParam
{
public:
	CGetTxPoolConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// removependingtx

// CRemovePendingTxParam
class CRemovePendingTxParam : public CRPCParam
{
public:
	__required__ CRPCString strTxid;
public:
	CRemovePendingTxParam();
	CRemovePendingTxParam(const CRPCString& strTxid);
	virtual json_spirit::Value ToJSON() const;
	virtual CRemovePendingTxParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CRemovePendingTxParam> MakeCRemovePendingTxParamPtr(Args&&... args)
{
	return std::make_shared<CRemovePendingTxParam>(std::forward<Args>(args)...);
}

// CRemovePendingTxResult
class CRemovePendingTxResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CRemovePendingTxResult();
	CRemovePendingTxResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CRemovePendingTxResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CRemovePendingTxResult> MakeCRemovePendingTxResultPtr(Args&&... args)
{
	return std::make_shared<CRemovePendingTxResult>(std::forward<Args>(args)...);
}

// CRemovePendingTxConfig
class CRemovePendingTxConfig : virtual public CMvBasicConfig, public CRemovePendingTxParam
{
public:
	CRemovePendingTxConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// gettransaction

// CGetTransactionParam
class CGetTransactionParam : public CRPCParam
{
public:
	__required__ CRPCString strTxid;
	__optional__ CRPCBool fSerialized = false;
public:
	CGetTransactionParam();
	CGetTransactionParam(const CRPCString& strTxid, const CRPCBool& fSerialized);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetTransactionParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetTransactionParam> MakeCGetTransactionParamPtr(Args&&... args)
{
	return std::make_shared<CGetTransactionParam>(std::forward<Args>(args)...);
}

// CGetTransactionResult
class CGetTransactionResult : public CRPCResult
{
public:
	__optional__ CRPCString strSerialization;
	__optional__ CTransactionData transaction;
public:
	CGetTransactionResult();
	CGetTransactionResult(const CRPCString& strSerialization, const CTransactionData& transaction);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetTransactionResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetTransactionResult> MakeCGetTransactionResultPtr(Args&&... args)
{
	return std::make_shared<CGetTransactionResult>(std::forward<Args>(args)...);
}

// CGetTransactionConfig
class CGetTransactionConfig : virtual public CMvBasicConfig, public CGetTransactionParam
{
public:
	CGetTransactionConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// sendtransaction

// CSendTransactionParam
class CSendTransactionParam : public CRPCParam
{
public:
	__required__ CRPCString strTxdata;
public:
	CSendTransactionParam();
	CSendTransactionParam(const CRPCString& strTxdata);
	virtual json_spirit::Value ToJSON() const;
	virtual CSendTransactionParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSendTransactionParam> MakeCSendTransactionParamPtr(Args&&... args)
{
	return std::make_shared<CSendTransactionParam>(std::forward<Args>(args)...);
}

// CSendTransactionResult
class CSendTransactionResult : public CRPCResult
{
public:
	__required__ CRPCString strData;
public:
	CSendTransactionResult();
	CSendTransactionResult(const CRPCString& strData);
	virtual json_spirit::Value ToJSON() const;
	virtual CSendTransactionResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSendTransactionResult> MakeCSendTransactionResultPtr(Args&&... args)
{
	return std::make_shared<CSendTransactionResult>(std::forward<Args>(args)...);
}

// CSendTransactionConfig
class CSendTransactionConfig : virtual public CMvBasicConfig, public CSendTransactionParam
{
public:
	CSendTransactionConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// listkey

// CListKeyParam
class CListKeyParam : public CRPCParam
{
public:
	CListKeyParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CListKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListKeyParam> MakeCListKeyParamPtr(Args&&... args)
{
	return std::make_shared<CListKeyParam>(std::forward<Args>(args)...);
}

// CListKeyResult
class CListKeyResult : public CRPCResult
{
public:
	// class CPubkey
	class CPubkey
	{
	public:
		__required__ CRPCString strKey;
		__required__ CRPCString strInfo;
	public:
		CPubkey();
		CPubkey(const CRPCString& strKey, const CRPCString& strInfo);
		CPubkey(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CPubkey& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCVector<CPubkey> vecPubkey = RPCValid;
public:
	CListKeyResult();
	CListKeyResult(const CRPCVector<CPubkey>& vecPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CListKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListKeyResult> MakeCListKeyResultPtr(Args&&... args)
{
	return std::make_shared<CListKeyResult>(std::forward<Args>(args)...);
}

// CListKeyConfig
class CListKeyConfig : virtual public CMvBasicConfig, public CListKeyParam
{
public:
	CListKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getnewkey

// CGetNewKeyParam
class CGetNewKeyParam : public CRPCParam
{
public:
	__optional__ CRPCString strPassphrase;
public:
	CGetNewKeyParam();
	CGetNewKeyParam(const CRPCString& strPassphrase);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetNewKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetNewKeyParam> MakeCGetNewKeyParamPtr(Args&&... args)
{
	return std::make_shared<CGetNewKeyParam>(std::forward<Args>(args)...);
}

// CGetNewKeyResult
class CGetNewKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strPubkey;
public:
	CGetNewKeyResult();
	CGetNewKeyResult(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetNewKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetNewKeyResult> MakeCGetNewKeyResultPtr(Args&&... args)
{
	return std::make_shared<CGetNewKeyResult>(std::forward<Args>(args)...);
}

// CGetNewKeyConfig
class CGetNewKeyConfig : virtual public CMvBasicConfig, public CGetNewKeyParam
{
public:
	CGetNewKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// encryptkey

// CEncryptKeyParam
class CEncryptKeyParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
	__required__ CRPCString strPassphrase;
	__optional__ CRPCString strOldpassphrase;
public:
	CEncryptKeyParam();
	CEncryptKeyParam(const CRPCString& strPubkey, const CRPCString& strPassphrase, const CRPCString& strOldpassphrase);
	virtual json_spirit::Value ToJSON() const;
	virtual CEncryptKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CEncryptKeyParam> MakeCEncryptKeyParamPtr(Args&&... args)
{
	return std::make_shared<CEncryptKeyParam>(std::forward<Args>(args)...);
}

// CEncryptKeyResult
class CEncryptKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CEncryptKeyResult();
	CEncryptKeyResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CEncryptKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CEncryptKeyResult> MakeCEncryptKeyResultPtr(Args&&... args)
{
	return std::make_shared<CEncryptKeyResult>(std::forward<Args>(args)...);
}

// CEncryptKeyConfig
class CEncryptKeyConfig : virtual public CMvBasicConfig, public CEncryptKeyParam
{
public:
	CEncryptKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// lockkey

// CLockKeyParam
class CLockKeyParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
public:
	CLockKeyParam();
	CLockKeyParam(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CLockKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CLockKeyParam> MakeCLockKeyParamPtr(Args&&... args)
{
	return std::make_shared<CLockKeyParam>(std::forward<Args>(args)...);
}

// CLockKeyResult
class CLockKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CLockKeyResult();
	CLockKeyResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CLockKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CLockKeyResult> MakeCLockKeyResultPtr(Args&&... args)
{
	return std::make_shared<CLockKeyResult>(std::forward<Args>(args)...);
}

// CLockKeyConfig
class CLockKeyConfig : virtual public CMvBasicConfig, public CLockKeyParam
{
public:
	CLockKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// unlockkey

// CUnlockKeyParam
class CUnlockKeyParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
	__required__ CRPCString strPassphrase;
	__optional__ CRPCInt64 nTimeout;
public:
	CUnlockKeyParam();
	CUnlockKeyParam(const CRPCString& strPubkey, const CRPCString& strPassphrase, const CRPCInt64& nTimeout);
	virtual json_spirit::Value ToJSON() const;
	virtual CUnlockKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CUnlockKeyParam> MakeCUnlockKeyParamPtr(Args&&... args)
{
	return std::make_shared<CUnlockKeyParam>(std::forward<Args>(args)...);
}

// CUnlockKeyResult
class CUnlockKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CUnlockKeyResult();
	CUnlockKeyResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CUnlockKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CUnlockKeyResult> MakeCUnlockKeyResultPtr(Args&&... args)
{
	return std::make_shared<CUnlockKeyResult>(std::forward<Args>(args)...);
}

// CUnlockKeyConfig
class CUnlockKeyConfig : virtual public CMvBasicConfig, public CUnlockKeyParam
{
public:
	CUnlockKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// importprivkey

// CImportPrivKeyParam
class CImportPrivKeyParam : public CRPCParam
{
public:
	__required__ CRPCString strPrivkey;
	__optional__ CRPCString strPassphrase;
public:
	CImportPrivKeyParam();
	CImportPrivKeyParam(const CRPCString& strPrivkey, const CRPCString& strPassphrase);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportPrivKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportPrivKeyParam> MakeCImportPrivKeyParamPtr(Args&&... args)
{
	return std::make_shared<CImportPrivKeyParam>(std::forward<Args>(args)...);
}

// CImportPrivKeyResult
class CImportPrivKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strPubkey;
public:
	CImportPrivKeyResult();
	CImportPrivKeyResult(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportPrivKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportPrivKeyResult> MakeCImportPrivKeyResultPtr(Args&&... args)
{
	return std::make_shared<CImportPrivKeyResult>(std::forward<Args>(args)...);
}

// CImportPrivKeyConfig
class CImportPrivKeyConfig : virtual public CMvBasicConfig, public CImportPrivKeyParam
{
public:
	CImportPrivKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// importkey

// CImportKeyParam
class CImportKeyParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
public:
	CImportKeyParam();
	CImportKeyParam(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportKeyParam> MakeCImportKeyParamPtr(Args&&... args)
{
	return std::make_shared<CImportKeyParam>(std::forward<Args>(args)...);
}

// CImportKeyResult
class CImportKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strPubkey;
public:
	CImportKeyResult();
	CImportKeyResult(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportKeyResult> MakeCImportKeyResultPtr(Args&&... args)
{
	return std::make_shared<CImportKeyResult>(std::forward<Args>(args)...);
}

// CImportKeyConfig
class CImportKeyConfig : virtual public CMvBasicConfig, public CImportKeyParam
{
public:
	CImportKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// exportkey

// CExportKeyParam
class CExportKeyParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
public:
	CExportKeyParam();
	CExportKeyParam(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CExportKeyParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CExportKeyParam> MakeCExportKeyParamPtr(Args&&... args)
{
	return std::make_shared<CExportKeyParam>(std::forward<Args>(args)...);
}

// CExportKeyResult
class CExportKeyResult : public CRPCResult
{
public:
	__required__ CRPCString strPubkey;
public:
	CExportKeyResult();
	CExportKeyResult(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CExportKeyResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CExportKeyResult> MakeCExportKeyResultPtr(Args&&... args)
{
	return std::make_shared<CExportKeyResult>(std::forward<Args>(args)...);
}

// CExportKeyConfig
class CExportKeyConfig : virtual public CMvBasicConfig, public CExportKeyParam
{
public:
	CExportKeyConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// addnewtemplate

// CAddNewTemplateParam
class CAddNewTemplateParam : public CRPCParam
{
public:
	__required__ CTemplateRequest data;
public:
	CAddNewTemplateParam();
	CAddNewTemplateParam(const CTemplateRequest& data);
	virtual json_spirit::Value ToJSON() const;
	virtual CAddNewTemplateParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CAddNewTemplateParam> MakeCAddNewTemplateParamPtr(Args&&... args)
{
	return std::make_shared<CAddNewTemplateParam>(std::forward<Args>(args)...);
}

// CAddNewTemplateResult
class CAddNewTemplateResult : public CRPCResult
{
public:
	__required__ CRPCString strAddress;
public:
	CAddNewTemplateResult();
	CAddNewTemplateResult(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CAddNewTemplateResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CAddNewTemplateResult> MakeCAddNewTemplateResultPtr(Args&&... args)
{
	return std::make_shared<CAddNewTemplateResult>(std::forward<Args>(args)...);
}

// CAddNewTemplateConfig
class CAddNewTemplateConfig : virtual public CMvBasicConfig, public CAddNewTemplateParam
{
public:
	CAddNewTemplateConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// importtemplate

// CImportTemplateParam
class CImportTemplateParam : public CRPCParam
{
public:
	__required__ CRPCString strData;
public:
	CImportTemplateParam();
	CImportTemplateParam(const CRPCString& strData);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportTemplateParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportTemplateParam> MakeCImportTemplateParamPtr(Args&&... args)
{
	return std::make_shared<CImportTemplateParam>(std::forward<Args>(args)...);
}

// CImportTemplateResult
class CImportTemplateResult : public CRPCResult
{
public:
	__required__ CRPCString strAddress;
public:
	CImportTemplateResult();
	CImportTemplateResult(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportTemplateResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportTemplateResult> MakeCImportTemplateResultPtr(Args&&... args)
{
	return std::make_shared<CImportTemplateResult>(std::forward<Args>(args)...);
}

// CImportTemplateConfig
class CImportTemplateConfig : virtual public CMvBasicConfig, public CImportTemplateParam
{
public:
	CImportTemplateConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// exporttemplate

// CExportTemplateParam
class CExportTemplateParam : public CRPCParam
{
public:
	__required__ CRPCString strAddress;
public:
	CExportTemplateParam();
	CExportTemplateParam(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CExportTemplateParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CExportTemplateParam> MakeCExportTemplateParamPtr(Args&&... args)
{
	return std::make_shared<CExportTemplateParam>(std::forward<Args>(args)...);
}

// CExportTemplateResult
class CExportTemplateResult : public CRPCResult
{
public:
	__required__ CRPCString strData;
public:
	CExportTemplateResult();
	CExportTemplateResult(const CRPCString& strData);
	virtual json_spirit::Value ToJSON() const;
	virtual CExportTemplateResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CExportTemplateResult> MakeCExportTemplateResultPtr(Args&&... args)
{
	return std::make_shared<CExportTemplateResult>(std::forward<Args>(args)...);
}

// CExportTemplateConfig
class CExportTemplateConfig : virtual public CMvBasicConfig, public CExportTemplateParam
{
public:
	CExportTemplateConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// validateaddress

// CValidateAddressParam
class CValidateAddressParam : public CRPCParam
{
public:
	__required__ CRPCString strAddress;
public:
	CValidateAddressParam();
	CValidateAddressParam(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CValidateAddressParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CValidateAddressParam> MakeCValidateAddressParamPtr(Args&&... args)
{
	return std::make_shared<CValidateAddressParam>(std::forward<Args>(args)...);
}

// CValidateAddressResult
class CValidateAddressResult : public CRPCResult
{
public:
	// class CAddressdata
	class CAddressdata
	{
	public:
		__required__ CRPCString strAddress;
		__required__ CRPCBool fIsmine;
		__required__ CRPCString strType;
		__required__ CRPCString strPubkey;
		__required__ CRPCString strTemplate;
		__optional__ CTemplateResponse templatedata;
	public:
		CAddressdata();
		CAddressdata(const CRPCString& strAddress, const CRPCBool& fIsmine, const CRPCString& strType, const CRPCString& strPubkey, const CRPCString& strTemplate, const CTemplateResponse& templatedata);
		CAddressdata(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CAddressdata& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCBool fIsvalid;
	__required__ CAddressdata addressdata;
public:
	CValidateAddressResult();
	CValidateAddressResult(const CRPCBool& fIsvalid, const CAddressdata& addressdata);
	virtual json_spirit::Value ToJSON() const;
	virtual CValidateAddressResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CValidateAddressResult> MakeCValidateAddressResultPtr(Args&&... args)
{
	return std::make_shared<CValidateAddressResult>(std::forward<Args>(args)...);
}

// CValidateAddressConfig
class CValidateAddressConfig : virtual public CMvBasicConfig, public CValidateAddressParam
{
public:
	CValidateAddressConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// resyncwallet

// CResyncWalletParam
class CResyncWalletParam : public CRPCParam
{
public:
	__optional__ CRPCString strAddress;
public:
	CResyncWalletParam();
	CResyncWalletParam(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CResyncWalletParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CResyncWalletParam> MakeCResyncWalletParamPtr(Args&&... args)
{
	return std::make_shared<CResyncWalletParam>(std::forward<Args>(args)...);
}

// CResyncWalletResult
class CResyncWalletResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CResyncWalletResult();
	CResyncWalletResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CResyncWalletResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CResyncWalletResult> MakeCResyncWalletResultPtr(Args&&... args)
{
	return std::make_shared<CResyncWalletResult>(std::forward<Args>(args)...);
}

// CResyncWalletConfig
class CResyncWalletConfig : virtual public CMvBasicConfig, public CResyncWalletParam
{
public:
	CResyncWalletConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getbalance

// CGetBalanceParam
class CGetBalanceParam : public CRPCParam
{
public:
	__optional__ CRPCString strFork;
	__optional__ CRPCString strAddress;
public:
	CGetBalanceParam();
	CGetBalanceParam(const CRPCString& strFork, const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBalanceParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBalanceParam> MakeCGetBalanceParamPtr(Args&&... args)
{
	return std::make_shared<CGetBalanceParam>(std::forward<Args>(args)...);
}

// CGetBalanceResult
class CGetBalanceResult : public CRPCResult
{
public:
	// class CBalance
	class CBalance
	{
	public:
		__required__ CRPCString strAddress;
		__required__ CRPCDouble fAvail;
		__required__ CRPCDouble fLocked;
		__required__ CRPCDouble fUnconfirmed;
	public:
		CBalance();
		CBalance(const CRPCString& strAddress, const CRPCDouble& fAvail, const CRPCDouble& fLocked, const CRPCDouble& fUnconfirmed);
		CBalance(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CBalance& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCVector<CBalance> vecBalance = RPCValid;
public:
	CGetBalanceResult();
	CGetBalanceResult(const CRPCVector<CBalance>& vecBalance);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetBalanceResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetBalanceResult> MakeCGetBalanceResultPtr(Args&&... args)
{
	return std::make_shared<CGetBalanceResult>(std::forward<Args>(args)...);
}

// CGetBalanceConfig
class CGetBalanceConfig : virtual public CMvBasicConfig, public CGetBalanceParam
{
public:
	CGetBalanceConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// listtransaction

// CListTransactionParam
class CListTransactionParam : public CRPCParam
{
public:
	__optional__ CRPCUint64 nCount;
	__optional__ CRPCInt64 nOffset;
public:
	CListTransactionParam();
	CListTransactionParam(const CRPCUint64& nCount, const CRPCInt64& nOffset);
	virtual json_spirit::Value ToJSON() const;
	virtual CListTransactionParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListTransactionParam> MakeCListTransactionParamPtr(Args&&... args)
{
	return std::make_shared<CListTransactionParam>(std::forward<Args>(args)...);
}

// CListTransactionResult
class CListTransactionResult : public CRPCResult
{
public:
	__required__ CRPCVector<CWalletTxData> vecTransaction = RPCValid;
public:
	CListTransactionResult();
	CListTransactionResult(const CRPCVector<CWalletTxData>& vecTransaction);
	virtual json_spirit::Value ToJSON() const;
	virtual CListTransactionResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListTransactionResult> MakeCListTransactionResultPtr(Args&&... args)
{
	return std::make_shared<CListTransactionResult>(std::forward<Args>(args)...);
}

// CListTransactionConfig
class CListTransactionConfig : virtual public CMvBasicConfig, public CListTransactionParam
{
public:
	CListTransactionConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// sendfrom

// CSendFromParam
class CSendFromParam : public CRPCParam
{
public:
	__required__ CRPCString strFrom;
	__required__ CRPCString strTo;
	__required__ CRPCDouble fAmount;
	__optional__ CRPCDouble fTxfee;
	__optional__ CRPCString strFork;
public:
	CSendFromParam();
	CSendFromParam(const CRPCString& strFrom, const CRPCString& strTo, const CRPCDouble& fAmount, const CRPCDouble& fTxfee, const CRPCString& strFork);
	virtual json_spirit::Value ToJSON() const;
	virtual CSendFromParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSendFromParam> MakeCSendFromParamPtr(Args&&... args)
{
	return std::make_shared<CSendFromParam>(std::forward<Args>(args)...);
}

// CSendFromResult
class CSendFromResult : public CRPCResult
{
public:
	__required__ CRPCString strTransaction;
public:
	CSendFromResult();
	CSendFromResult(const CRPCString& strTransaction);
	virtual json_spirit::Value ToJSON() const;
	virtual CSendFromResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSendFromResult> MakeCSendFromResultPtr(Args&&... args)
{
	return std::make_shared<CSendFromResult>(std::forward<Args>(args)...);
}

// CSendFromConfig
class CSendFromConfig : virtual public CMvBasicConfig, public CSendFromParam
{
public:
	CSendFromConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// createtransaction

// CCreateTransactionParam
class CCreateTransactionParam : public CRPCParam
{
public:
	__required__ CRPCString strFrom;
	__required__ CRPCString strTo;
	__required__ CRPCDouble fAmount;
	__optional__ CRPCDouble fTxfee;
	__optional__ CRPCString strFork;
	__optional__ CRPCString strData;
public:
	CCreateTransactionParam();
	CCreateTransactionParam(const CRPCString& strFrom, const CRPCString& strTo, const CRPCDouble& fAmount, const CRPCDouble& fTxfee, const CRPCString& strFork, const CRPCString& strData);
	virtual json_spirit::Value ToJSON() const;
	virtual CCreateTransactionParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CCreateTransactionParam> MakeCCreateTransactionParamPtr(Args&&... args)
{
	return std::make_shared<CCreateTransactionParam>(std::forward<Args>(args)...);
}

// CCreateTransactionResult
class CCreateTransactionResult : public CRPCResult
{
public:
	__required__ CRPCString strTransaction;
public:
	CCreateTransactionResult();
	CCreateTransactionResult(const CRPCString& strTransaction);
	virtual json_spirit::Value ToJSON() const;
	virtual CCreateTransactionResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CCreateTransactionResult> MakeCCreateTransactionResultPtr(Args&&... args)
{
	return std::make_shared<CCreateTransactionResult>(std::forward<Args>(args)...);
}

// CCreateTransactionConfig
class CCreateTransactionConfig : virtual public CMvBasicConfig, public CCreateTransactionParam
{
public:
	CCreateTransactionConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// signtransaction

// CSignTransactionParam
class CSignTransactionParam : public CRPCParam
{
public:
	__required__ CRPCString strTxdata;
public:
	CSignTransactionParam();
	CSignTransactionParam(const CRPCString& strTxdata);
	virtual json_spirit::Value ToJSON() const;
	virtual CSignTransactionParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSignTransactionParam> MakeCSignTransactionParamPtr(Args&&... args)
{
	return std::make_shared<CSignTransactionParam>(std::forward<Args>(args)...);
}

// CSignTransactionResult
class CSignTransactionResult : public CRPCResult
{
public:
	__required__ CRPCString strHex;
	__required__ CRPCBool fComplete;
public:
	CSignTransactionResult();
	CSignTransactionResult(const CRPCString& strHex, const CRPCBool& fComplete);
	virtual json_spirit::Value ToJSON() const;
	virtual CSignTransactionResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSignTransactionResult> MakeCSignTransactionResultPtr(Args&&... args)
{
	return std::make_shared<CSignTransactionResult>(std::forward<Args>(args)...);
}

// CSignTransactionConfig
class CSignTransactionConfig : virtual public CMvBasicConfig, public CSignTransactionParam
{
public:
	CSignTransactionConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// signmessage

// CSignMessageParam
class CSignMessageParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
	__required__ CRPCString strMessage;
public:
	CSignMessageParam();
	CSignMessageParam(const CRPCString& strPubkey, const CRPCString& strMessage);
	virtual json_spirit::Value ToJSON() const;
	virtual CSignMessageParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSignMessageParam> MakeCSignMessageParamPtr(Args&&... args)
{
	return std::make_shared<CSignMessageParam>(std::forward<Args>(args)...);
}

// CSignMessageResult
class CSignMessageResult : public CRPCResult
{
public:
	__required__ CRPCString strSignature;
public:
	CSignMessageResult();
	CSignMessageResult(const CRPCString& strSignature);
	virtual json_spirit::Value ToJSON() const;
	virtual CSignMessageResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSignMessageResult> MakeCSignMessageResultPtr(Args&&... args)
{
	return std::make_shared<CSignMessageResult>(std::forward<Args>(args)...);
}

// CSignMessageConfig
class CSignMessageConfig : virtual public CMvBasicConfig, public CSignMessageParam
{
public:
	CSignMessageConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// listaddress

// CListAddressParam
class CListAddressParam : public CRPCParam
{
public:
	CListAddressParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CListAddressParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListAddressParam> MakeCListAddressParamPtr(Args&&... args)
{
	return std::make_shared<CListAddressParam>(std::forward<Args>(args)...);
}

// CListAddressResult
class CListAddressResult : public CRPCResult
{
public:
	// class CAddressdata
	class CAddressdata
	{
	public:
		__required__ CRPCString strType;
		__required__ CTemplatePubKey pubkey;
		__required__ CRPCString strTemplate;
		__optional__ CTemplateResponse templatedata;
	public:
		CAddressdata();
		CAddressdata(const CRPCString& strType, const CTemplatePubKey& pubkey, const CRPCString& strTemplate, const CTemplateResponse& templatedata);
		CAddressdata(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CAddressdata& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__required__ CRPCVector<CAddressdata> vecAddressdata = RPCValid;
public:
	CListAddressResult();
	CListAddressResult(const CRPCVector<CAddressdata>& vecAddressdata);
	virtual json_spirit::Value ToJSON() const;
	virtual CListAddressResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CListAddressResult> MakeCListAddressResultPtr(Args&&... args)
{
	return std::make_shared<CListAddressResult>(std::forward<Args>(args)...);
}

// CListAddressConfig
class CListAddressConfig : virtual public CMvBasicConfig, public CListAddressParam
{
public:
	CListAddressConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// exportwallet

// CExportWalletParam
class CExportWalletParam : public CRPCParam
{
public:
	__required__ CRPCString strPath;
public:
	CExportWalletParam();
	CExportWalletParam(const CRPCString& strPath);
	virtual json_spirit::Value ToJSON() const;
	virtual CExportWalletParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CExportWalletParam> MakeCExportWalletParamPtr(Args&&... args)
{
	return std::make_shared<CExportWalletParam>(std::forward<Args>(args)...);
}

// CExportWalletResult
class CExportWalletResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CExportWalletResult();
	CExportWalletResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CExportWalletResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CExportWalletResult> MakeCExportWalletResultPtr(Args&&... args)
{
	return std::make_shared<CExportWalletResult>(std::forward<Args>(args)...);
}

// CExportWalletConfig
class CExportWalletConfig : virtual public CMvBasicConfig, public CExportWalletParam
{
public:
	CExportWalletConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// importwallet

// CImportWalletParam
class CImportWalletParam : public CRPCParam
{
public:
	__required__ CRPCString strPath;
public:
	CImportWalletParam();
	CImportWalletParam(const CRPCString& strPath);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportWalletParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportWalletParam> MakeCImportWalletParamPtr(Args&&... args)
{
	return std::make_shared<CImportWalletParam>(std::forward<Args>(args)...);
}

// CImportWalletResult
class CImportWalletResult : public CRPCResult
{
public:
	__required__ CRPCString strResult;
public:
	CImportWalletResult();
	CImportWalletResult(const CRPCString& strResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CImportWalletResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CImportWalletResult> MakeCImportWalletResultPtr(Args&&... args)
{
	return std::make_shared<CImportWalletResult>(std::forward<Args>(args)...);
}

// CImportWalletConfig
class CImportWalletConfig : virtual public CMvBasicConfig, public CImportWalletParam
{
public:
	CImportWalletConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// verifymessage

// CVerifyMessageParam
class CVerifyMessageParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
	__required__ CRPCString strMessage;
	__required__ CRPCString strSig;
public:
	CVerifyMessageParam();
	CVerifyMessageParam(const CRPCString& strPubkey, const CRPCString& strMessage, const CRPCString& strSig);
	virtual json_spirit::Value ToJSON() const;
	virtual CVerifyMessageParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CVerifyMessageParam> MakeCVerifyMessageParamPtr(Args&&... args)
{
	return std::make_shared<CVerifyMessageParam>(std::forward<Args>(args)...);
}

// CVerifyMessageResult
class CVerifyMessageResult : public CRPCResult
{
public:
	__required__ CRPCBool fResult;
public:
	CVerifyMessageResult();
	CVerifyMessageResult(const CRPCBool& fResult);
	virtual json_spirit::Value ToJSON() const;
	virtual CVerifyMessageResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CVerifyMessageResult> MakeCVerifyMessageResultPtr(Args&&... args)
{
	return std::make_shared<CVerifyMessageResult>(std::forward<Args>(args)...);
}

// CVerifyMessageConfig
class CVerifyMessageConfig : virtual public CMvBasicConfig, public CVerifyMessageParam
{
public:
	CVerifyMessageConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// makekeypair

// CMakeKeyPairParam
class CMakeKeyPairParam : public CRPCParam
{
public:
	CMakeKeyPairParam();
	virtual json_spirit::Value ToJSON() const;
	virtual CMakeKeyPairParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CMakeKeyPairParam> MakeCMakeKeyPairParamPtr(Args&&... args)
{
	return std::make_shared<CMakeKeyPairParam>(std::forward<Args>(args)...);
}

// CMakeKeyPairResult
class CMakeKeyPairResult : public CRPCResult
{
public:
	__required__ CRPCString strPrivkey;
	__required__ CRPCString strPubkey;
public:
	CMakeKeyPairResult();
	CMakeKeyPairResult(const CRPCString& strPrivkey, const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CMakeKeyPairResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CMakeKeyPairResult> MakeCMakeKeyPairResultPtr(Args&&... args)
{
	return std::make_shared<CMakeKeyPairResult>(std::forward<Args>(args)...);
}

// CMakeKeyPairConfig
class CMakeKeyPairConfig : virtual public CMvBasicConfig, public CMakeKeyPairParam
{
public:
	CMakeKeyPairConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getpubkeyaddress

// CGetPubkeyAddressParam
class CGetPubkeyAddressParam : public CRPCParam
{
public:
	__required__ CRPCString strPubkey;
public:
	CGetPubkeyAddressParam();
	CGetPubkeyAddressParam(const CRPCString& strPubkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetPubkeyAddressParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetPubkeyAddressParam> MakeCGetPubkeyAddressParamPtr(Args&&... args)
{
	return std::make_shared<CGetPubkeyAddressParam>(std::forward<Args>(args)...);
}

// CGetPubkeyAddressResult
class CGetPubkeyAddressResult : public CRPCResult
{
public:
	__required__ CRPCString strAddress;
public:
	CGetPubkeyAddressResult();
	CGetPubkeyAddressResult(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetPubkeyAddressResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetPubkeyAddressResult> MakeCGetPubkeyAddressResultPtr(Args&&... args)
{
	return std::make_shared<CGetPubkeyAddressResult>(std::forward<Args>(args)...);
}

// CGetPubkeyAddressConfig
class CGetPubkeyAddressConfig : virtual public CMvBasicConfig, public CGetPubkeyAddressParam
{
public:
	CGetPubkeyAddressConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// gettemplateaddress

// CGetTemplateAddressParam
class CGetTemplateAddressParam : public CRPCParam
{
public:
	__required__ CRPCString strTid;
public:
	CGetTemplateAddressParam();
	CGetTemplateAddressParam(const CRPCString& strTid);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetTemplateAddressParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetTemplateAddressParam> MakeCGetTemplateAddressParamPtr(Args&&... args)
{
	return std::make_shared<CGetTemplateAddressParam>(std::forward<Args>(args)...);
}

// CGetTemplateAddressResult
class CGetTemplateAddressResult : public CRPCResult
{
public:
	__required__ CRPCString strAddress;
public:
	CGetTemplateAddressResult();
	CGetTemplateAddressResult(const CRPCString& strAddress);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetTemplateAddressResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetTemplateAddressResult> MakeCGetTemplateAddressResultPtr(Args&&... args)
{
	return std::make_shared<CGetTemplateAddressResult>(std::forward<Args>(args)...);
}

// CGetTemplateAddressConfig
class CGetTemplateAddressConfig : virtual public CMvBasicConfig, public CGetTemplateAddressParam
{
public:
	CGetTemplateAddressConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// maketemplate

// CMakeTemplateParam
class CMakeTemplateParam : public CRPCParam
{
public:
	__required__ CTemplateRequest data;
public:
	CMakeTemplateParam();
	CMakeTemplateParam(const CTemplateRequest& data);
	virtual json_spirit::Value ToJSON() const;
	virtual CMakeTemplateParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CMakeTemplateParam> MakeCMakeTemplateParamPtr(Args&&... args)
{
	return std::make_shared<CMakeTemplateParam>(std::forward<Args>(args)...);
}

// CMakeTemplateResult
class CMakeTemplateResult : public CRPCResult
{
public:
	__required__ CRPCString strAddress;
	__required__ CRPCString strHex;
public:
	CMakeTemplateResult();
	CMakeTemplateResult(const CRPCString& strAddress, const CRPCString& strHex);
	virtual json_spirit::Value ToJSON() const;
	virtual CMakeTemplateResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CMakeTemplateResult> MakeCMakeTemplateResultPtr(Args&&... args)
{
	return std::make_shared<CMakeTemplateResult>(std::forward<Args>(args)...);
}

// CMakeTemplateConfig
class CMakeTemplateConfig : virtual public CMvBasicConfig, public CMakeTemplateParam
{
public:
	CMakeTemplateConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// decodetransaction

// CDecodeTransactionParam
class CDecodeTransactionParam : public CRPCParam
{
public:
	__required__ CRPCString strTxdata;
public:
	CDecodeTransactionParam();
	CDecodeTransactionParam(const CRPCString& strTxdata);
	virtual json_spirit::Value ToJSON() const;
	virtual CDecodeTransactionParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CDecodeTransactionParam> MakeCDecodeTransactionParamPtr(Args&&... args)
{
	return std::make_shared<CDecodeTransactionParam>(std::forward<Args>(args)...);
}

// CDecodeTransactionResult
class CDecodeTransactionResult : public CRPCResult
{
public:
	__required__ CTransactionData transaction;
public:
	CDecodeTransactionResult();
	CDecodeTransactionResult(const CTransactionData& transaction);
	virtual json_spirit::Value ToJSON() const;
	virtual CDecodeTransactionResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CDecodeTransactionResult> MakeCDecodeTransactionResultPtr(Args&&... args)
{
	return std::make_shared<CDecodeTransactionResult>(std::forward<Args>(args)...);
}

// CDecodeTransactionConfig
class CDecodeTransactionConfig : virtual public CMvBasicConfig, public CDecodeTransactionParam
{
public:
	CDecodeTransactionConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// makeorigin

// CMakeOriginParam
class CMakeOriginParam : public CRPCParam
{
public:
	__required__ CRPCString strPrev;
	__required__ CRPCString strAddress;
	__required__ CRPCDouble fAmount;
	__required__ CRPCString strIdent;
public:
	CMakeOriginParam();
	CMakeOriginParam(const CRPCString& strPrev, const CRPCString& strAddress, const CRPCDouble& fAmount, const CRPCString& strIdent);
	virtual json_spirit::Value ToJSON() const;
	virtual CMakeOriginParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CMakeOriginParam> MakeCMakeOriginParamPtr(Args&&... args)
{
	return std::make_shared<CMakeOriginParam>(std::forward<Args>(args)...);
}

// CMakeOriginResult
class CMakeOriginResult : public CRPCResult
{
public:
	__required__ CRPCString strHash;
	__required__ CRPCString strHex;
public:
	CMakeOriginResult();
	CMakeOriginResult(const CRPCString& strHash, const CRPCString& strHex);
	virtual json_spirit::Value ToJSON() const;
	virtual CMakeOriginResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CMakeOriginResult> MakeCMakeOriginResultPtr(Args&&... args)
{
	return std::make_shared<CMakeOriginResult>(std::forward<Args>(args)...);
}

// CMakeOriginConfig
class CMakeOriginConfig : virtual public CMvBasicConfig, public CMakeOriginParam
{
public:
	CMakeOriginConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// getwork

// CGetWorkParam
class CGetWorkParam : public CRPCParam
{
public:
	__optional__ CRPCString strPrev;
public:
	CGetWorkParam();
	CGetWorkParam(const CRPCString& strPrev);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetWorkParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetWorkParam> MakeCGetWorkParamPtr(Args&&... args)
{
	return std::make_shared<CGetWorkParam>(std::forward<Args>(args)...);
}

// CGetWorkResult
class CGetWorkResult : public CRPCResult
{
public:
	// class CWork
	class CWork
	{
	public:
		__required__ CRPCString strPrevblockhash;
		__required__ CRPCUint64 nPrevblocktime;
		__required__ CRPCInt64 nAlgo;
		__required__ CRPCInt64 nBits;
		__required__ CRPCString strData;
	public:
		CWork();
		CWork(const CRPCString& strPrevblockhash, const CRPCUint64& nPrevblocktime, const CRPCInt64& nAlgo, const CRPCInt64& nBits, const CRPCString& strData);
		CWork(const CRPCType& type);
		json_spirit::Value ToJSON() const;
		CWork& FromJSON(const json_spirit::Value& v);
		bool IsValid() const;
	};
public:
	__optional__ CRPCBool fResult;
	__optional__ CWork work;
public:
	CGetWorkResult();
	CGetWorkResult(const CRPCBool& fResult, const CWork& work);
	virtual json_spirit::Value ToJSON() const;
	virtual CGetWorkResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CGetWorkResult> MakeCGetWorkResultPtr(Args&&... args)
{
	return std::make_shared<CGetWorkResult>(std::forward<Args>(args)...);
}

// CGetWorkConfig
class CGetWorkConfig : virtual public CMvBasicConfig, public CGetWorkParam
{
public:
	CGetWorkConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

/////////////////////////////////////////////////////
// submitwork

// CSubmitWorkParam
class CSubmitWorkParam : public CRPCParam
{
public:
	__required__ CRPCString strData;
	__required__ CRPCString strSpent;
	__required__ CRPCString strPrivkey;
public:
	CSubmitWorkParam();
	CSubmitWorkParam(const CRPCString& strData, const CRPCString& strSpent, const CRPCString& strPrivkey);
	virtual json_spirit::Value ToJSON() const;
	virtual CSubmitWorkParam& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSubmitWorkParam> MakeCSubmitWorkParamPtr(Args&&... args)
{
	return std::make_shared<CSubmitWorkParam>(std::forward<Args>(args)...);
}

// CSubmitWorkResult
class CSubmitWorkResult : public CRPCResult
{
public:
	__required__ CRPCString strHash;
public:
	CSubmitWorkResult();
	CSubmitWorkResult(const CRPCString& strHash);
	virtual json_spirit::Value ToJSON() const;
	virtual CSubmitWorkResult& FromJSON(const json_spirit::Value& v);
	virtual std::string Method() const;
};
template <typename... Args>
std::shared_ptr<CSubmitWorkResult> MakeCSubmitWorkResultPtr(Args&&... args)
{
	return std::make_shared<CSubmitWorkResult>(std::forward<Args>(args)...);
}

// CSubmitWorkConfig
class CSubmitWorkConfig : virtual public CMvBasicConfig, public CSubmitWorkParam
{
public:
	CSubmitWorkConfig();
	virtual bool PostLoad();
	virtual std::string ListConfig() const;
	virtual std::string Help() const;
};

# undef __required__
# undef __optional__

# ifdef PUSHED_MACRO_REQUIRED
# pragma pop_macro("__required__")
# endif

# ifdef PUSHED_MACRO_OPTIONAL
# pragma pop_macro("__optional__")
# endif

}  // namespace rpc

}  // namespace multiverse

# endif  // MULTIVERSE_RPC_AUTO_PROTOCOL_H
