# include "auto_protocol.h"

# include <sstream>

# include "json/json_spirit_reader_template.h"
# include "json/json_spirit_utils.h"

using namespace std;
using namespace json_spirit;

namespace multiverse
{
namespace rpc
{
static void CheckJSONType(const Value& value, const string& type, const string& key)
{
	bool b;
	if (type == "int")
	{
		b = (value.type() == int_type);
	}
	else if (type == "uint")
	{
		b = ((value.type() == int_type) && (value.is_uint64() || value.get_int64() >= 0));
	}
	else if (type == "double")
	{
		b = ((value.type() == real_type) || (value.type() == int_type));
	}
	else if (type == "bool")
	{
		b = (value.type() == bool_type);
	}
	else if (type == "string")
	{
		b = (value.type() == str_type);
	}
	else if (type == "array")
	{
		b = (value.type() == array_type);
	}
	else if (type == "object")
	{
		b = (value.type() == obj_type);
	}
	else 
	{
		b = false;
	}
	if (!b)
	{
		throw CRPCException(RPC_INVALID_PARAMS, string("[") + key + "] type is not " + type);
	}
}
template <typename T>
static void CheckIsValid(const T& value, const string& key)
{
	if (!value.IsValid())
	{
		throw CRPCException(RPC_INVALID_PARAMS, string("required param [") + key + "] is not valid");
	}
}

// CTemplatePubKey
CTemplatePubKey::CTemplatePubKey() {}
CTemplatePubKey::CTemplatePubKey(const CRPCString& strKey, const CRPCString& strAddress)
	: strKey(strKey), strAddress(strAddress)
{
}
CTemplatePubKey::CTemplatePubKey(const CRPCType& null)
	: strKey(null), strAddress(null)
{
}
Value CTemplatePubKey::ToJSON() const
{
	Object ret;
	CheckIsValid(strKey, "strKey");
	ret.push_back(Pair("key", std::string(strKey)));
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));

	return ret;
}
CTemplatePubKey& CTemplatePubKey::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplatePubKey");
	auto obj = v.get_obj();
	auto valKey = find_value(obj, "key");
	CheckJSONType(valKey, "string", "key");
	strKey = valKey.get_str();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	return *this;
}
bool CTemplatePubKey::IsValid() const
{
	if (!strKey.IsValid()) { return false; }
	if (!strAddress.IsValid()) { return false; }
	return true;
}

// CTemplatePubKeyWeight
CTemplatePubKeyWeight::CTemplatePubKeyWeight() {}
CTemplatePubKeyWeight::CTemplatePubKeyWeight(const CRPCString& strKey, const CRPCInt64& nWeight)
	: strKey(strKey), nWeight(nWeight)
{
}
CTemplatePubKeyWeight::CTemplatePubKeyWeight(const CRPCType& null)
	: strKey(null), nWeight(null)
{
}
Value CTemplatePubKeyWeight::ToJSON() const
{
	Object ret;
	CheckIsValid(strKey, "strKey");
	ret.push_back(Pair("key", std::string(strKey)));
	CheckIsValid(nWeight, "nWeight");
	ret.push_back(Pair("weight", int64(nWeight)));

	return ret;
}
CTemplatePubKeyWeight& CTemplatePubKeyWeight::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplatePubKeyWeight");
	auto obj = v.get_obj();
	auto valKey = find_value(obj, "key");
	CheckJSONType(valKey, "string", "key");
	strKey = valKey.get_str();
	auto valWeight = find_value(obj, "weight");
	CheckJSONType(valWeight, "int", "weight");
	nWeight = valWeight.get_int64();
	return *this;
}
bool CTemplatePubKeyWeight::IsValid() const
{
	if (!strKey.IsValid()) { return false; }
	if (!nWeight.IsValid()) { return false; }
	return true;
}

// CTemplateRequest::CMultisig
CTemplateRequest::CMultisig::CMultisig() {}
CTemplateRequest::CMultisig::CMultisig(const CRPCInt64& nRequired, const CRPCVector<std::string>& vecPubkeys)
	: nRequired(nRequired), vecPubkeys(vecPubkeys)
{
}
CTemplateRequest::CMultisig::CMultisig(const CRPCType& null)
	: nRequired(null), vecPubkeys(null)
{
}
Value CTemplateRequest::CMultisig::ToJSON() const
{
	Object ret;
	CheckIsValid(nRequired, "nRequired");
	ret.push_back(Pair("required", int64(nRequired)));
	CheckIsValid(vecPubkeys, "vecPubkeys");
	Array vecPubkeysArray;
	for (auto& v : vecPubkeys)
	{
		vecPubkeysArray.push_back(std::string(v));
	}
	ret.push_back(Pair("pubkeys", vecPubkeysArray));

	return ret;
}
CTemplateRequest::CMultisig& CTemplateRequest::CMultisig::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateRequest::CMultisig");
	auto obj = v.get_obj();
	auto valRequired = find_value(obj, "required");
	CheckJSONType(valRequired, "int", "required");
	nRequired = valRequired.get_int64();
	auto valPubkeys = find_value(obj, "pubkeys");
	CheckJSONType(valPubkeys, "array", "pubkeys");
	auto vecPubkeysArray = valPubkeys.get_array();
	for (auto& v : vecPubkeysArray)
	{
		vecPubkeys.push_back(v.get_str());
	}
	return *this;
}
bool CTemplateRequest::CMultisig::IsValid() const
{
	if (!nRequired.IsValid()) { return false; }
	if (!vecPubkeys.IsValid()) { return false; }
	return true;
}

// CTemplateRequest::CFork
CTemplateRequest::CFork::CFork() {}
CTemplateRequest::CFork::CFork(const CRPCString& strRedeem, const CRPCString& strFork)
	: strRedeem(strRedeem), strFork(strFork)
{
}
CTemplateRequest::CFork::CFork(const CRPCType& null)
	: strRedeem(null), strFork(null)
{
}
Value CTemplateRequest::CFork::ToJSON() const
{
	Object ret;
	CheckIsValid(strRedeem, "strRedeem");
	ret.push_back(Pair("redeem", std::string(strRedeem)));
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));

	return ret;
}
CTemplateRequest::CFork& CTemplateRequest::CFork::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateRequest::CFork");
	auto obj = v.get_obj();
	auto valRedeem = find_value(obj, "redeem");
	CheckJSONType(valRedeem, "string", "redeem");
	strRedeem = valRedeem.get_str();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	return *this;
}
bool CTemplateRequest::CFork::IsValid() const
{
	if (!strRedeem.IsValid()) { return false; }
	if (!strFork.IsValid()) { return false; }
	return true;
}

// CTemplateRequest::CDelegate
CTemplateRequest::CDelegate::CDelegate() {}
CTemplateRequest::CDelegate::CDelegate(const CRPCString& strDelegate, const CRPCString& strOwner)
	: strDelegate(strDelegate), strOwner(strOwner)
{
}
CTemplateRequest::CDelegate::CDelegate(const CRPCType& null)
	: strDelegate(null), strOwner(null)
{
}
Value CTemplateRequest::CDelegate::ToJSON() const
{
	Object ret;
	CheckIsValid(strDelegate, "strDelegate");
	ret.push_back(Pair("delegate", std::string(strDelegate)));
	CheckIsValid(strOwner, "strOwner");
	ret.push_back(Pair("owner", std::string(strOwner)));

	return ret;
}
CTemplateRequest::CDelegate& CTemplateRequest::CDelegate::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateRequest::CDelegate");
	auto obj = v.get_obj();
	auto valDelegate = find_value(obj, "delegate");
	CheckJSONType(valDelegate, "string", "delegate");
	strDelegate = valDelegate.get_str();
	auto valOwner = find_value(obj, "owner");
	CheckJSONType(valOwner, "string", "owner");
	strOwner = valOwner.get_str();
	return *this;
}
bool CTemplateRequest::CDelegate::IsValid() const
{
	if (!strDelegate.IsValid()) { return false; }
	if (!strOwner.IsValid()) { return false; }
	return true;
}

// CTemplateRequest::CMint
CTemplateRequest::CMint::CMint() {}
CTemplateRequest::CMint::CMint(const CRPCString& strMint, const CRPCString& strSpent)
	: strMint(strMint), strSpent(strSpent)
{
}
CTemplateRequest::CMint::CMint(const CRPCType& null)
	: strMint(null), strSpent(null)
{
}
Value CTemplateRequest::CMint::ToJSON() const
{
	Object ret;
	CheckIsValid(strMint, "strMint");
	ret.push_back(Pair("mint", std::string(strMint)));
	CheckIsValid(strSpent, "strSpent");
	ret.push_back(Pair("spent", std::string(strSpent)));

	return ret;
}
CTemplateRequest::CMint& CTemplateRequest::CMint::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateRequest::CMint");
	auto obj = v.get_obj();
	auto valMint = find_value(obj, "mint");
	CheckJSONType(valMint, "string", "mint");
	strMint = valMint.get_str();
	auto valSpent = find_value(obj, "spent");
	CheckJSONType(valSpent, "string", "spent");
	strSpent = valSpent.get_str();
	return *this;
}
bool CTemplateRequest::CMint::IsValid() const
{
	if (!strMint.IsValid()) { return false; }
	if (!strSpent.IsValid()) { return false; }
	return true;
}

// CTemplateRequest::CWeighted
CTemplateRequest::CWeighted::CWeighted() {}
CTemplateRequest::CWeighted::CWeighted(const CRPCInt64& nRequired, const CRPCVector<CTemplatePubKeyWeight>& vecPubkeys)
	: nRequired(nRequired), vecPubkeys(vecPubkeys)
{
}
CTemplateRequest::CWeighted::CWeighted(const CRPCType& null)
	: nRequired(null), vecPubkeys(null)
{
}
Value CTemplateRequest::CWeighted::ToJSON() const
{
	Object ret;
	CheckIsValid(nRequired, "nRequired");
	ret.push_back(Pair("required", int64(nRequired)));
	CheckIsValid(vecPubkeys, "vecPubkeys");
	Array vecPubkeysArray;
	for (auto& v : vecPubkeys)
	{
		vecPubkeysArray.push_back(v.ToJSON());
	}
	ret.push_back(Pair("pubkeys", vecPubkeysArray));

	return ret;
}
CTemplateRequest::CWeighted& CTemplateRequest::CWeighted::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateRequest::CWeighted");
	auto obj = v.get_obj();
	auto valRequired = find_value(obj, "required");
	CheckJSONType(valRequired, "int", "required");
	nRequired = valRequired.get_int64();
	auto valPubkeys = find_value(obj, "pubkeys");
	CheckJSONType(valPubkeys, "array", "pubkeys");
	auto vecPubkeysArray = valPubkeys.get_array();
	for (auto& v : vecPubkeysArray)
	{
		vecPubkeys.push_back(CRPCVector<CTemplatePubKeyWeight>::value_type().FromJSON(v));
	}
	return *this;
}
bool CTemplateRequest::CWeighted::IsValid() const
{
	if (!nRequired.IsValid()) { return false; }
	if (!vecPubkeys.IsValid()) { return false; }
	return true;
}

// CTemplateRequest
CTemplateRequest::CTemplateRequest() {}
CTemplateRequest::CTemplateRequest(const CRPCString& strType, const CDelegate& delegate, const CFork& fork, const CMint& mint, const CMultisig& multisig, const CWeighted& weighted)
	: strType(strType), delegate(delegate), fork(fork), mint(mint), multisig(multisig), weighted(weighted)
{
}
CTemplateRequest::CTemplateRequest(const CRPCType& null)
	: strType(null), delegate(null), fork(null), mint(null), multisig(null), weighted(null)
{
}
Value CTemplateRequest::ToJSON() const
{
	Object ret;
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	if (strType == "delegate")
	{
		CheckIsValid(delegate, "delegate");
		ret.push_back(Pair("delegate", delegate.ToJSON()));
	}
	if (strType == "fork")
	{
		CheckIsValid(fork, "fork");
		ret.push_back(Pair("fork", fork.ToJSON()));
	}
	if (strType == "mint")
	{
		CheckIsValid(mint, "mint");
		ret.push_back(Pair("mint", mint.ToJSON()));
	}
	if (strType == "multisig")
	{
		CheckIsValid(multisig, "multisig");
		ret.push_back(Pair("multisig", multisig.ToJSON()));
	}
	if (strType == "weighted")
	{
		CheckIsValid(weighted, "weighted");
		ret.push_back(Pair("weighted", weighted.ToJSON()));
	}

	return ret;
}
CTemplateRequest& CTemplateRequest::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateRequest");
	auto obj = v.get_obj();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	if (strType == "delegate")
	{
		auto valDelegate = find_value(obj, "delegate");
		CheckJSONType(valDelegate, "object", "delegate");
		delegate.FromJSON(valDelegate.get_obj());
	}
	if (strType == "fork")
	{
		auto valFork = find_value(obj, "fork");
		CheckJSONType(valFork, "object", "fork");
		fork.FromJSON(valFork.get_obj());
	}
	if (strType == "mint")
	{
		auto valMint = find_value(obj, "mint");
		CheckJSONType(valMint, "object", "mint");
		mint.FromJSON(valMint.get_obj());
	}
	if (strType == "multisig")
	{
		auto valMultisig = find_value(obj, "multisig");
		CheckJSONType(valMultisig, "object", "multisig");
		multisig.FromJSON(valMultisig.get_obj());
	}
	if (strType == "weighted")
	{
		auto valWeighted = find_value(obj, "weighted");
		CheckJSONType(valWeighted, "object", "weighted");
		weighted.FromJSON(valWeighted.get_obj());
	}
	return *this;
}
bool CTemplateRequest::IsValid() const
{
	if (!strType.IsValid()) { return false; }
	if (strType == "delegate")
	{
		if (!delegate.IsValid()) { return false; }
	}
	if (strType == "fork")
	{
		if (!fork.IsValid()) { return false; }
	}
	if (strType == "mint")
	{
		if (!mint.IsValid()) { return false; }
	}
	if (strType == "multisig")
	{
		if (!multisig.IsValid()) { return false; }
	}
	if (strType == "weighted")
	{
		if (!weighted.IsValid()) { return false; }
	}
	return true;
}

// CTemplateResponse::CFork
CTemplateResponse::CFork::CFork() {}
CTemplateResponse::CFork::CFork(const CRPCString& strRedeem, const CRPCString& strFork)
	: strRedeem(strRedeem), strFork(strFork)
{
}
CTemplateResponse::CFork::CFork(const CRPCType& null)
	: strRedeem(null), strFork(null)
{
}
Value CTemplateResponse::CFork::ToJSON() const
{
	Object ret;
	CheckIsValid(strRedeem, "strRedeem");
	ret.push_back(Pair("redeem", std::string(strRedeem)));
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));

	return ret;
}
CTemplateResponse::CFork& CTemplateResponse::CFork::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateResponse::CFork");
	auto obj = v.get_obj();
	auto valRedeem = find_value(obj, "redeem");
	CheckJSONType(valRedeem, "string", "redeem");
	strRedeem = valRedeem.get_str();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	return *this;
}
bool CTemplateResponse::CFork::IsValid() const
{
	if (!strRedeem.IsValid()) { return false; }
	if (!strFork.IsValid()) { return false; }
	return true;
}

// CTemplateResponse::CWeighted
CTemplateResponse::CWeighted::CWeighted() {}
CTemplateResponse::CWeighted::CWeighted(const CRPCInt64& nSigsrequired, const CRPCVector<CTemplatePubKeyWeight>& vecAddresses)
	: nSigsrequired(nSigsrequired), vecAddresses(vecAddresses)
{
}
CTemplateResponse::CWeighted::CWeighted(const CRPCType& null)
	: nSigsrequired(null), vecAddresses(null)
{
}
Value CTemplateResponse::CWeighted::ToJSON() const
{
	Object ret;
	CheckIsValid(nSigsrequired, "nSigsrequired");
	ret.push_back(Pair("sigsrequired", int64(nSigsrequired)));
	CheckIsValid(vecAddresses, "vecAddresses");
	Array vecAddressesArray;
	for (auto& v : vecAddresses)
	{
		vecAddressesArray.push_back(v.ToJSON());
	}
	ret.push_back(Pair("addresses", vecAddressesArray));

	return ret;
}
CTemplateResponse::CWeighted& CTemplateResponse::CWeighted::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateResponse::CWeighted");
	auto obj = v.get_obj();
	auto valSigsrequired = find_value(obj, "sigsrequired");
	CheckJSONType(valSigsrequired, "int", "sigsrequired");
	nSigsrequired = valSigsrequired.get_int64();
	auto valAddresses = find_value(obj, "addresses");
	CheckJSONType(valAddresses, "array", "addresses");
	auto vecAddressesArray = valAddresses.get_array();
	for (auto& v : vecAddressesArray)
	{
		vecAddresses.push_back(CRPCVector<CTemplatePubKeyWeight>::value_type().FromJSON(v));
	}
	return *this;
}
bool CTemplateResponse::CWeighted::IsValid() const
{
	if (!nSigsrequired.IsValid()) { return false; }
	if (!vecAddresses.IsValid()) { return false; }
	return true;
}

// CTemplateResponse::CDelegate
CTemplateResponse::CDelegate::CDelegate() {}
CTemplateResponse::CDelegate::CDelegate(const CRPCString& strDelegate, const CRPCString& strOwner)
	: strDelegate(strDelegate), strOwner(strOwner)
{
}
CTemplateResponse::CDelegate::CDelegate(const CRPCType& null)
	: strDelegate(null), strOwner(null)
{
}
Value CTemplateResponse::CDelegate::ToJSON() const
{
	Object ret;
	CheckIsValid(strDelegate, "strDelegate");
	ret.push_back(Pair("delegate", std::string(strDelegate)));
	CheckIsValid(strOwner, "strOwner");
	ret.push_back(Pair("owner", std::string(strOwner)));

	return ret;
}
CTemplateResponse::CDelegate& CTemplateResponse::CDelegate::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateResponse::CDelegate");
	auto obj = v.get_obj();
	auto valDelegate = find_value(obj, "delegate");
	CheckJSONType(valDelegate, "string", "delegate");
	strDelegate = valDelegate.get_str();
	auto valOwner = find_value(obj, "owner");
	CheckJSONType(valOwner, "string", "owner");
	strOwner = valOwner.get_str();
	return *this;
}
bool CTemplateResponse::CDelegate::IsValid() const
{
	if (!strDelegate.IsValid()) { return false; }
	if (!strOwner.IsValid()) { return false; }
	return true;
}

// CTemplateResponse::CMint
CTemplateResponse::CMint::CMint() {}
CTemplateResponse::CMint::CMint(const CRPCString& strMint, const CRPCString& strSpent)
	: strMint(strMint), strSpent(strSpent)
{
}
CTemplateResponse::CMint::CMint(const CRPCType& null)
	: strMint(null), strSpent(null)
{
}
Value CTemplateResponse::CMint::ToJSON() const
{
	Object ret;
	CheckIsValid(strMint, "strMint");
	ret.push_back(Pair("mint", std::string(strMint)));
	CheckIsValid(strSpent, "strSpent");
	ret.push_back(Pair("spent", std::string(strSpent)));

	return ret;
}
CTemplateResponse::CMint& CTemplateResponse::CMint::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateResponse::CMint");
	auto obj = v.get_obj();
	auto valMint = find_value(obj, "mint");
	CheckJSONType(valMint, "string", "mint");
	strMint = valMint.get_str();
	auto valSpent = find_value(obj, "spent");
	CheckJSONType(valSpent, "string", "spent");
	strSpent = valSpent.get_str();
	return *this;
}
bool CTemplateResponse::CMint::IsValid() const
{
	if (!strMint.IsValid()) { return false; }
	if (!strSpent.IsValid()) { return false; }
	return true;
}

// CTemplateResponse::CMultisig
CTemplateResponse::CMultisig::CMultisig() {}
CTemplateResponse::CMultisig::CMultisig(const CRPCInt64& nSigsrequired, const CRPCVector<std::string>& vecAddresses)
	: nSigsrequired(nSigsrequired), vecAddresses(vecAddresses)
{
}
CTemplateResponse::CMultisig::CMultisig(const CRPCType& null)
	: nSigsrequired(null), vecAddresses(null)
{
}
Value CTemplateResponse::CMultisig::ToJSON() const
{
	Object ret;
	CheckIsValid(nSigsrequired, "nSigsrequired");
	ret.push_back(Pair("sigsrequired", int64(nSigsrequired)));
	CheckIsValid(vecAddresses, "vecAddresses");
	Array vecAddressesArray;
	for (auto& v : vecAddresses)
	{
		vecAddressesArray.push_back(std::string(v));
	}
	ret.push_back(Pair("addresses", vecAddressesArray));

	return ret;
}
CTemplateResponse::CMultisig& CTemplateResponse::CMultisig::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateResponse::CMultisig");
	auto obj = v.get_obj();
	auto valSigsrequired = find_value(obj, "sigsrequired");
	CheckJSONType(valSigsrequired, "int", "sigsrequired");
	nSigsrequired = valSigsrequired.get_int64();
	auto valAddresses = find_value(obj, "addresses");
	CheckJSONType(valAddresses, "array", "addresses");
	auto vecAddressesArray = valAddresses.get_array();
	for (auto& v : vecAddressesArray)
	{
		vecAddresses.push_back(v.get_str());
	}
	return *this;
}
bool CTemplateResponse::CMultisig::IsValid() const
{
	if (!nSigsrequired.IsValid()) { return false; }
	if (!vecAddresses.IsValid()) { return false; }
	return true;
}

// CTemplateResponse
CTemplateResponse::CTemplateResponse() {}
CTemplateResponse::CTemplateResponse(const CRPCString& strType, const CRPCString& strHex, const CDelegate& delegate, const CFork& fork, const CMint& mint, const CMultisig& multisig, const CWeighted& weighted)
	: strType(strType), strHex(strHex), delegate(delegate), fork(fork), mint(mint), multisig(multisig), weighted(weighted)
{
}
CTemplateResponse::CTemplateResponse(const CRPCType& null)
	: strType(null), strHex(null), delegate(null), fork(null), mint(null), multisig(null), weighted(null)
{
}
Value CTemplateResponse::ToJSON() const
{
	Object ret;
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	CheckIsValid(strHex, "strHex");
	ret.push_back(Pair("hex", std::string(strHex)));
	if (strType == "delegate")
	{
		CheckIsValid(delegate, "delegate");
		ret.push_back(Pair("delegate", delegate.ToJSON()));
	}
	if (strType == "fork")
	{
		CheckIsValid(fork, "fork");
		ret.push_back(Pair("fork", fork.ToJSON()));
	}
	if (strType == "mint")
	{
		CheckIsValid(mint, "mint");
		ret.push_back(Pair("mint", mint.ToJSON()));
	}
	if (strType == "multisig")
	{
		CheckIsValid(multisig, "multisig");
		ret.push_back(Pair("multisig", multisig.ToJSON()));
	}
	if (strType == "weighted")
	{
		CheckIsValid(weighted, "weighted");
		ret.push_back(Pair("weighted", weighted.ToJSON()));
	}

	return ret;
}
CTemplateResponse& CTemplateResponse::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTemplateResponse");
	auto obj = v.get_obj();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	auto valHex = find_value(obj, "hex");
	CheckJSONType(valHex, "string", "hex");
	strHex = valHex.get_str();
	if (strType == "delegate")
	{
		auto valDelegate = find_value(obj, "delegate");
		CheckJSONType(valDelegate, "object", "delegate");
		delegate.FromJSON(valDelegate.get_obj());
	}
	if (strType == "fork")
	{
		auto valFork = find_value(obj, "fork");
		CheckJSONType(valFork, "object", "fork");
		fork.FromJSON(valFork.get_obj());
	}
	if (strType == "mint")
	{
		auto valMint = find_value(obj, "mint");
		CheckJSONType(valMint, "object", "mint");
		mint.FromJSON(valMint.get_obj());
	}
	if (strType == "multisig")
	{
		auto valMultisig = find_value(obj, "multisig");
		CheckJSONType(valMultisig, "object", "multisig");
		multisig.FromJSON(valMultisig.get_obj());
	}
	if (strType == "weighted")
	{
		auto valWeighted = find_value(obj, "weighted");
		CheckJSONType(valWeighted, "object", "weighted");
		weighted.FromJSON(valWeighted.get_obj());
	}
	return *this;
}
bool CTemplateResponse::IsValid() const
{
	if (!strType.IsValid()) { return false; }
	if (!strHex.IsValid()) { return false; }
	if (strType == "delegate")
	{
		if (!delegate.IsValid()) { return false; }
	}
	if (strType == "fork")
	{
		if (!fork.IsValid()) { return false; }
	}
	if (strType == "mint")
	{
		if (!mint.IsValid()) { return false; }
	}
	if (strType == "multisig")
	{
		if (!multisig.IsValid()) { return false; }
	}
	if (strType == "weighted")
	{
		if (!weighted.IsValid()) { return false; }
	}
	return true;
}

// CTransactionData::CVin
CTransactionData::CVin::CVin() {}
CTransactionData::CVin::CVin(const CRPCString& strTxid, const CRPCUint64& nVout)
	: strTxid(strTxid), nVout(nVout)
{
}
CTransactionData::CVin::CVin(const CRPCType& null)
	: strTxid(null), nVout(null)
{
}
Value CTransactionData::CVin::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxid, "strTxid");
	ret.push_back(Pair("txid", std::string(strTxid)));
	CheckIsValid(nVout, "nVout");
	ret.push_back(Pair("vout", uint64(nVout)));

	return ret;
}
CTransactionData::CVin& CTransactionData::CVin::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTransactionData::CVin");
	auto obj = v.get_obj();
	auto valTxid = find_value(obj, "txid");
	CheckJSONType(valTxid, "string", "txid");
	strTxid = valTxid.get_str();
	auto valVout = find_value(obj, "vout");
	CheckJSONType(valVout, "uint", "vout");
	nVout = valVout.get_uint64();
	return *this;
}
bool CTransactionData::CVin::IsValid() const
{
	if (!strTxid.IsValid()) { return false; }
	if (!nVout.IsValid()) { return false; }
	return true;
}

// CTransactionData
CTransactionData::CTransactionData() {}
CTransactionData::CTransactionData(const CRPCString& strTxid, const CRPCUint64& nVersion, const CRPCString& strType, const CRPCUint64& nLockuntil, const CRPCString& strAnchor, const CRPCVector<CVin>& vecVin, const CRPCString& strSendto, const CRPCDouble& fAmount, const CRPCDouble& fTxfee, const CRPCString& strData, const CRPCString& strSig, const CRPCString& strFork, const CRPCInt64& nConfirmations)
	: strTxid(strTxid), nVersion(nVersion), strType(strType), nLockuntil(nLockuntil), strAnchor(strAnchor), vecVin(vecVin), strSendto(strSendto), fAmount(fAmount), fTxfee(fTxfee), strData(strData), strSig(strSig), strFork(strFork), nConfirmations(nConfirmations)
{
}
CTransactionData::CTransactionData(const CRPCType& null)
	: strTxid(null), nVersion(null), strType(null), nLockuntil(null), strAnchor(null), vecVin(null), strSendto(null), fAmount(null), fTxfee(null), strData(null), strSig(null), strFork(null), nConfirmations(null)
{
}
Value CTransactionData::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxid, "strTxid");
	ret.push_back(Pair("txid", std::string(strTxid)));
	CheckIsValid(nVersion, "nVersion");
	ret.push_back(Pair("version", uint64(nVersion)));
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	CheckIsValid(nLockuntil, "nLockuntil");
	ret.push_back(Pair("lockuntil", uint64(nLockuntil)));
	CheckIsValid(strAnchor, "strAnchor");
	ret.push_back(Pair("anchor", std::string(strAnchor)));
	CheckIsValid(vecVin, "vecVin");
	Array vecVinArray;
	for (auto& v : vecVin)
	{
		vecVinArray.push_back(v.ToJSON());
	}
	ret.push_back(Pair("vin", vecVinArray));
	CheckIsValid(strSendto, "strSendto");
	ret.push_back(Pair("sendto", std::string(strSendto)));
	CheckIsValid(fAmount, "fAmount");
	ret.push_back(Pair("amount", double(fAmount)));
	CheckIsValid(fTxfee, "fTxfee");
	ret.push_back(Pair("txfee", double(fTxfee)));
	CheckIsValid(strData, "strData");
	ret.push_back(Pair("data", std::string(strData)));
	CheckIsValid(strSig, "strSig");
	ret.push_back(Pair("sig", std::string(strSig)));
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));
	if (nConfirmations.IsValid())
	{
		ret.push_back(Pair("confirmations", int64(nConfirmations)));
	}

	return ret;
}
CTransactionData& CTransactionData::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CTransactionData");
	auto obj = v.get_obj();
	auto valTxid = find_value(obj, "txid");
	CheckJSONType(valTxid, "string", "txid");
	strTxid = valTxid.get_str();
	auto valVersion = find_value(obj, "version");
	CheckJSONType(valVersion, "uint", "version");
	nVersion = valVersion.get_uint64();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	auto valLockuntil = find_value(obj, "lockuntil");
	CheckJSONType(valLockuntil, "uint", "lockuntil");
	nLockuntil = valLockuntil.get_uint64();
	auto valAnchor = find_value(obj, "anchor");
	CheckJSONType(valAnchor, "string", "anchor");
	strAnchor = valAnchor.get_str();
	auto valVin = find_value(obj, "vin");
	CheckJSONType(valVin, "array", "vin");
	auto vecVinArray = valVin.get_array();
	for (auto& v : vecVinArray)
	{
		vecVin.push_back(CRPCVector<CVin>::value_type().FromJSON(v));
	}
	auto valSendto = find_value(obj, "sendto");
	CheckJSONType(valSendto, "string", "sendto");
	strSendto = valSendto.get_str();
	auto valAmount = find_value(obj, "amount");
	CheckJSONType(valAmount, "double", "amount");
	fAmount = valAmount.get_real();
	auto valTxfee = find_value(obj, "txfee");
	CheckJSONType(valTxfee, "double", "txfee");
	fTxfee = valTxfee.get_real();
	auto valData = find_value(obj, "data");
	CheckJSONType(valData, "string", "data");
	strData = valData.get_str();
	auto valSig = find_value(obj, "sig");
	CheckJSONType(valSig, "string", "sig");
	strSig = valSig.get_str();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	auto valConfirmations = find_value(obj, "confirmations");
	if (!valConfirmations.is_null())
	{
		CheckJSONType(valConfirmations, "int", "confirmations");
		nConfirmations = valConfirmations.get_int64();
	}
	return *this;
}
bool CTransactionData::IsValid() const
{
	if (!strTxid.IsValid()) { return false; }
	if (!nVersion.IsValid()) { return false; }
	if (!strType.IsValid()) { return false; }
	if (!nLockuntil.IsValid()) { return false; }
	if (!strAnchor.IsValid()) { return false; }
	if (!vecVin.IsValid()) { return false; }
	if (!strSendto.IsValid()) { return false; }
	if (!fAmount.IsValid()) { return false; }
	if (!fTxfee.IsValid()) { return false; }
	if (!strData.IsValid()) { return false; }
	if (!strSig.IsValid()) { return false; }
	if (!strFork.IsValid()) { return false; }
	return true;
}

// CWalletTxData
CWalletTxData::CWalletTxData() {}
CWalletTxData::CWalletTxData(const CRPCString& strTxid, const CRPCString& strFork, const CRPCString& strType, const CRPCBool& fSend, const CRPCString& strTo, const CRPCDouble& fAmount, const CRPCDouble& fFee, const CRPCUint64& nLockuntil, const CRPCInt64& nBlockheight, const CRPCString& strFrom)
	: strTxid(strTxid), strFork(strFork), strType(strType), fSend(fSend), strTo(strTo), fAmount(fAmount), fFee(fFee), nLockuntil(nLockuntil), nBlockheight(nBlockheight), strFrom(strFrom)
{
}
CWalletTxData::CWalletTxData(const CRPCType& null)
	: strTxid(null), strFork(null), strType(null), fSend(null), strTo(null), fAmount(null), fFee(null), nLockuntil(null), nBlockheight(null), strFrom(null)
{
}
Value CWalletTxData::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxid, "strTxid");
	ret.push_back(Pair("txid", std::string(strTxid)));
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	CheckIsValid(fSend, "fSend");
	ret.push_back(Pair("send", bool(fSend)));
	CheckIsValid(strTo, "strTo");
	ret.push_back(Pair("to", std::string(strTo)));
	CheckIsValid(fAmount, "fAmount");
	ret.push_back(Pair("amount", double(fAmount)));
	CheckIsValid(fFee, "fFee");
	ret.push_back(Pair("fee", double(fFee)));
	CheckIsValid(nLockuntil, "nLockuntil");
	ret.push_back(Pair("lockuntil", uint64(nLockuntil)));
	if (nBlockheight.IsValid())
	{
		ret.push_back(Pair("blockheight", int64(nBlockheight)));
	}
	if (strFrom.IsValid())
	{
		ret.push_back(Pair("from", std::string(strFrom)));
	}

	return ret;
}
CWalletTxData& CWalletTxData::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CWalletTxData");
	auto obj = v.get_obj();
	auto valTxid = find_value(obj, "txid");
	CheckJSONType(valTxid, "string", "txid");
	strTxid = valTxid.get_str();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	auto valSend = find_value(obj, "send");
	CheckJSONType(valSend, "bool", "send");
	fSend = valSend.get_bool();
	auto valTo = find_value(obj, "to");
	CheckJSONType(valTo, "string", "to");
	strTo = valTo.get_str();
	auto valAmount = find_value(obj, "amount");
	CheckJSONType(valAmount, "double", "amount");
	fAmount = valAmount.get_real();
	auto valFee = find_value(obj, "fee");
	CheckJSONType(valFee, "double", "fee");
	fFee = valFee.get_real();
	auto valLockuntil = find_value(obj, "lockuntil");
	CheckJSONType(valLockuntil, "uint", "lockuntil");
	nLockuntil = valLockuntil.get_uint64();
	auto valBlockheight = find_value(obj, "blockheight");
	if (!valBlockheight.is_null())
	{
		CheckJSONType(valBlockheight, "int", "blockheight");
		nBlockheight = valBlockheight.get_int64();
	}
	auto valFrom = find_value(obj, "from");
	if (!valFrom.is_null())
	{
		CheckJSONType(valFrom, "string", "from");
		strFrom = valFrom.get_str();
	}
	return *this;
}
bool CWalletTxData::IsValid() const
{
	if (!strTxid.IsValid()) { return false; }
	if (!strFork.IsValid()) { return false; }
	if (!strType.IsValid()) { return false; }
	if (!fSend.IsValid()) { return false; }
	if (!strTo.IsValid()) { return false; }
	if (!fAmount.IsValid()) { return false; }
	if (!fFee.IsValid()) { return false; }
	if (!nLockuntil.IsValid()) { return false; }
	return true;
}

// CBlockData
CBlockData::CBlockData() {}
CBlockData::CBlockData(const CRPCString& strHash, const CRPCUint64& nVersion, const CRPCString& strType, const CRPCUint64& nTime, const CRPCString& strFork, const CRPCUint64& nHeight, const CRPCString& strTxmint, const CRPCVector<std::string>& vecTx, const CRPCString& strPrev)
	: strHash(strHash), nVersion(nVersion), strType(strType), nTime(nTime), strFork(strFork), nHeight(nHeight), strTxmint(strTxmint), vecTx(vecTx), strPrev(strPrev)
{
}
CBlockData::CBlockData(const CRPCType& null)
	: strHash(null), nVersion(null), strType(null), nTime(null), strFork(null), nHeight(null), strTxmint(null), vecTx(null), strPrev(null)
{
}
Value CBlockData::ToJSON() const
{
	Object ret;
	CheckIsValid(strHash, "strHash");
	ret.push_back(Pair("hash", std::string(strHash)));
	CheckIsValid(nVersion, "nVersion");
	ret.push_back(Pair("version", uint64(nVersion)));
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	CheckIsValid(nTime, "nTime");
	ret.push_back(Pair("time", uint64(nTime)));
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));
	CheckIsValid(nHeight, "nHeight");
	ret.push_back(Pair("height", uint64(nHeight)));
	CheckIsValid(strTxmint, "strTxmint");
	ret.push_back(Pair("txmint", std::string(strTxmint)));
	CheckIsValid(vecTx, "vecTx");
	Array vecTxArray;
	for (auto& v : vecTx)
	{
		vecTxArray.push_back(std::string(v));
	}
	ret.push_back(Pair("tx", vecTxArray));
	if (strPrev.IsValid())
	{
		ret.push_back(Pair("prev", std::string(strPrev)));
	}

	return ret;
}
CBlockData& CBlockData::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CBlockData");
	auto obj = v.get_obj();
	auto valHash = find_value(obj, "hash");
	CheckJSONType(valHash, "string", "hash");
	strHash = valHash.get_str();
	auto valVersion = find_value(obj, "version");
	CheckJSONType(valVersion, "uint", "version");
	nVersion = valVersion.get_uint64();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	auto valTime = find_value(obj, "time");
	CheckJSONType(valTime, "uint", "time");
	nTime = valTime.get_uint64();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	auto valHeight = find_value(obj, "height");
	CheckJSONType(valHeight, "uint", "height");
	nHeight = valHeight.get_uint64();
	auto valTxmint = find_value(obj, "txmint");
	CheckJSONType(valTxmint, "string", "txmint");
	strTxmint = valTxmint.get_str();
	auto valTx = find_value(obj, "tx");
	CheckJSONType(valTx, "array", "tx");
	auto vecTxArray = valTx.get_array();
	for (auto& v : vecTxArray)
	{
		vecTx.push_back(v.get_str());
	}
	auto valPrev = find_value(obj, "prev");
	if (!valPrev.is_null())
	{
		CheckJSONType(valPrev, "string", "prev");
		strPrev = valPrev.get_str();
	}
	return *this;
}
bool CBlockData::IsValid() const
{
	if (!strHash.IsValid()) { return false; }
	if (!nVersion.IsValid()) { return false; }
	if (!strType.IsValid()) { return false; }
	if (!nTime.IsValid()) { return false; }
	if (!strFork.IsValid()) { return false; }
	if (!nHeight.IsValid()) { return false; }
	if (!strTxmint.IsValid()) { return false; }
	if (!vecTx.IsValid()) { return false; }
	return true;
}

/////////////////////////////////////////////////////
// help

// CHelpParam
CHelpParam::CHelpParam() {}
CHelpParam::CHelpParam(const CRPCString& strCommand)
	: strCommand(strCommand)
{
}
Value CHelpParam::ToJSON() const
{
	Object ret;
	if (strCommand.IsValid())
	{
		ret.push_back(Pair("command", std::string(strCommand)));
	}

	return ret;
}
CHelpParam& CHelpParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "help");
	auto obj = v.get_obj();
	auto valCommand = find_value(obj, "command");
	if (!valCommand.is_null())
	{
		CheckJSONType(valCommand, "string", "command");
		strCommand = valCommand.get_str();
	}
	return *this;
}
string CHelpParam::Method() const
{
	return "help";
}

// CHelpResult
CHelpResult::CHelpResult() {}
CHelpResult::CHelpResult(const CRPCString& strHelp)
	: strHelp(strHelp)
{
}
Value CHelpResult::ToJSON() const
{
	CheckIsValid(strHelp, "strHelp");
	Value val;
	val = Value(strHelp);
	return val;
}
CHelpResult& CHelpResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "help");
	strHelp = v.get_str();
	return *this;
}
string CHelpResult::Method() const
{
	return "help";
}

// CHelpConfig
CHelpConfig::CHelpConfig()
{
}
bool CHelpConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strCommand;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[command] type error, needs string");
		}
	}
	return true;
}
string CHelpConfig::ListConfig() const
{
	return "";
}
string CHelpConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        help (\"command\")\n";
	oss << "\n";
	oss << "List commands, or get help for a command.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"command\"                      (string, optional) command name\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"command\": \"\"                (string, optional) command name\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"help\"               (string, required) help info\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << "\tnone\n\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// stop

// CStopParam
CStopParam::CStopParam() {}
Value CStopParam::ToJSON() const
{
	Object ret;

	return ret;
}
CStopParam& CStopParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "stop");
	auto obj = v.get_obj();
	return *this;
}
string CStopParam::Method() const
{
	return "stop";
}

// CStopResult
CStopResult::CStopResult() {}
CStopResult::CStopResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CStopResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CStopResult& CStopResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CStopResult::Method() const
{
	return "stop";
}

// CStopConfig
CStopConfig::CStopConfig()
{
}
bool CStopConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CStopConfig::ListConfig() const
{
	return "";
}
string CStopConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        stop\n";
	oss << "\n";
	oss << "Stop multiverse server.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) stop result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli stop\n";
	oss << "<< multiverse server stopping\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"stop\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":\"multiverse server stopping\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getpeercount

// CGetPeerCountParam
CGetPeerCountParam::CGetPeerCountParam() {}
Value CGetPeerCountParam::ToJSON() const
{
	Object ret;

	return ret;
}
CGetPeerCountParam& CGetPeerCountParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getpeercount");
	auto obj = v.get_obj();
	return *this;
}
string CGetPeerCountParam::Method() const
{
	return "getpeercount";
}

// CGetPeerCountResult
CGetPeerCountResult::CGetPeerCountResult() {}
CGetPeerCountResult::CGetPeerCountResult(const CRPCInt64& nCount)
	: nCount(nCount)
{
}
Value CGetPeerCountResult::ToJSON() const
{
	CheckIsValid(nCount, "nCount");
	Value val;
	val = Value(nCount);
	return val;
}
CGetPeerCountResult& CGetPeerCountResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "int", "count");
	nCount = v.get_int64();
	return *this;
}
string CGetPeerCountResult::Method() const
{
	return "getpeercount";
}

// CGetPeerCountConfig
CGetPeerCountConfig::CGetPeerCountConfig()
{
}
bool CGetPeerCountConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CGetPeerCountConfig::ListConfig() const
{
	return "";
}
string CGetPeerCountConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getpeercount\n";
	oss << "\n";
	oss << "Returns the number of connections to other nodes.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": count                (int, required) peer count\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getpeercount\n";
	oss << "<< 0\n";
	oss << "\n>> curl -d '{\"id\":3,\"method\":\"getpeercount\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":3,\"jsonrpc\":\"2.0\",\"result\":0}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// listpeer

// CListPeerParam
CListPeerParam::CListPeerParam() {}
Value CListPeerParam::ToJSON() const
{
	Object ret;

	return ret;
}
CListPeerParam& CListPeerParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "listpeer");
	auto obj = v.get_obj();
	return *this;
}
string CListPeerParam::Method() const
{
	return "listpeer";
}

// CListPeerResult::CPeer
CListPeerResult::CPeer::CPeer() {}
CListPeerResult::CPeer::CPeer(const CRPCString& strAddress, const CRPCString& strServices, const CRPCInt64& nLastsend, const CRPCInt64& nLastrecv, const CRPCInt64& nConntime, const CRPCString& strVersion, const CRPCString& strSubver, const CRPCBool& fInbound, const CRPCInt64& nHeight, const CRPCBool& fBanscore)
	: strAddress(strAddress), strServices(strServices), nLastsend(nLastsend), nLastrecv(nLastrecv), nConntime(nConntime), strVersion(strVersion), strSubver(strSubver), fInbound(fInbound), nHeight(nHeight), fBanscore(fBanscore)
{
}
CListPeerResult::CPeer::CPeer(const CRPCType& null)
	: strAddress(null), strServices(null), nLastsend(null), nLastrecv(null), nConntime(null), strVersion(null), strSubver(null), fInbound(null), nHeight(null), fBanscore(null)
{
}
Value CListPeerResult::CPeer::ToJSON() const
{
	Object ret;
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));
	CheckIsValid(strServices, "strServices");
	ret.push_back(Pair("services", std::string(strServices)));
	CheckIsValid(nLastsend, "nLastsend");
	ret.push_back(Pair("lastsend", int64(nLastsend)));
	CheckIsValid(nLastrecv, "nLastrecv");
	ret.push_back(Pair("lastrecv", int64(nLastrecv)));
	CheckIsValid(nConntime, "nConntime");
	ret.push_back(Pair("conntime", int64(nConntime)));
	CheckIsValid(strVersion, "strVersion");
	ret.push_back(Pair("version", std::string(strVersion)));
	CheckIsValid(strSubver, "strSubver");
	ret.push_back(Pair("subver", std::string(strSubver)));
	CheckIsValid(fInbound, "fInbound");
	ret.push_back(Pair("inbound", bool(fInbound)));
	CheckIsValid(nHeight, "nHeight");
	ret.push_back(Pair("height", int64(nHeight)));
	CheckIsValid(fBanscore, "fBanscore");
	ret.push_back(Pair("banscore", bool(fBanscore)));

	return ret;
}
CListPeerResult::CPeer& CListPeerResult::CPeer::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CListPeerResult::CPeer");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	auto valServices = find_value(obj, "services");
	CheckJSONType(valServices, "string", "services");
	strServices = valServices.get_str();
	auto valLastsend = find_value(obj, "lastsend");
	CheckJSONType(valLastsend, "int", "lastsend");
	nLastsend = valLastsend.get_int64();
	auto valLastrecv = find_value(obj, "lastrecv");
	CheckJSONType(valLastrecv, "int", "lastrecv");
	nLastrecv = valLastrecv.get_int64();
	auto valConntime = find_value(obj, "conntime");
	CheckJSONType(valConntime, "int", "conntime");
	nConntime = valConntime.get_int64();
	auto valVersion = find_value(obj, "version");
	CheckJSONType(valVersion, "string", "version");
	strVersion = valVersion.get_str();
	auto valSubver = find_value(obj, "subver");
	CheckJSONType(valSubver, "string", "subver");
	strSubver = valSubver.get_str();
	auto valInbound = find_value(obj, "inbound");
	CheckJSONType(valInbound, "bool", "inbound");
	fInbound = valInbound.get_bool();
	auto valHeight = find_value(obj, "height");
	CheckJSONType(valHeight, "int", "height");
	nHeight = valHeight.get_int64();
	auto valBanscore = find_value(obj, "banscore");
	CheckJSONType(valBanscore, "bool", "banscore");
	fBanscore = valBanscore.get_bool();
	return *this;
}
bool CListPeerResult::CPeer::IsValid() const
{
	if (!strAddress.IsValid()) { return false; }
	if (!strServices.IsValid()) { return false; }
	if (!nLastsend.IsValid()) { return false; }
	if (!nLastrecv.IsValid()) { return false; }
	if (!nConntime.IsValid()) { return false; }
	if (!strVersion.IsValid()) { return false; }
	if (!strSubver.IsValid()) { return false; }
	if (!fInbound.IsValid()) { return false; }
	if (!nHeight.IsValid()) { return false; }
	if (!fBanscore.IsValid()) { return false; }
	return true;
}

// CListPeerResult
CListPeerResult::CListPeerResult() {}
CListPeerResult::CListPeerResult(const CRPCVector<CPeer>& vecPeer)
	: vecPeer(vecPeer)
{
}
Value CListPeerResult::ToJSON() const
{
	Array ret;
	for (auto& v : vecPeer)
	{
		ret.push_back(v.ToJSON());
	}
	return ret;
}
CListPeerResult& CListPeerResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "array", "peer");
	auto vecPeerArray = v.get_array();
	for (auto& v : vecPeerArray)
	{
		vecPeer.push_back(CRPCVector<CPeer>::value_type().FromJSON(v));
	}
	return *this;
}
string CListPeerResult::Method() const
{
	return "listpeer";
}

// CListPeerConfig
CListPeerConfig::CListPeerConfig()
{
}
bool CListPeerConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CListPeerConfig::ListConfig() const
{
	return "";
}
string CListPeerConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        listpeer\n";
	oss << "\n";
	oss << "Returns data about each connected network node.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << "   [\n";
	oss << "     \"peer\":                    (object, required) \n";
	oss << "     {\n";
	oss << "       \"address\": \"\",           (string, required) peer address\n";
	oss << "       \"services\": \"\",          (string, required) service\n";
	oss << "       \"lastsend\": 0,           (int, required) last send time\n";
	oss << "       \"lastrecv\": 0,           (int, required) last receive time\n";
	oss << "       \"conntime\": 0,           (int, required) active time\n";
	oss << "       \"version\": \"\",           (string, required) version\n";
	oss << "       \"subver\": \"\",            (string, required) sub version\n";
	oss << "       \"inbound\": true|false,   (bool, required) accept multiple connection or not\n";
	oss << "       \"height\": 0,             (int, required) starting height\n";
	oss << "       \"banscore\": true|false   (bool, required) ban score\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli listpeer\n";
	oss << "<< [{\"address\":\"113.105.146.22\",\"services\":\"0000000000000001\",\"lastsend\":1538113861,\"lastrecv\":1538113861,\"conntime\":1538113661,\"version\":\"0.1.0\",\"subver\":\"/Multiverse:0.1.0/Protocol:0.1.0/\",\"inbound\":false,\"height\":31028,\"banscore\":true}]\n";
	oss << "\n>> curl -d '{\"id\":40,\"method\":\"listpeer\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":40,\"jsonrpc\":\"2.0\",\"result\":[{\"address\":\"113.105.146.22\",\"services\":\"0000000000000001\",\"lastsend\":1538113861,\"lastrecv\":1538113861,\"conntime\":1538113661,\"version\":\"0.1.0\",\"subver\":\"/Multiverse:0.1.0/Protocol:0.1.0/\",\"inbound\":false,\"height\":31028,\"banscore\":true}]}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// addnode

// CAddNodeParam
CAddNodeParam::CAddNodeParam() {}
CAddNodeParam::CAddNodeParam(const CRPCString& strNode)
	: strNode(strNode)
{
}
Value CAddNodeParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strNode, "strNode");
	ret.push_back(Pair("node", std::string(strNode)));

	return ret;
}
CAddNodeParam& CAddNodeParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "addnode");
	auto obj = v.get_obj();
	auto valNode = find_value(obj, "node");
	CheckJSONType(valNode, "string", "node");
	strNode = valNode.get_str();
	return *this;
}
string CAddNodeParam::Method() const
{
	return "addnode";
}

// CAddNodeResult
CAddNodeResult::CAddNodeResult() {}
CAddNodeResult::CAddNodeResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CAddNodeResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CAddNodeResult& CAddNodeResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CAddNodeResult::Method() const
{
	return "addnode";
}

// CAddNodeConfig
CAddNodeConfig::CAddNodeConfig()
{
}
bool CAddNodeConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strNode;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[node] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[node] is required");
	}
	return true;
}
string CAddNodeConfig::ListConfig() const
{
	return "";
}
string CAddNodeConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        addnode <\"node\">\n";
	oss << "\n";
	oss << "Attempts add a node into the addnode list.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"node\"                         (string, required) node host:port\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"node\": \"\"                   (string, required) node host:port\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) add node result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli addnode 113.105.146.22\n";
	oss << "<< Add node successfully: 113.105.146.22\n";
	oss << "\n>> curl -d '{\"id\":3,\"method\":\"addnode\",\"jsonrpc\":\"2.0\",\"params\":{\"node\":\"113.105.146.22:6811\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":3,\"jsonrpc\":\"2.0\",\"result\":\"Add node successfully: 113.105.146.22:6811\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// removenode

// CRemoveNodeParam
CRemoveNodeParam::CRemoveNodeParam() {}
CRemoveNodeParam::CRemoveNodeParam(const CRPCString& strNode)
	: strNode(strNode)
{
}
Value CRemoveNodeParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strNode, "strNode");
	ret.push_back(Pair("node", std::string(strNode)));

	return ret;
}
CRemoveNodeParam& CRemoveNodeParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "removenode");
	auto obj = v.get_obj();
	auto valNode = find_value(obj, "node");
	CheckJSONType(valNode, "string", "node");
	strNode = valNode.get_str();
	return *this;
}
string CRemoveNodeParam::Method() const
{
	return "removenode";
}

// CRemoveNodeResult
CRemoveNodeResult::CRemoveNodeResult() {}
CRemoveNodeResult::CRemoveNodeResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CRemoveNodeResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CRemoveNodeResult& CRemoveNodeResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CRemoveNodeResult::Method() const
{
	return "removenode";
}

// CRemoveNodeConfig
CRemoveNodeConfig::CRemoveNodeConfig()
{
}
bool CRemoveNodeConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strNode;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[node] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[node] is required");
	}
	return true;
}
string CRemoveNodeConfig::ListConfig() const
{
	return "";
}
string CRemoveNodeConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        removenode <\"node\">\n";
	oss << "\n";
	oss << "Attempts remove a node from the addnode list.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"node\"                         (string, required) node host:port\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"node\": \"\"                   (string, required) node host:port\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) remove node result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli removenode 113.105.146.22\n";
	oss << "<< Remove node successfully: 113.105.146.22\n";
	oss << "\n>> curl -d '{\"id\":67,\"method\":\"removenode\",\"jsonrpc\":\"2.0\",\"params\":{\"node\":\"113.105.146.22:6811\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":67,\"jsonrpc\":\"2.0\",\"result\":\"Remove node successfully: 113.105.146.22:6811\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getforkcount

// CGetForkCountParam
CGetForkCountParam::CGetForkCountParam() {}
Value CGetForkCountParam::ToJSON() const
{
	Object ret;

	return ret;
}
CGetForkCountParam& CGetForkCountParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getforkcount");
	auto obj = v.get_obj();
	return *this;
}
string CGetForkCountParam::Method() const
{
	return "getforkcount";
}

// CGetForkCountResult
CGetForkCountResult::CGetForkCountResult() {}
CGetForkCountResult::CGetForkCountResult(const CRPCInt64& nCount)
	: nCount(nCount)
{
}
Value CGetForkCountResult::ToJSON() const
{
	CheckIsValid(nCount, "nCount");
	Value val;
	val = Value(nCount);
	return val;
}
CGetForkCountResult& CGetForkCountResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "int", "count");
	nCount = v.get_int64();
	return *this;
}
string CGetForkCountResult::Method() const
{
	return "getforkcount";
}

// CGetForkCountConfig
CGetForkCountConfig::CGetForkCountConfig()
{
}
bool CGetForkCountConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CGetForkCountConfig::ListConfig() const
{
	return "";
}
string CGetForkCountConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getforkcount\n";
	oss << "\n";
	oss << "Returns the number of forks.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": count                (int, required) fork count\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getforkcount\n";
	oss << "<< 1\n";
	oss << "\n>> curl -d '{\"id\":69,\"method\":\"getforkcount\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":69,\"jsonrpc\":\"2.0\",\"result\":1}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// listfork

// CListForkParam
CListForkParam::CListForkParam() {}
Value CListForkParam::ToJSON() const
{
	Object ret;

	return ret;
}
CListForkParam& CListForkParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "listfork");
	auto obj = v.get_obj();
	return *this;
}
string CListForkParam::Method() const
{
	return "listfork";
}

// CListForkResult::CProfile
CListForkResult::CProfile::CProfile() {}
CListForkResult::CProfile::CProfile(const CRPCString& strFork, const CRPCString& strName, const CRPCString& strSymbol, const CRPCBool& fIsolated, const CRPCBool& fPrivate, const CRPCBool& fEnclosed, const CRPCString& strOwner)
	: strFork(strFork), strName(strName), strSymbol(strSymbol), fIsolated(fIsolated), fPrivate(fPrivate), fEnclosed(fEnclosed), strOwner(strOwner)
{
}
CListForkResult::CProfile::CProfile(const CRPCType& null)
	: strFork(null), strName(null), strSymbol(null), fIsolated(null), fPrivate(null), fEnclosed(null), strOwner(null)
{
}
Value CListForkResult::CProfile::ToJSON() const
{
	Object ret;
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));
	CheckIsValid(strName, "strName");
	ret.push_back(Pair("name", std::string(strName)));
	CheckIsValid(strSymbol, "strSymbol");
	ret.push_back(Pair("symbol", std::string(strSymbol)));
	CheckIsValid(fIsolated, "fIsolated");
	ret.push_back(Pair("isolated", bool(fIsolated)));
	CheckIsValid(fPrivate, "fPrivate");
	ret.push_back(Pair("private", bool(fPrivate)));
	CheckIsValid(fEnclosed, "fEnclosed");
	ret.push_back(Pair("enclosed", bool(fEnclosed)));
	CheckIsValid(strOwner, "strOwner");
	ret.push_back(Pair("owner", std::string(strOwner)));

	return ret;
}
CListForkResult::CProfile& CListForkResult::CProfile::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CListForkResult::CProfile");
	auto obj = v.get_obj();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	auto valName = find_value(obj, "name");
	CheckJSONType(valName, "string", "name");
	strName = valName.get_str();
	auto valSymbol = find_value(obj, "symbol");
	CheckJSONType(valSymbol, "string", "symbol");
	strSymbol = valSymbol.get_str();
	auto valIsolated = find_value(obj, "isolated");
	CheckJSONType(valIsolated, "bool", "isolated");
	fIsolated = valIsolated.get_bool();
	auto valPrivate = find_value(obj, "private");
	CheckJSONType(valPrivate, "bool", "private");
	fPrivate = valPrivate.get_bool();
	auto valEnclosed = find_value(obj, "enclosed");
	CheckJSONType(valEnclosed, "bool", "enclosed");
	fEnclosed = valEnclosed.get_bool();
	auto valOwner = find_value(obj, "owner");
	CheckJSONType(valOwner, "string", "owner");
	strOwner = valOwner.get_str();
	return *this;
}
bool CListForkResult::CProfile::IsValid() const
{
	if (!strFork.IsValid()) { return false; }
	if (!strName.IsValid()) { return false; }
	if (!strSymbol.IsValid()) { return false; }
	if (!fIsolated.IsValid()) { return false; }
	if (!fPrivate.IsValid()) { return false; }
	if (!fEnclosed.IsValid()) { return false; }
	if (!strOwner.IsValid()) { return false; }
	return true;
}

// CListForkResult
CListForkResult::CListForkResult() {}
CListForkResult::CListForkResult(const CRPCVector<CProfile>& vecProfile)
	: vecProfile(vecProfile)
{
}
Value CListForkResult::ToJSON() const
{
	Array ret;
	for (auto& v : vecProfile)
	{
		ret.push_back(v.ToJSON());
	}
	return ret;
}
CListForkResult& CListForkResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "array", "profile");
	auto vecProfileArray = v.get_array();
	for (auto& v : vecProfileArray)
	{
		vecProfile.push_back(CRPCVector<CProfile>::value_type().FromJSON(v));
	}
	return *this;
}
string CListForkResult::Method() const
{
	return "listfork";
}

// CListForkConfig
CListForkConfig::CListForkConfig()
{
}
bool CListForkConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CListForkConfig::ListConfig() const
{
	return "";
}
string CListForkConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        listfork\n";
	oss << "\n";
	oss << "Returns the list of forks.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << "   [\n";
	oss << "     \"profile\":                 (object, required) fork profile info\n";
	oss << "     {\n";
	oss << "       \"fork\": \"\",              (string, required) fork id with hex system\n";
	oss << "       \"name\": \"\",              (string, required) fork name\n";
	oss << "       \"symbol\": \"\",            (string, required) fork symbol\n";
	oss << "       \"isolated\": true|false,  (bool, required) is isolated\n";
	oss << "       \"private\": true|false,   (bool, required) is private\n";
	oss << "       \"enclosed\": true|false,  (bool, required) is enclosed\n";
	oss << "       \"owner\": \"\"              (string, required) owner's address\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli listfork\n";
	oss << "<< 1\n";
	oss << "\n>> {\"id\":69,\"method\":\"listfork\",\"jsonrpc\":\"2.0\",\"params\":{}}\n";
	oss << "<< {\"id\":69,\"jsonrpc\":\"2.0\",\"result\":[{\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"name\":\"Fission And Fusion Network\",\"symbol\":\"FnFn\",\"isolated\":true,\"private\":false,\"enclosed\":false,\"owner\":\"1mjw7aa0s7v9sv7x3thvcexxzjz4tq82j5qc12dy29ktqy84haa0j7dwb\"}]}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getgenealogy

// CGetGenealogyParam
CGetGenealogyParam::CGetGenealogyParam() {}
CGetGenealogyParam::CGetGenealogyParam(const CRPCString& strFork)
	: strFork(strFork)
{
}
Value CGetGenealogyParam::ToJSON() const
{
	Object ret;
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}

	return ret;
}
CGetGenealogyParam& CGetGenealogyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getgenealogy");
	auto obj = v.get_obj();
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	return *this;
}
string CGetGenealogyParam::Method() const
{
	return "getgenealogy";
}

// CGetGenealogyResult::CAncestry
CGetGenealogyResult::CAncestry::CAncestry() {}
CGetGenealogyResult::CAncestry::CAncestry(const CRPCString& strParent, const CRPCInt64& nHeight)
	: strParent(strParent), nHeight(nHeight)
{
}
CGetGenealogyResult::CAncestry::CAncestry(const CRPCType& null)
	: strParent(null), nHeight(null)
{
}
Value CGetGenealogyResult::CAncestry::ToJSON() const
{
	Object ret;
	CheckIsValid(strParent, "strParent");
	ret.push_back(Pair("parent", std::string(strParent)));
	CheckIsValid(nHeight, "nHeight");
	ret.push_back(Pair("height", int64(nHeight)));

	return ret;
}
CGetGenealogyResult::CAncestry& CGetGenealogyResult::CAncestry::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CGetGenealogyResult::CAncestry");
	auto obj = v.get_obj();
	auto valParent = find_value(obj, "parent");
	CheckJSONType(valParent, "string", "parent");
	strParent = valParent.get_str();
	auto valHeight = find_value(obj, "height");
	CheckJSONType(valHeight, "int", "height");
	nHeight = valHeight.get_int64();
	return *this;
}
bool CGetGenealogyResult::CAncestry::IsValid() const
{
	if (!strParent.IsValid()) { return false; }
	if (!nHeight.IsValid()) { return false; }
	return true;
}

// CGetGenealogyResult::CSubline
CGetGenealogyResult::CSubline::CSubline() {}
CGetGenealogyResult::CSubline::CSubline(const CRPCString& strSub, const CRPCInt64& nHeight)
	: strSub(strSub), nHeight(nHeight)
{
}
CGetGenealogyResult::CSubline::CSubline(const CRPCType& null)
	: strSub(null), nHeight(null)
{
}
Value CGetGenealogyResult::CSubline::ToJSON() const
{
	Object ret;
	CheckIsValid(strSub, "strSub");
	ret.push_back(Pair("sub", std::string(strSub)));
	CheckIsValid(nHeight, "nHeight");
	ret.push_back(Pair("height", int64(nHeight)));

	return ret;
}
CGetGenealogyResult::CSubline& CGetGenealogyResult::CSubline::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CGetGenealogyResult::CSubline");
	auto obj = v.get_obj();
	auto valSub = find_value(obj, "sub");
	CheckJSONType(valSub, "string", "sub");
	strSub = valSub.get_str();
	auto valHeight = find_value(obj, "height");
	CheckJSONType(valHeight, "int", "height");
	nHeight = valHeight.get_int64();
	return *this;
}
bool CGetGenealogyResult::CSubline::IsValid() const
{
	if (!strSub.IsValid()) { return false; }
	if (!nHeight.IsValid()) { return false; }
	return true;
}

// CGetGenealogyResult
CGetGenealogyResult::CGetGenealogyResult() {}
CGetGenealogyResult::CGetGenealogyResult(const CRPCVector<CAncestry>& vecAncestry, const CRPCVector<CSubline>& vecSubline)
	: vecAncestry(vecAncestry), vecSubline(vecSubline)
{
}
Value CGetGenealogyResult::ToJSON() const
{
	Object ret;
	CheckIsValid(vecAncestry, "vecAncestry");
	Array vecAncestryArray;
	for (auto& v : vecAncestry)
	{
		vecAncestryArray.push_back(v.ToJSON());
	}
	ret.push_back(Pair("ancestry", vecAncestryArray));
	CheckIsValid(vecSubline, "vecSubline");
	Array vecSublineArray;
	for (auto& v : vecSubline)
	{
		vecSublineArray.push_back(v.ToJSON());
	}
	ret.push_back(Pair("subline", vecSublineArray));

	return ret;
}
CGetGenealogyResult& CGetGenealogyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getgenealogy");
	auto obj = v.get_obj();
	auto valAncestry = find_value(obj, "ancestry");
	CheckJSONType(valAncestry, "array", "ancestry");
	auto vecAncestryArray = valAncestry.get_array();
	for (auto& v : vecAncestryArray)
	{
		vecAncestry.push_back(CRPCVector<CAncestry>::value_type().FromJSON(v));
	}
	auto valSubline = find_value(obj, "subline");
	CheckJSONType(valSubline, "array", "subline");
	auto vecSublineArray = valSubline.get_array();
	for (auto& v : vecSublineArray)
	{
		vecSubline.push_back(CRPCVector<CSubline>::value_type().FromJSON(v));
	}
	return *this;
}
string CGetGenealogyResult::Method() const
{
	return "getgenealogy";
}

// CGetGenealogyConfig
CGetGenealogyConfig::CGetGenealogyConfig()
{
	boost::program_options::options_description desc("CGetGenealogyConfig");

	AddOpt<string>(desc, "f");

	AddOptions(desc);
}
bool CGetGenealogyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	return true;
}
string CGetGenealogyConfig::ListConfig() const
{
	return "";
}
string CGetGenealogyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getgenealogy (-f=\"fork\")\n";
	oss << "\n";
	oss << "Returns the list of ancestry and subline.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"fork\": \"\"                   (string, optional) fork hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   [\n";
	oss << "     \"ancestry\":                (object, required) ancestry\n";
	oss << "     {\n";
	oss << "       \"parent\": \"\",            (string, required) parent fork hash\n";
	oss << "       \"height\": 0              (int, required) parent origin height\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "   [\n";
	oss << "     \"subline\":                 (object, required) subline\n";
	oss << "     {\n";
	oss << "       \"sub\": \"\",               (string, required) sub fork hash\n";
	oss << "       \"height\": 0              (int, required) sub origin height\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getgenealogy\n";
	oss << "<< {\"ancestry\":[],\"subline\":[]}\n";
	oss << "\n>> curl -d '{\"id\":75,\"method\":\"getgenealogy\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":75,\"jsonrpc\":\"2.0\",\"result\":{\"ancestry\":[],\"subline\":[]}}\n";
	oss << "\n>> multiverse-cli getgenealogy 1\n";
	oss << "<< {\"code\":-6,\"message\":\"Unknown fork\"}\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"getgenealogy\",\"jsonrpc\":\"2.0\",\"params\":{\"fork\":\"1\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"error\":{\"code\":-6,\"message\":\"Unknown fork\"}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getblocklocation

// CGetBlockLocationParam
CGetBlockLocationParam::CGetBlockLocationParam() {}
CGetBlockLocationParam::CGetBlockLocationParam(const CRPCString& strBlock)
	: strBlock(strBlock)
{
}
Value CGetBlockLocationParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strBlock, "strBlock");
	ret.push_back(Pair("block", std::string(strBlock)));

	return ret;
}
CGetBlockLocationParam& CGetBlockLocationParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getblocklocation");
	auto obj = v.get_obj();
	auto valBlock = find_value(obj, "block");
	CheckJSONType(valBlock, "string", "block");
	strBlock = valBlock.get_str();
	return *this;
}
string CGetBlockLocationParam::Method() const
{
	return "getblocklocation";
}

// CGetBlockLocationResult
CGetBlockLocationResult::CGetBlockLocationResult() {}
CGetBlockLocationResult::CGetBlockLocationResult(const CRPCString& strFork, const CRPCInt64& nHeight)
	: strFork(strFork), nHeight(nHeight)
{
}
Value CGetBlockLocationResult::ToJSON() const
{
	Object ret;
	CheckIsValid(strFork, "strFork");
	ret.push_back(Pair("fork", std::string(strFork)));
	CheckIsValid(nHeight, "nHeight");
	ret.push_back(Pair("height", int64(nHeight)));

	return ret;
}
CGetBlockLocationResult& CGetBlockLocationResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getblocklocation");
	auto obj = v.get_obj();
	auto valFork = find_value(obj, "fork");
	CheckJSONType(valFork, "string", "fork");
	strFork = valFork.get_str();
	auto valHeight = find_value(obj, "height");
	CheckJSONType(valHeight, "int", "height");
	nHeight = valHeight.get_int64();
	return *this;
}
string CGetBlockLocationResult::Method() const
{
	return "getblocklocation";
}

// CGetBlockLocationConfig
CGetBlockLocationConfig::CGetBlockLocationConfig()
{
}
bool CGetBlockLocationConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strBlock;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[block] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[block] is required");
	}
	return true;
}
string CGetBlockLocationConfig::ListConfig() const
{
	return "";
}
string CGetBlockLocationConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getblocklocation <\"block\">\n";
	oss << "\n";
	oss << "Returns the location with given block.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"block\"                        (string, required) block hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"block\": \"\"                  (string, required) block hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"fork\": \"\",                  (string, required) fork hash\n";
	oss << "   \"height\": 0                  (int, required) block height\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getblocklocation 609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\n";
	oss << "<< {\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"height\":1}\n";
	oss << "\n>> curl -d '{\"id\":6,\"method\":\"getblocklocation\",\"jsonrpc\":\"2.0\",\"params\":{\"block\":\"609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":6,\"jsonrpc\":\"2.0\",\"result\":{\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"height\":1}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getblockcount

// CGetBlockCountParam
CGetBlockCountParam::CGetBlockCountParam() {}
CGetBlockCountParam::CGetBlockCountParam(const CRPCString& strFork)
	: strFork(strFork)
{
}
Value CGetBlockCountParam::ToJSON() const
{
	Object ret;
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}

	return ret;
}
CGetBlockCountParam& CGetBlockCountParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getblockcount");
	auto obj = v.get_obj();
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	return *this;
}
string CGetBlockCountParam::Method() const
{
	return "getblockcount";
}

// CGetBlockCountResult
CGetBlockCountResult::CGetBlockCountResult() {}
CGetBlockCountResult::CGetBlockCountResult(const CRPCInt64& nCount)
	: nCount(nCount)
{
}
Value CGetBlockCountResult::ToJSON() const
{
	CheckIsValid(nCount, "nCount");
	Value val;
	val = Value(nCount);
	return val;
}
CGetBlockCountResult& CGetBlockCountResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "int", "count");
	nCount = v.get_int64();
	return *this;
}
string CGetBlockCountResult::Method() const
{
	return "getblockcount";
}

// CGetBlockCountConfig
CGetBlockCountConfig::CGetBlockCountConfig()
{
	boost::program_options::options_description desc("CGetBlockCountConfig");

	AddOpt<string>(desc, "f");

	AddOptions(desc);
}
bool CGetBlockCountConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	return true;
}
string CGetBlockCountConfig::ListConfig() const
{
	return "";
}
string CGetBlockCountConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getblockcount (-f=\"fork\")\n";
	oss << "\n";
	oss << "Returns the number of blocks in the given fork.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"fork\": \"\"                   (string, optional) fork hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": count                (int, required) block count\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getblockcount\n";
	oss << "<< 32081\n";
	oss << "\n>> curl -d '{\"id\":4,\"method\":\"getblockcount\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":4,\"jsonrpc\":\"2.0\",\"result\":32081}\n";
	oss << "\n>> multiverse-cli getblockcount 0\n";
	oss << "<< 32081\n";
	oss << "\n>> curl -d '{\"id\":5,\"method\":\"getblockcount\",\"jsonrpc\":\"2.0\",\"params\":{\"fork\":\"0\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":5,\"jsonrpc\":\"2.0\",\"result\":32081}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getblockhash

// CGetBlockHashParam
CGetBlockHashParam::CGetBlockHashParam() {}
CGetBlockHashParam::CGetBlockHashParam(const CRPCInt64& nHeight, const CRPCString& strFork)
	: nHeight(nHeight), strFork(strFork)
{
}
Value CGetBlockHashParam::ToJSON() const
{
	Object ret;
	CheckIsValid(nHeight, "nHeight");
	ret.push_back(Pair("height", int64(nHeight)));
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}

	return ret;
}
CGetBlockHashParam& CGetBlockHashParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getblockhash");
	auto obj = v.get_obj();
	auto valHeight = find_value(obj, "height");
	CheckJSONType(valHeight, "int", "height");
	nHeight = valHeight.get_int64();
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	return *this;
}
string CGetBlockHashParam::Method() const
{
	return "getblockhash";
}

// CGetBlockHashResult
CGetBlockHashResult::CGetBlockHashResult() {}
CGetBlockHashResult::CGetBlockHashResult(const CRPCString& strHash)
	: strHash(strHash)
{
}
Value CGetBlockHashResult::ToJSON() const
{
	CheckIsValid(strHash, "strHash");
	Value val;
	val = Value(strHash);
	return val;
}
CGetBlockHashResult& CGetBlockHashResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "hash");
	strHash = v.get_str();
	return *this;
}
string CGetBlockHashResult::Method() const
{
	return "getblockhash";
}

// CGetBlockHashConfig
CGetBlockHashConfig::CGetBlockHashConfig()
{
	boost::program_options::options_description desc("CGetBlockHashConfig");

	AddOpt<string>(desc, "f");

	AddOptions(desc);
}
bool CGetBlockHashConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> nHeight;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[height] type error, needs int");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[height] is required");
	}
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	return true;
}
string CGetBlockHashConfig::ListConfig() const
{
	return "";
}
string CGetBlockHashConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getblockhash <height> (-f=\"fork\")\n";
	oss << "\n";
	oss << "Returns hash of block in fork at <index>.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " height                         (int, required) block height\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"height\": 0,                 (int, required) block height\n";
	oss << "   \"fork\": \"\"                   (string, optional) fork hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"hash\"               (string, required) block hash\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getblockhash 1\n";
	oss << "<< 609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\n";
	oss << "\n>> curl -d '{\"id\":2,\"method\":\"getblockhash\",\"jsonrpc\":\"2.0\",\"params\":{\"height\":1}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":2,\"jsonrpc\":\"2.0\",\"result\":\"609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\"}\n";
	oss << "\n>> multiverse-cli getblockhash 1 0\n";
	oss << "<< 609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\n";
	oss << "\n>> curl -d '{\"id\":3,\"method\":\"getblockhash\",\"jsonrpc\":\"2.0\",\"params\":{\"height\":1,\"fork\":\"0\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":3,\"jsonrpc\":\"2.0\",\"result\":\"609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\"}\n";
	oss << "\n>> multiverse-cli getblockhash 1 -f=0\n";
	oss << "<< 609a797ca28042d562b11355038c516d65ba30b91c7033d83c61b81aa8c538e3\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getblock

// CGetBlockParam
CGetBlockParam::CGetBlockParam() {}
CGetBlockParam::CGetBlockParam(const CRPCString& strBlock)
	: strBlock(strBlock)
{
}
Value CGetBlockParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strBlock, "strBlock");
	ret.push_back(Pair("block", std::string(strBlock)));

	return ret;
}
CGetBlockParam& CGetBlockParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getblock");
	auto obj = v.get_obj();
	auto valBlock = find_value(obj, "block");
	CheckJSONType(valBlock, "string", "block");
	strBlock = valBlock.get_str();
	return *this;
}
string CGetBlockParam::Method() const
{
	return "getblock";
}

// CGetBlockResult
CGetBlockResult::CGetBlockResult() {}
CGetBlockResult::CGetBlockResult(const CBlockData& block)
	: block(block)
{
}
Value CGetBlockResult::ToJSON() const
{
	CheckIsValid(block, "block");
	return block.ToJSON();
}
CGetBlockResult& CGetBlockResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "block");
	block.FromJSON(v.get_obj());
	return *this;
}
string CGetBlockResult::Method() const
{
	return "getblock";
}

// CGetBlockConfig
CGetBlockConfig::CGetBlockConfig()
{
}
bool CGetBlockConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strBlock;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[block] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[block] is required");
	}
	return true;
}
string CGetBlockConfig::ListConfig() const
{
	return "";
}
string CGetBlockConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getblock <\"block\">\n";
	oss << "\n";
	oss << "Returns details of a block with given block-hash.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"block\"                        (string, required) block hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"block\": \"\"                  (string, required) block hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"hash\": \"\",                  (string, required) block hash\n";
	oss << "   \"version\": 0,                (uint, required) version\n";
	oss << "   \"type\": \"\",                  (string, required) block type\n";
	oss << "   \"time\": 0,                   (uint, required) block time\n";
	oss << "   \"fork\": \"\",                  (string, required) fork hash\n";
	oss << "   \"height\": 0,                 (uint, required) block height\n";
	oss << "   \"txmint\": \"\",                (string, required) transaction mint hash\n";
	oss << "   [\n";
	oss << "     \"tx\": \"\"                   (string, required) transaction hash\n";
	oss << "   ]\n";
	oss << "   \"prev\": \"\"                   (string, optional) previous block hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getblock ca49b8d07ac2849c455a813dd967bb0b306b48406d787259f4ddb8f6a0e0cf4c\n";
	oss << "<< {\"hash\":\"ca49b8d07ac2849c455a813dd967bb0b306b48406d787259f4ddb8f6a0e0cf4c\",\"version\":1,\"type\":\"primary-pow\",\"time\":1538138566,\"prev\":\"47b86e794e7ce0546def4fe3603d58d9cc9fc87eeee676bd15ae90e45ab51f8a\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"height\":31296,\"txmint\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"tx\":[]}\n";
	oss << "\n>> curl -d '{\"id\":10,\"method\":\"getblock\",\"jsonrpc\":\"2.0\",\"params\":{\"block\":\"ca49b8d07ac2849c455a813dd967bb0b306b48406d787259f4ddb8f6a0e0cf4c\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":10,\"jsonrpc\":\"2.0\",\"result\":{\"hash\":\"ca49b8d07ac2849c455a813dd967bb0b306b48406d787259f4ddb8f6a0e0cf4c\",\"version\":1,\"type\":\"primary-pow\",\"time\":1538138566,\"prev\":\"47b86e794e7ce0546def4fe3603d58d9cc9fc87eeee676bd15ae90e45ab51f8a\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"height\":31296,\"txmint\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"tx\":[]}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// gettxpool

// CGetTxPoolParam
CGetTxPoolParam::CGetTxPoolParam() {}
CGetTxPoolParam::CGetTxPoolParam(const CRPCString& strFork, const CRPCBool& fDetail)
	: strFork(strFork), fDetail(fDetail)
{
}
Value CGetTxPoolParam::ToJSON() const
{
	Object ret;
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}
	if (fDetail.IsValid())
	{
		ret.push_back(Pair("detail", bool(fDetail)));
	}

	return ret;
}
CGetTxPoolParam& CGetTxPoolParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "gettxpool");
	auto obj = v.get_obj();
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	auto valDetail = find_value(obj, "detail");
	if (!valDetail.is_null())
	{
		CheckJSONType(valDetail, "bool", "detail");
		fDetail = valDetail.get_bool();
	}
	return *this;
}
string CGetTxPoolParam::Method() const
{
	return "gettxpool";
}

// CGetTxPoolResult::CList
CGetTxPoolResult::CList::CList() {}
CGetTxPoolResult::CList::CList(const CRPCString& strHex, const CRPCUint64& nSize)
	: strHex(strHex), nSize(nSize)
{
}
CGetTxPoolResult::CList::CList(const CRPCType& null)
	: strHex(null), nSize(null)
{
}
Value CGetTxPoolResult::CList::ToJSON() const
{
	Object ret;
	CheckIsValid(strHex, "strHex");
	ret.push_back(Pair("hex", std::string(strHex)));
	CheckIsValid(nSize, "nSize");
	ret.push_back(Pair("size", uint64(nSize)));

	return ret;
}
CGetTxPoolResult::CList& CGetTxPoolResult::CList::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CGetTxPoolResult::CList");
	auto obj = v.get_obj();
	auto valHex = find_value(obj, "hex");
	CheckJSONType(valHex, "string", "hex");
	strHex = valHex.get_str();
	auto valSize = find_value(obj, "size");
	CheckJSONType(valSize, "uint", "size");
	nSize = valSize.get_uint64();
	return *this;
}
bool CGetTxPoolResult::CList::IsValid() const
{
	if (!strHex.IsValid()) { return false; }
	if (!nSize.IsValid()) { return false; }
	return true;
}

// CGetTxPoolResult
CGetTxPoolResult::CGetTxPoolResult() {}
CGetTxPoolResult::CGetTxPoolResult(const CRPCUint64& nCount, const CRPCUint64& nSize, const CRPCVector<CList>& vecList)
	: nCount(nCount), nSize(nSize), vecList(vecList)
{
}
Value CGetTxPoolResult::ToJSON() const
{
	Object ret;
	if (nCount.IsValid())
	{
		ret.push_back(Pair("count", uint64(nCount)));
	}
	if (nSize.IsValid())
	{
		ret.push_back(Pair("size", uint64(nSize)));
	}
	if (vecList.IsValid())
	{
		Array vecListArray;
		for (auto& v : vecList)
		{
			vecListArray.push_back(v.ToJSON());
		}
		ret.push_back(Pair("list", vecListArray));
	}

	return ret;
}
CGetTxPoolResult& CGetTxPoolResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "gettxpool");
	auto obj = v.get_obj();
	auto valCount = find_value(obj, "count");
	if (!valCount.is_null())
	{
		CheckJSONType(valCount, "uint", "count");
		nCount = valCount.get_uint64();
	}
	auto valSize = find_value(obj, "size");
	if (!valSize.is_null())
	{
		CheckJSONType(valSize, "uint", "size");
		nSize = valSize.get_uint64();
	}
	auto valList = find_value(obj, "list");
	if (!valList.is_null())
	{
		CheckJSONType(valList, "array", "list");
		auto vecListArray = valList.get_array();
		for (auto& v : vecListArray)
		{
			vecList.push_back(CRPCVector<CList>::value_type().FromJSON(v));
		}
	}
	return *this;
}
string CGetTxPoolResult::Method() const
{
	return "gettxpool";
}

// CGetTxPoolConfig
CGetTxPoolConfig::CGetTxPoolConfig()
{
	boost::program_options::options_description desc("CGetTxPoolConfig");

	AddOpt<string>(desc, "f");
	AddOpt<bool>(desc, "d");

	AddOptions(desc);
}
bool CGetTxPoolConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	if (vm.find("d") != vm.end())
	{
		auto value = vm["d"];
		fDetail = value.as<bool>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> boolalpha >> fDetail;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[detail] type error, needs bool");
			}
		}
		else
		{
			fDetail = false;
		}
	}
	return true;
}
string CGetTxPoolConfig::ListConfig() const
{
	return "";
}
string CGetTxPoolConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        gettxpool (-f=\"fork\") (-d|-nod*detail*)\n";
	oss << "\n";
	oss << "If detail==0, returns the count and total size of txs for given fork.\n"
	       "Otherwise,returns all transaction ids and sizes in memory pool for given fork.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << " -d|-nod*detail*                (bool, optional, default=false) get detail or not\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"fork\": \"\",                  (string, optional) fork hash\n";
	oss << "   \"detail\": true|false         (bool, optional, default=false) get detail or not\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   (if \"detail\" is false)\n";
	oss << "   \"count\": 0,                  (uint, optional) transaction pool count\n";
	oss << "   (if \"detail\" is false)\n";
	oss << "   \"size\": 0,                   (uint, optional) transaction total size\n";
	oss << "   (if \"detail\" is true)\n";
	oss << "   [\n";
	oss << "     \"pool\":                    (object, required) pool struct\n";
	oss << "     {\n";
	oss << "       \"hex\": \"\",               (string, required) tx pool hex\n";
	oss << "       \"size\": 0                (uint, required) tx pool size\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli gettxpool\n";
	oss << "<< {\"count\":0,\"size\":0}\n";
	oss << "\n>> curl -d '{\"id\":11,\"method\":\"gettxpool\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":11,\"jsonrpc\":\"2.0\",\"result\":{\"count\":0,\"size\":0}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// removependingtx

// CRemovePendingTxParam
CRemovePendingTxParam::CRemovePendingTxParam() {}
CRemovePendingTxParam::CRemovePendingTxParam(const CRPCString& strTxid)
	: strTxid(strTxid)
{
}
Value CRemovePendingTxParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxid, "strTxid");
	ret.push_back(Pair("txid", std::string(strTxid)));

	return ret;
}
CRemovePendingTxParam& CRemovePendingTxParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "removependingtx");
	auto obj = v.get_obj();
	auto valTxid = find_value(obj, "txid");
	CheckJSONType(valTxid, "string", "txid");
	strTxid = valTxid.get_str();
	return *this;
}
string CRemovePendingTxParam::Method() const
{
	return "removependingtx";
}

// CRemovePendingTxResult
CRemovePendingTxResult::CRemovePendingTxResult() {}
CRemovePendingTxResult::CRemovePendingTxResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CRemovePendingTxResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CRemovePendingTxResult& CRemovePendingTxResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CRemovePendingTxResult::Method() const
{
	return "removependingtx";
}

// CRemovePendingTxConfig
CRemovePendingTxConfig::CRemovePendingTxConfig()
{
}
bool CRemovePendingTxConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTxid;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txid] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[txid] is required");
	}
	return true;
}
string CRemovePendingTxConfig::ListConfig() const
{
	return "";
}
string CRemovePendingTxConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        removependingtx <\"txid\">\n";
	oss << "\n";
	oss << "Removes tx whose id is <txid> from txpool.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"txid\"                         (string, required) transaction hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"txid\": \"\"                   (string, required) transaction hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) remove tx result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli removependingtx 01a9f3bb967f24396293903c856e99896a514756a220266afa347a8b8c7f0038\n";
	oss << "<< Remove tx successfully: 01a9f3bb967f24396293903c856e99896a514756a220266afa347a8b8c7f0038\n";
	oss << "\n>> curl -d '{\"id\":21,\"method\":\"removependingtx\",\"jsonrpc\":\"2.0\",\"params\":{\"txid\":\"01a9f3bb967f24396293903c856e99896a514756a220266afa347a8b8c7f0038\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":21,\"jsonrpc\":\"2.0\",\"result\":\"Remove tx successfully: 01a9f3bb967f24396293903c856e99896a514756a220266afa347a8b8c7f0038\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// gettransaction

// CGetTransactionParam
CGetTransactionParam::CGetTransactionParam() {}
CGetTransactionParam::CGetTransactionParam(const CRPCString& strTxid, const CRPCBool& fSerialized)
	: strTxid(strTxid), fSerialized(fSerialized)
{
}
Value CGetTransactionParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxid, "strTxid");
	ret.push_back(Pair("txid", std::string(strTxid)));
	if (fSerialized.IsValid())
	{
		ret.push_back(Pair("serialized", bool(fSerialized)));
	}

	return ret;
}
CGetTransactionParam& CGetTransactionParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "gettransaction");
	auto obj = v.get_obj();
	auto valTxid = find_value(obj, "txid");
	CheckJSONType(valTxid, "string", "txid");
	strTxid = valTxid.get_str();
	auto valSerialized = find_value(obj, "serialized");
	if (!valSerialized.is_null())
	{
		CheckJSONType(valSerialized, "bool", "serialized");
		fSerialized = valSerialized.get_bool();
	}
	return *this;
}
string CGetTransactionParam::Method() const
{
	return "gettransaction";
}

// CGetTransactionResult
CGetTransactionResult::CGetTransactionResult() {}
CGetTransactionResult::CGetTransactionResult(const CRPCString& strSerialization, const CTransactionData& transaction)
	: strSerialization(strSerialization), transaction(transaction)
{
}
Value CGetTransactionResult::ToJSON() const
{
	Object ret;
	if (strSerialization.IsValid())
	{
		ret.push_back(Pair("serialization", std::string(strSerialization)));
	}
	if (transaction.IsValid())
	{
		ret.push_back(Pair("transaction", transaction.ToJSON()));
	}

	return ret;
}
CGetTransactionResult& CGetTransactionResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "gettransaction");
	auto obj = v.get_obj();
	auto valSerialization = find_value(obj, "serialization");
	if (!valSerialization.is_null())
	{
		CheckJSONType(valSerialization, "string", "serialization");
		strSerialization = valSerialization.get_str();
	}
	auto valTransaction = find_value(obj, "transaction");
	if (!valTransaction.is_null())
	{
		CheckJSONType(valTransaction, "object", "transaction");
		transaction.FromJSON(valTransaction.get_obj());
	}
	return *this;
}
string CGetTransactionResult::Method() const
{
	return "gettransaction";
}

// CGetTransactionConfig
CGetTransactionConfig::CGetTransactionConfig()
{
	boost::program_options::options_description desc("CGetTransactionConfig");

	AddOpt<bool>(desc, "s");

	AddOptions(desc);
}
bool CGetTransactionConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTxid;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txid] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[txid] is required");
	}
	if (vm.find("s") != vm.end())
	{
		auto value = vm["s"];
		fSerialized = value.as<bool>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> boolalpha >> fSerialized;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[serialized] type error, needs bool");
			}
		}
		else
		{
			fSerialized = false;
		}
	}
	return true;
}
string CGetTransactionConfig::ListConfig() const
{
	return "";
}
string CGetTransactionConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        gettransaction <\"txid\"> (-s|-nos*serialized*)\n";
	oss << "\n";
	oss << "Get transaction info\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"txid\"                         (string, required) transaction hash\n";
	oss << " -s|-nos*serialized*            (bool, optional, default=false) If serialized=0,\n"
	       "                                 returns an Object with information about <txid>.\nIf \n"
	       "                                serialized is non-zero, returns a string that is\nserialized,\n"
	       "                                 hex-encoded data for <txid>.\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"txid\": \"\",                  (string, required) transaction hash\n";
	oss << "   \"serialized\": true|false     (bool, optional, default=false) If serialized=0,\n"
	       "                                 returns an Object with information about <txid>.\nIf \n"
	       "                                serialized is non-zero, returns a string that is\nserialized,\n"
	       "                                 hex-encoded data for <txid>.\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   (if \"serialized\" is true)\n";
	oss << "   \"serialization\": \"\",         (string, optional) transaction hex data\n";
	oss << "   (if \"serialized\" is false)\n";
	oss << "   \"transaction\":               (object, optional) transaction data\n";
	oss << "   {\n";
	oss << "     \"txid\": \"\",                (string, required) transaction hash\n";
	oss << "     \"version\": 0,              (uint, required) version\n";
	oss << "     \"type\": \"\",                (string, required) transaction type\n";
	oss << "     \"lockuntil\": 0,            (uint, required) unlock time\n";
	oss << "     \"anchor\": \"\",              (string, required) anchor hash\n";
	oss << "     [\n";
	oss << "       \"vin\":                   (object, required) vin struct\n";
	oss << "       {\n";
	oss << "         \"txid\": \"\",            (string, required) pre-vout transaction hash\n";
	oss << "         \"vout\": 0              (uint, required) pre-vout number\n";
	oss << "       }\n";
	oss << "     ]\n";
	oss << "     \"sendto\": \"\",              (string, required) send to address\n";
	oss << "     \"amount\": 0.0,             (double, required) amount\n";
	oss << "     \"txfee\": 0.0,              (double, required) transaction fee\n";
	oss << "     \"data\": \"\",                (string, required) data\n";
	oss << "     \"sig\": \"\",                 (string, required) sign\n";
	oss << "     \"fork\": \"\",                (string, required) fork hash\n";
	oss << "     \"confirmations\": 0         (int, optional) confirmations\n";
	oss << "   }\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli gettransaction 3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\n";
	oss << "<< {\"transaction\":{\"txid\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"version\":1,\"type\":\"work\",\"lockuntil\":0,\"anchor\":\"47b86e794e7ce0546def4fe3603d58d9cc9fc87eeee676bd15ae90e45ab51f8a\",\"vin\":[],\"sendto\":\"20g00k7pe4krdbbxpn5swgbxc0w4a54w054stt1z107d9n3sk6q7js9gw\",\"amount\":15.00000000,\"txfee\":0.00000000,\"data\":\"\",\"sig\":\"\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"confirmations\":785}}\n";
	oss << "\n>> curl -d '{\"id\":13,\"method\":\"gettransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"txid\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"serialized\":false}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":13,\"jsonrpc\":\"2.0\",\"result\":{\"transaction\":{\"txid\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"version\":1,\"type\":\"work\",\"lockuntil\":0,\"anchor\":\"47b86e794e7ce0546def4fe3603d58d9cc9fc87eeee676bd15ae90e45ab51f8a\",\"vin\":[],\"sendto\":\"20g00k7pe4krdbbxpn5swgbxc0w4a54w054stt1z107d9n3sk6q7js9gw\",\"amount\":15.00000000,\"txfee\":0.00000000,\"data\":\"\",\"sig\":\"\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"confirmations\":785}}}\n";
	oss << "\n>> multiverse-cli gettransaction -s 3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\n";
	oss << "<< {\"serialization\":\"01000003000000008a1fb55ae490ae15bd76e6ee7ec89fccd9583d60e34fef6d54e07c4e796eb84700020400099ece24f0d5afb6a973c82fac0708a293802933ad07e101da9a8f3335cfc0e1e4000000000000000000000000000000\"}\n";
	oss << "\n>> curl -d '{\"id\":13,\"method\":\"gettransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"txid\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"serialized\":false}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":14,\"method\":\"gettransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"txid\":\"3d4ed629c594b924d72480e29a332ca91915be685c85940a8c501f8248269e29\",\"serialized\":true}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// sendtransaction

// CSendTransactionParam
CSendTransactionParam::CSendTransactionParam() {}
CSendTransactionParam::CSendTransactionParam(const CRPCString& strTxdata)
	: strTxdata(strTxdata)
{
}
Value CSendTransactionParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxdata, "strTxdata");
	ret.push_back(Pair("txdata", std::string(strTxdata)));

	return ret;
}
CSendTransactionParam& CSendTransactionParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "sendtransaction");
	auto obj = v.get_obj();
	auto valTxdata = find_value(obj, "txdata");
	CheckJSONType(valTxdata, "string", "txdata");
	strTxdata = valTxdata.get_str();
	return *this;
}
string CSendTransactionParam::Method() const
{
	return "sendtransaction";
}

// CSendTransactionResult
CSendTransactionResult::CSendTransactionResult() {}
CSendTransactionResult::CSendTransactionResult(const CRPCString& strData)
	: strData(strData)
{
}
Value CSendTransactionResult::ToJSON() const
{
	CheckIsValid(strData, "strData");
	Value val;
	val = Value(strData);
	return val;
}
CSendTransactionResult& CSendTransactionResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "data");
	strData = v.get_str();
	return *this;
}
string CSendTransactionResult::Method() const
{
	return "sendtransaction";
}

// CSendTransactionConfig
CSendTransactionConfig::CSendTransactionConfig()
{
}
bool CSendTransactionConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTxdata;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txdata] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[txdata] is required");
	}
	return true;
}
string CSendTransactionConfig::ListConfig() const
{
	return "";
}
string CSendTransactionConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        sendtransaction <\"txdata\">\n";
	oss << "\n";
	oss << "Submits raw transaction (serialized, hex-encoded) to local node and network.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"txdata\"                       (string, required) transaction binary data\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"txdata\": \"\"                 (string, required) transaction binary data\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"data\"               (string, required) transaction raw data\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli sendtransaction 01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a0860100000000000212348182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e0182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052ed494d90cd96c252446b4a10459fea8c06186154b2bee2ce2182556e9ba40e7e69ddae2501862e4251bba2abf11c90d6f1fd0dec48a1419e81bb8c7d922cf3e03\n";
	oss << "<< 0a1b944071970589aa524a6f4e40e0b50bab9a64feefc292867692bbf35442a6\n";
	oss << "\n>> curl -d '{\"id\":9,\"method\":\"sendtransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"txdata\":\"01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a0860100000000000212348182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e0182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052ed494d90cd96c252446b4a10459fea8c06186154b2bee2ce2182556e9ba40e7e69ddae2501862e4251bba2abf11c90d6f1fd0dec48a1419e81bb8c7d922cf3e03\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":9,\"jsonrpc\":\"2.0\",\"result\":\"0a1b944071970589aa524a6f4e40e0b50bab9a64feefc292867692bbf35442a6\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-8,\"message\":\"TX decode failed\"}\n";
	oss << "* {\"code\":-10,\"message\":\"Tx rejected : xxx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// listkey

// CListKeyParam
CListKeyParam::CListKeyParam() {}
Value CListKeyParam::ToJSON() const
{
	Object ret;

	return ret;
}
CListKeyParam& CListKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "listkey");
	auto obj = v.get_obj();
	return *this;
}
string CListKeyParam::Method() const
{
	return "listkey";
}

// CListKeyResult::CPubkey
CListKeyResult::CPubkey::CPubkey() {}
CListKeyResult::CPubkey::CPubkey(const CRPCString& strKey, const CRPCString& strInfo)
	: strKey(strKey), strInfo(strInfo)
{
}
CListKeyResult::CPubkey::CPubkey(const CRPCType& null)
	: strKey(null), strInfo(null)
{
}
Value CListKeyResult::CPubkey::ToJSON() const
{
	Object ret;
	CheckIsValid(strKey, "strKey");
	ret.push_back(Pair("key", std::string(strKey)));
	CheckIsValid(strInfo, "strInfo");
	ret.push_back(Pair("info", std::string(strInfo)));

	return ret;
}
CListKeyResult::CPubkey& CListKeyResult::CPubkey::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CListKeyResult::CPubkey");
	auto obj = v.get_obj();
	auto valKey = find_value(obj, "key");
	CheckJSONType(valKey, "string", "key");
	strKey = valKey.get_str();
	auto valInfo = find_value(obj, "info");
	CheckJSONType(valInfo, "string", "info");
	strInfo = valInfo.get_str();
	return *this;
}
bool CListKeyResult::CPubkey::IsValid() const
{
	if (!strKey.IsValid()) { return false; }
	if (!strInfo.IsValid()) { return false; }
	return true;
}

// CListKeyResult
CListKeyResult::CListKeyResult() {}
CListKeyResult::CListKeyResult(const CRPCVector<CPubkey>& vecPubkey)
	: vecPubkey(vecPubkey)
{
}
Value CListKeyResult::ToJSON() const
{
	Array ret;
	for (auto& v : vecPubkey)
	{
		ret.push_back(v.ToJSON());
	}
	return ret;
}
CListKeyResult& CListKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "array", "pubkey");
	auto vecPubkeyArray = v.get_array();
	for (auto& v : vecPubkeyArray)
	{
		vecPubkey.push_back(CRPCVector<CPubkey>::value_type().FromJSON(v));
	}
	return *this;
}
string CListKeyResult::Method() const
{
	return "listkey";
}

// CListKeyConfig
CListKeyConfig::CListKeyConfig()
{
}
bool CListKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CListKeyConfig::ListConfig() const
{
	return "";
}
string CListKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        listkey\n";
	oss << "\n";
	oss << "Returns Object that has pubkey as keys, associated status as values.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << "   [\n";
	oss << "     \"pubkey\":                  (object, required) public key info\n";
	oss << "     {\n";
	oss << "       \"key\": \"\",               (string, required) public key with hex system\n";
	oss << "       \"info\": \"\"               (string, required) key other info\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli listkey\n";
	oss << "<< [{\"key\":\"3d266a564ec85f3385babf615b1d7eeb01b3e4456d35174732bb9ec0fa8c8f4f\",\"info\":\"ver=0;locked\"},{\"key\":\"58e148d9e8610a6504c26ed346d15920c4d832cf0f03ecb8a016e0d0ec838b1b\",\"info\":\"ver=0;locked\"}]\n";
	oss << "\n>> curl -d '{\"id\":43,\"method\":\"listkey\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":43,\"jsonrpc\":\"2.0\",\"result\":[{\"key\":\"3d266a564ec85f3385babf615b1d7eeb01b3e4456d35174732bb9ec0fa8c8f4f\",\"info\":\"ver=0;locked\"},{\"key\":\"58e148d9e8610a6504c26ed346d15920c4d832cf0f03ecb8a016e0d0ec838b1b\",\"info\":\"ver=1;unlocked;timeout=5\"}]}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getnewkey

// CGetNewKeyParam
CGetNewKeyParam::CGetNewKeyParam() {}
CGetNewKeyParam::CGetNewKeyParam(const CRPCString& strPassphrase)
	: strPassphrase(strPassphrase)
{
}
Value CGetNewKeyParam::ToJSON() const
{
	Object ret;
	if (strPassphrase.IsValid())
	{
		ret.push_back(Pair("passphrase", std::string(strPassphrase)));
	}

	return ret;
}
CGetNewKeyParam& CGetNewKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getnewkey");
	auto obj = v.get_obj();
	auto valPassphrase = find_value(obj, "passphrase");
	if (!valPassphrase.is_null())
	{
		CheckJSONType(valPassphrase, "string", "passphrase");
		strPassphrase = valPassphrase.get_str();
	}
	return *this;
}
string CGetNewKeyParam::Method() const
{
	return "getnewkey";
}

// CGetNewKeyResult
CGetNewKeyResult::CGetNewKeyResult() {}
CGetNewKeyResult::CGetNewKeyResult(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CGetNewKeyResult::ToJSON() const
{
	CheckIsValid(strPubkey, "strPubkey");
	Value val;
	val = Value(strPubkey);
	return val;
}
CGetNewKeyResult& CGetNewKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "pubkey");
	strPubkey = v.get_str();
	return *this;
}
string CGetNewKeyResult::Method() const
{
	return "getnewkey";
}

// CGetNewKeyConfig
CGetNewKeyConfig::CGetNewKeyConfig()
{
}
bool CGetNewKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPassphrase;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[passphrase] type error, needs string");
		}
	}
	return true;
}
string CGetNewKeyConfig::ListConfig() const
{
	return "";
}
string CGetNewKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getnewkey (\"passphrase\")\n";
	oss << "\n";
	oss << "Returns a new pubkey for receiving payments.  \n"
	       "If (passphrase) is specified, the key will be encrypted with (passphrase).\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"passphrase\"                   (string, optional) passphrase\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"passphrase\": \"\"             (string, optional) passphrase\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"pubkey\"             (string, required) public key\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getnewkey 123\n";
	oss << "<< f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\n";
	oss << "\n>> curl -d '{\"id\":7,\"method\":\"getnewkey\",\"jsonrpc\":\"2.0\",\"params\":{\"passphrase\":\"123\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":7,\"jsonrpc\":\"2.0\",\"result\":\"f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// encryptkey

// CEncryptKeyParam
CEncryptKeyParam::CEncryptKeyParam() {}
CEncryptKeyParam::CEncryptKeyParam(const CRPCString& strPubkey, const CRPCString& strPassphrase, const CRPCString& strOldpassphrase)
	: strPubkey(strPubkey), strPassphrase(strPassphrase), strOldpassphrase(strOldpassphrase)
{
}
Value CEncryptKeyParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));
	CheckIsValid(strPassphrase, "strPassphrase");
	ret.push_back(Pair("passphrase", std::string(strPassphrase)));
	if (strOldpassphrase.IsValid())
	{
		ret.push_back(Pair("oldpassphrase", std::string(strOldpassphrase)));
	}

	return ret;
}
CEncryptKeyParam& CEncryptKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "encryptkey");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	auto valPassphrase = find_value(obj, "passphrase");
	CheckJSONType(valPassphrase, "string", "passphrase");
	strPassphrase = valPassphrase.get_str();
	auto valOldpassphrase = find_value(obj, "oldpassphrase");
	if (!valOldpassphrase.is_null())
	{
		CheckJSONType(valOldpassphrase, "string", "oldpassphrase");
		strOldpassphrase = valOldpassphrase.get_str();
	}
	return *this;
}
string CEncryptKeyParam::Method() const
{
	return "encryptkey";
}

// CEncryptKeyResult
CEncryptKeyResult::CEncryptKeyResult() {}
CEncryptKeyResult::CEncryptKeyResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CEncryptKeyResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CEncryptKeyResult& CEncryptKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CEncryptKeyResult::Method() const
{
	return "encryptkey";
}

// CEncryptKeyConfig
CEncryptKeyConfig::CEncryptKeyConfig()
{
}
bool CEncryptKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 4)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPassphrase;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[passphrase] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[passphrase] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strOldpassphrase;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[oldpassphrase] type error, needs string");
		}
	}
	return true;
}
string CEncryptKeyConfig::ListConfig() const
{
	return "";
}
string CEncryptKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        encryptkey <\"pubkey\"> <\"passphrase\"> (\"oldpassphrase\")\n";
	oss << "\n";
	oss << "Encrypts the key assoiciated with <passphrase>.  \n"
	       "For encrypted key, changes the passphrase for (oldpassphrase) to <passphrase>\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) public key\n";
	oss << " \"passphrase\"                   (string, required) passphrase of key\n";
	oss << " \"oldpassphrase\"                (string, optional) old passphrase of key\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\",                (string, required) public key\n";
	oss << "   \"passphrase\": \"\",            (string, required) passphrase of key\n";
	oss << "   \"oldpassphrase\": \"\"          (string, optional) old passphrase of key\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) encrypt key result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli encryptkey 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882 123\n";
	oss << "<< Encrypt key successfully: 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\n";
	oss << "\n>> curl -d '{\"id\":64,\"method\":\"encryptkey\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\",\"passphrase\":\"123\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":64,\"jsonrpc\":\"2.0\",\"result\":\"Encrypt key successfully: 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-4,\"message\":\"Unknown key\"}\n";
	oss << "* {\"code\":-406,\"message\":\"The passphrase entered was incorrect.\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// lockkey

// CLockKeyParam
CLockKeyParam::CLockKeyParam() {}
CLockKeyParam::CLockKeyParam(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CLockKeyParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));

	return ret;
}
CLockKeyParam& CLockKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "lockkey");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	return *this;
}
string CLockKeyParam::Method() const
{
	return "lockkey";
}

// CLockKeyResult
CLockKeyResult::CLockKeyResult() {}
CLockKeyResult::CLockKeyResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CLockKeyResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CLockKeyResult& CLockKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CLockKeyResult::Method() const
{
	return "lockkey";
}

// CLockKeyConfig
CLockKeyConfig::CLockKeyConfig()
{
}
bool CLockKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	return true;
}
string CLockKeyConfig::ListConfig() const
{
	return "";
}
string CLockKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        lockkey <\"pubkey\">\n";
	oss << "\n";
	oss << "Removes the encryption key from memory, locking the key.\n"
	       "After calling this method, you will need to call unlockkey again.\n"
	       "before being able to call any methods which require the key to be unlocked.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) pubkey or pubkey address\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\"                 (string, required) pubkey or pubkey address\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) lock key result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli lockkey 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\n";
	oss << "<< Lock key successfully: 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"lockkey\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":\"Lock key successfully: 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-4,\"message\":\"Unknown key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to lock key\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// unlockkey

// CUnlockKeyParam
CUnlockKeyParam::CUnlockKeyParam() {}
CUnlockKeyParam::CUnlockKeyParam(const CRPCString& strPubkey, const CRPCString& strPassphrase, const CRPCInt64& nTimeout)
	: strPubkey(strPubkey), strPassphrase(strPassphrase), nTimeout(nTimeout)
{
}
Value CUnlockKeyParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));
	CheckIsValid(strPassphrase, "strPassphrase");
	ret.push_back(Pair("passphrase", std::string(strPassphrase)));
	if (nTimeout.IsValid())
	{
		ret.push_back(Pair("timeout", int64(nTimeout)));
	}

	return ret;
}
CUnlockKeyParam& CUnlockKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "unlockkey");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	auto valPassphrase = find_value(obj, "passphrase");
	CheckJSONType(valPassphrase, "string", "passphrase");
	strPassphrase = valPassphrase.get_str();
	auto valTimeout = find_value(obj, "timeout");
	if (!valTimeout.is_null())
	{
		CheckJSONType(valTimeout, "int", "timeout");
		nTimeout = valTimeout.get_int64();
	}
	return *this;
}
string CUnlockKeyParam::Method() const
{
	return "unlockkey";
}

// CUnlockKeyResult
CUnlockKeyResult::CUnlockKeyResult() {}
CUnlockKeyResult::CUnlockKeyResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CUnlockKeyResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CUnlockKeyResult& CUnlockKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CUnlockKeyResult::Method() const
{
	return "unlockkey";
}

// CUnlockKeyConfig
CUnlockKeyConfig::CUnlockKeyConfig()
{
	boost::program_options::options_description desc("CUnlockKeyConfig");

	AddOpt<int64>(desc, "t");

	AddOptions(desc);
}
bool CUnlockKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 4)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPassphrase;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[passphrase] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[passphrase] is required");
	}
	if (vm.find("t") != vm.end())
	{
		auto value = vm["t"];
		nTimeout = value.as<int64>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> nTimeout;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[timeout] type error, needs int");
			}
		}
	}
	return true;
}
string CUnlockKeyConfig::ListConfig() const
{
	return "";
}
string CUnlockKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        unlockkey <\"pubkey\"> <\"passphrase\"> (-t=timeout)\n";
	oss << "\n";
	oss << "If (timeout) > 0,stores the wallet decryption key in memory for (timeout) seconds.\nbefore \n"
	       "being able to call any methods which require the key to be locked.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) pubkey or pubkey address\n";
	oss << " \"passphrase\"                   (string, required) passphrase\n";
	oss << " -t=timeout                     (int, optional) auto unlock timeout\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\",                (string, required) pubkey or pubkey address\n";
	oss << "   \"passphrase\": \"\",            (string, required) passphrase\n";
	oss << "   \"timeout\": 0                 (int, optional) auto unlock timeout\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) unlock key result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli unlockkey d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07 1234\n";
	oss << "<< Unlock key successfully: d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\n";
	oss << "\n>> curl -d '{\"id\":13,\"method\":\"unlockkey\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\",\"passphrase\":\"1234\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":13,\"jsonrpc\":\"2.0\",\"result\":\"Unlock key successfully: d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\"}\n";
	oss << "\n>> multiverse-cli unlockkey f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9 123 10\n";
	oss << "<< Unlock key successfully: f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\n";
	oss << "\n>> curl -d '{\"id\":15,\"method\":\"unlockkey\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\",\"passphrase\":\"123\",\"timeout\":10}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":15,\"jsonrpc\":\"2.0\",\"result\":\"Unlock key successfully: f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-4,\"message\":\"Unknown key\"}\n";
	oss << "* {\"code\":-409,\"message\":\"Key is already unlocked\"}\n";
	oss << "* {\"code\":-406,\"message\":\"The passphrase entered was incorrect.\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// importprivkey

// CImportPrivKeyParam
CImportPrivKeyParam::CImportPrivKeyParam() {}
CImportPrivKeyParam::CImportPrivKeyParam(const CRPCString& strPrivkey, const CRPCString& strPassphrase)
	: strPrivkey(strPrivkey), strPassphrase(strPassphrase)
{
}
Value CImportPrivKeyParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPrivkey, "strPrivkey");
	ret.push_back(Pair("privkey", std::string(strPrivkey)));
	if (strPassphrase.IsValid())
	{
		ret.push_back(Pair("passphrase", std::string(strPassphrase)));
	}

	return ret;
}
CImportPrivKeyParam& CImportPrivKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "importprivkey");
	auto obj = v.get_obj();
	auto valPrivkey = find_value(obj, "privkey");
	CheckJSONType(valPrivkey, "string", "privkey");
	strPrivkey = valPrivkey.get_str();
	auto valPassphrase = find_value(obj, "passphrase");
	if (!valPassphrase.is_null())
	{
		CheckJSONType(valPassphrase, "string", "passphrase");
		strPassphrase = valPassphrase.get_str();
	}
	return *this;
}
string CImportPrivKeyParam::Method() const
{
	return "importprivkey";
}

// CImportPrivKeyResult
CImportPrivKeyResult::CImportPrivKeyResult() {}
CImportPrivKeyResult::CImportPrivKeyResult(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CImportPrivKeyResult::ToJSON() const
{
	CheckIsValid(strPubkey, "strPubkey");
	Value val;
	val = Value(strPubkey);
	return val;
}
CImportPrivKeyResult& CImportPrivKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "pubkey");
	strPubkey = v.get_str();
	return *this;
}
string CImportPrivKeyResult::Method() const
{
	return "importprivkey";
}

// CImportPrivKeyConfig
CImportPrivKeyConfig::CImportPrivKeyConfig()
{
	boost::program_options::options_description desc("CImportPrivKeyConfig");

	AddOpt<string>(desc, "p");

	AddOptions(desc);
}
bool CImportPrivKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPrivkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[privkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[privkey] is required");
	}
	if (vm.find("p") != vm.end())
	{
		auto value = vm["p"];
		strPassphrase = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strPassphrase;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[passphrase] type error, needs string");
			}
		}
	}
	return true;
}
string CImportPrivKeyConfig::ListConfig() const
{
	return "";
}
string CImportPrivKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        importprivkey <\"privkey\"> (-p=\"passphrase\")\n";
	oss << "\n";
	oss << "Adds a private key (as returned by dumpprivkey) to your wallet.\n"
	       "If (passphrase) is specified, the key will be encrypted with (passphrase).\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"privkey\"                      (string, required) private key\n";
	oss << " -p=\"passphrase\"                (string, optional) passphrase\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"privkey\": \"\",               (string, required) private key\n";
	oss << "   \"passphrase\": \"\"             (string, optional) passphrase\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"pubkey\"             (string, required) public key with hex number system\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli importprivkey feb51e048380c0ade1cdb60b25e9f3e05cd4507553a97faadc8a94771fcb1a5b\n";
	oss << "<< d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\n";
	oss << "\n>> curl -d '{\"id\":9,\"method\":\"importprivkey\",\"jsonrpc\":\"2.0\",\"params\":{\"privkey\":\"feb51e048380c0ade1cdb60b25e9f3e05cd4507553a97faadc8a94771fcb1a5b\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":9,\"jsonrpc\":\"2.0\",\"result\":\"d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-4,\"message\":\"Invalid private key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Already have key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to add key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sync wallet tx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// importkey

// CImportKeyParam
CImportKeyParam::CImportKeyParam() {}
CImportKeyParam::CImportKeyParam(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CImportKeyParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));

	return ret;
}
CImportKeyParam& CImportKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "importkey");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	return *this;
}
string CImportKeyParam::Method() const
{
	return "importkey";
}

// CImportKeyResult
CImportKeyResult::CImportKeyResult() {}
CImportKeyResult::CImportKeyResult(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CImportKeyResult::ToJSON() const
{
	CheckIsValid(strPubkey, "strPubkey");
	Value val;
	val = Value(strPubkey);
	return val;
}
CImportKeyResult& CImportKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "pubkey");
	strPubkey = v.get_str();
	return *this;
}
string CImportKeyResult::Method() const
{
	return "importkey";
}

// CImportKeyConfig
CImportKeyConfig::CImportKeyConfig()
{
}
bool CImportKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	return true;
}
string CImportKeyConfig::ListConfig() const
{
	return "";
}
string CImportKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        importkey <\"pubkey\">\n";
	oss << "\n";
	oss << "Reveals the serialized key corresponding to <pubkey>.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) public key data\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\"                 (string, required) public key data\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"pubkey\"             (string, required) public key with hex number system\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli importkey 071c39eccba20183ae693ada9724525b327e70a75147d5579a648ee52ce716d700000000346783653e9035de91d5ac79545e17911e5825b50d60a2e04ac8451aa83c4ae4f0ea063a5917b68256a3bd391966b30f5660e7d5d1777ae996a49f4f\n";
	oss << "<< d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\n";
	oss << "\n>> curl -d '{\"id\":14,\"method\":\"importkey\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"071c39eccba20183ae693ada9724525b327e70a75147d5579a648ee52ce716d700000000346783653e9035de91d5ac79545e17911e5825b50d60a2e04ac8451aa83c4ae4f0ea063a5917b68256a3bd391966b30f5660e7d5d1777ae996a49f4f\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":14,\"jsonrpc\":\"2.0\",\"result\":\"d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-32602,\"message\":\"Failed to verify serialized key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Already have key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to add key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sync wallet tx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// exportkey

// CExportKeyParam
CExportKeyParam::CExportKeyParam() {}
CExportKeyParam::CExportKeyParam(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CExportKeyParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));

	return ret;
}
CExportKeyParam& CExportKeyParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "exportkey");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	return *this;
}
string CExportKeyParam::Method() const
{
	return "exportkey";
}

// CExportKeyResult
CExportKeyResult::CExportKeyResult() {}
CExportKeyResult::CExportKeyResult(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CExportKeyResult::ToJSON() const
{
	CheckIsValid(strPubkey, "strPubkey");
	Value val;
	val = Value(strPubkey);
	return val;
}
CExportKeyResult& CExportKeyResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "pubkey");
	strPubkey = v.get_str();
	return *this;
}
string CExportKeyResult::Method() const
{
	return "exportkey";
}

// CExportKeyConfig
CExportKeyConfig::CExportKeyConfig()
{
}
bool CExportKeyConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	return true;
}
string CExportKeyConfig::ListConfig() const
{
	return "";
}
string CExportKeyConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        exportkey <\"pubkey\">\n";
	oss << "\n";
	oss << "Reveals the serialized key corresponding to <pubkey>.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) public key\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\"                 (string, required) public key\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"pubkey\"             (string, required) public key with binary system\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli exportkey d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\n";
	oss << "<< 071c39eccba20183ae693ada9724525b327e70a75147d5579a648ee52ce716d700000000346783653e9035de91d5ac79545e17911e5825b50d60a2e04ac8451aa83c4ae4f0ea063a5917b68256a3bd391966b30f5660e7d5d1777ae996a49f4f\n";
	oss << "\n>> curl -d '{\"id\":10,\"method\":\"exportkey\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"d716e72ce58e649a57d54751a7707e325b522497da3a69ae8301a2cbec391c07\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":10,\"jsonrpc\":\"2.0\",\"result\":\"071c39eccba20183ae693ada9724525b327e70a75147d5579a648ee52ce716d700000000346783653e9035de91d5ac79545e17911e5825b50d60a2e04ac8451aa83c4ae4f0ea063a5917b68256a3bd391966b30f5660e7d5d1777ae996a49f4f\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-4,\"message\":\"Unknown key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to export key\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// addnewtemplate

// CAddNewTemplateParam
CAddNewTemplateParam::CAddNewTemplateParam() {}
CAddNewTemplateParam::CAddNewTemplateParam(const CTemplateRequest& data)
	: data(data)
{
}
Value CAddNewTemplateParam::ToJSON() const
{
	CheckIsValid(data, "data");
	return data.ToJSON();
}
CAddNewTemplateParam& CAddNewTemplateParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "data");
	data.FromJSON(v.get_obj());
	return *this;
}
string CAddNewTemplateParam::Method() const
{
	return "addnewtemplate";
}

// CAddNewTemplateResult
CAddNewTemplateResult::CAddNewTemplateResult() {}
CAddNewTemplateResult::CAddNewTemplateResult(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CAddNewTemplateResult::ToJSON() const
{
	CheckIsValid(strAddress, "strAddress");
	Value val;
	val = Value(strAddress);
	return val;
}
CAddNewTemplateResult& CAddNewTemplateResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "address");
	strAddress = v.get_str();
	return *this;
}
string CAddNewTemplateResult::Method() const
{
	return "addnewtemplate";
}

// CAddNewTemplateConfig
CAddNewTemplateConfig::CAddNewTemplateConfig()
{
}
bool CAddNewTemplateConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 7)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> data.strType;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[type] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[type] is required");
	}
	if (data.strType == "delegate")
	{
		string strOriginDelegate;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginDelegate = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[delegate] is required");
		}
		Value valOriginDelegate;
		if (!read_string(strOriginDelegate, valOriginDelegate))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.delegate.FromJSON(valOriginDelegate);
	}
	if (data.strType == "fork")
	{
		string strOriginFork;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginFork = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[fork] is required");
		}
		Value valOriginFork;
		if (!read_string(strOriginFork, valOriginFork))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.fork.FromJSON(valOriginFork);
	}
	if (data.strType == "mint")
	{
		string strOriginMint;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginMint = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[mint] is required");
		}
		Value valOriginMint;
		if (!read_string(strOriginMint, valOriginMint))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.mint.FromJSON(valOriginMint);
	}
	if (data.strType == "multisig")
	{
		string strOriginMultisig;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginMultisig = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[multisig] is required");
		}
		Value valOriginMultisig;
		if (!read_string(strOriginMultisig, valOriginMultisig))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.multisig.FromJSON(valOriginMultisig);
	}
	if (data.strType == "weighted")
	{
		string strOriginWeighted;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginWeighted = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[weighted] is required");
		}
		Value valOriginWeighted;
		if (!read_string(strOriginWeighted, valOriginWeighted))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.weighted.FromJSON(valOriginWeighted);
	}
	return true;
}
string CAddNewTemplateConfig::ListConfig() const
{
	return "";
}
string CAddNewTemplateConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        addnewtemplate <\"type\"> <{delegate}>|<{fork}>|<{mint}>|<{multisig}>|<{weighted}>\n";
	oss << "\n";
	oss << "Returns encoded address for the given template id.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"type\"                         (string, required) template type\n";
	oss << "  (if type is \"delegate\")\n";
	oss << " {delegate}                     (object, required) a delegate template\n";
	oss << "  (if type is \"fork\")\n";
	oss << " {fork}                         (object, required) a new fork template\n";
	oss << "  (if type is \"mint\")\n";
	oss << " {mint}                         (object, required) a mint template\n";
	oss << "  (if type is \"multisig\")\n";
	oss << " {multisig}                     (object, required) a multiple sign template\n";
	oss << "  (if type is \"weighted\")\n";
	oss << " {weighted}                     (object, required) a weighted multiple sign template\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"type\": \"\",                  (string, required) template type\n";
	oss << "   (if \"type\" is \"delegate\")\n";
	oss << "   \"delegate\":                  (object, required) a delegate template\n";
	oss << "   {\n";
	oss << "     \"delegate\": \"\",            (string, required) delegate public key\n";
	oss << "     \"owner\": \"\"                (string, required) owner address\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"fork\")\n";
	oss << "   \"fork\":                      (object, required) a new fork template\n";
	oss << "   {\n";
	oss << "     \"redeem\": \"\",              (string, required) redeem address\n";
	oss << "     \"fork\": \"\"                 (string, required) fork hash\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"mint\")\n";
	oss << "   \"mint\":                      (object, required) a mint template\n";
	oss << "   {\n";
	oss << "     \"mint\": \"\",                (string, required) mint public key\n";
	oss << "     \"spent\": \"\"                (string, required) spent address\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"multisig\")\n";
	oss << "   \"multisig\":                  (object, required) a multiple sign template\n";
	oss << "   {\n";
	oss << "     \"required\": 0,             (int, required) required weight > 0\n";
	oss << "     [\n";
	oss << "       \"key\": \"\"                (string, required) public key\n";
	oss << "     ]\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"weighted\")\n";
	oss << "   \"weighted\":                  (object, required) a weighted multiple sign template\n";
	oss << "   {\n";
	oss << "     \"required\": 0,             (int, required) required weight\n";
	oss << "     [\n";
	oss << "       \"pubkey\":                (object, required) public key\n";
	oss << "       {\n";
	oss << "         \"key\": \"\",             (string, required) public key\n";
	oss << "         \"weight\": 0            (int, required) weight\n";
	oss << "       }\n";
	oss << "     ]\n";
	oss << "   }\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"address\"            (string, required) address of template\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli addnewtemplate mint '{\"mint\": \"e8e3770e774d5ad84a8ea65ed08cc7c5c30b42e045623604d5c5c6be95afb4f9\", \"spent\": \"1z6taz5dyrv2xa11pc92y0ggbrf2wf36gbtk8wjprb96qe3kqwfm3ayc1\"}'\n";
	oss << "<< 20g0b87qxcd52ceh9zmpzx0hy46pjfzdnqbkh8f4tqs4y0r6sxyzyny25\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"addnewtemplate\",\"jsonrpc\":\"2.0\",\"params\":{\"type\":\"mint\",\"mint\":{\"mint\":\"e8e3770e774d5ad84a8ea65ed08cc7c5c30b42e045623604d5c5c6be95afb4f9\",\"spent\":\"1z6taz5dyrv2xa11pc92y0ggbrf2wf36gbtk8wjprb96qe3kqwfm3ayc1\"}}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":\"20g0b87qxcd52ceh9zmpzx0hy46pjfzdnqbkh8f4tqs4y0r6sxyzyny25\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameters,failed to make template\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to add template\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing weight\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing redeem address\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing spent address\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing owner address\"}\n";
	oss << "* {\"code\":-6,\"message\":\"template type error. type: xxx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// importtemplate

// CImportTemplateParam
CImportTemplateParam::CImportTemplateParam() {}
CImportTemplateParam::CImportTemplateParam(const CRPCString& strData)
	: strData(strData)
{
}
Value CImportTemplateParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strData, "strData");
	ret.push_back(Pair("data", std::string(strData)));

	return ret;
}
CImportTemplateParam& CImportTemplateParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "importtemplate");
	auto obj = v.get_obj();
	auto valData = find_value(obj, "data");
	CheckJSONType(valData, "string", "data");
	strData = valData.get_str();
	return *this;
}
string CImportTemplateParam::Method() const
{
	return "importtemplate";
}

// CImportTemplateResult
CImportTemplateResult::CImportTemplateResult() {}
CImportTemplateResult::CImportTemplateResult(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CImportTemplateResult::ToJSON() const
{
	CheckIsValid(strAddress, "strAddress");
	Value val;
	val = Value(strAddress);
	return val;
}
CImportTemplateResult& CImportTemplateResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "address");
	strAddress = v.get_str();
	return *this;
}
string CImportTemplateResult::Method() const
{
	return "importtemplate";
}

// CImportTemplateConfig
CImportTemplateConfig::CImportTemplateConfig()
{
}
bool CImportTemplateConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strData;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[data] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[data] is required");
	}
	return true;
}
string CImportTemplateConfig::ListConfig() const
{
	return "";
}
string CImportTemplateConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        importtemplate <\"data\">\n";
	oss << "\n";
	oss << "Returns encoded address for the given template.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"data\"                         (string, required) template data\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"data\": \"\"                   (string, required) template data\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"address\"            (string, required) address of template\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli importtemplate 0100010282e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e01b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f402\n";
	oss << "<< 21w2040000000000000000000000000000000000000000000000epcek\n";
	oss << "\n>> curl -d '{\"id\":52,\"method\":\"importtemplate\",\"jsonrpc\":\"2.0\",\"params\":{\"data\":\"0100010282e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e01b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f402\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":52,\"jsonrpc\":\"2.0\",\"result\":\"21w2040000000000000000000000000000000000000000000000epcek\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameters,failed to make template\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Already have this template\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to add template\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sync wallet tx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// exporttemplate

// CExportTemplateParam
CExportTemplateParam::CExportTemplateParam() {}
CExportTemplateParam::CExportTemplateParam(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CExportTemplateParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));

	return ret;
}
CExportTemplateParam& CExportTemplateParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "exporttemplate");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	return *this;
}
string CExportTemplateParam::Method() const
{
	return "exporttemplate";
}

// CExportTemplateResult
CExportTemplateResult::CExportTemplateResult() {}
CExportTemplateResult::CExportTemplateResult(const CRPCString& strData)
	: strData(strData)
{
}
Value CExportTemplateResult::ToJSON() const
{
	CheckIsValid(strData, "strData");
	Value val;
	val = Value(strData);
	return val;
}
CExportTemplateResult& CExportTemplateResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "data");
	strData = v.get_str();
	return *this;
}
string CExportTemplateResult::Method() const
{
	return "exporttemplate";
}

// CExportTemplateConfig
CExportTemplateConfig::CExportTemplateConfig()
{
}
bool CExportTemplateConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strAddress;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[address] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[address] is required");
	}
	return true;
}
string CExportTemplateConfig::ListConfig() const
{
	return "";
}
string CExportTemplateConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        exporttemplate <\"address\">\n";
	oss << "\n";
	oss << "Returns encoded address for the given template.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"address\"                      (string, required) template address\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"address\": \"\"                (string, required) template address\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"data\"               (string, required) data of template\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli exporttemplate 2040fpytdr4k5h8tk0nferr7zb51tkccrkgqf341s6tg05q9xe6hth1m\n";
	oss << "<< 0100010282e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e01b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f402\n";
	oss << "\n>> curl -d '{\"id\":25,\"method\":\"exporttemplate\",\"jsonrpc\":\"2.0\",\"params\":{\"address\":\"2040fpytdr4k5h8tk0nferr7zb51tkccrkgqf341s6tg05q9xe6hth1m4\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":25,\"jsonrpc\":\"2.0\",\"result\":\"0100010282e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e01b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f402\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid address, should be template address\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Unkown template\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// validateaddress

// CValidateAddressParam
CValidateAddressParam::CValidateAddressParam() {}
CValidateAddressParam::CValidateAddressParam(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CValidateAddressParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));

	return ret;
}
CValidateAddressParam& CValidateAddressParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "validateaddress");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	return *this;
}
string CValidateAddressParam::Method() const
{
	return "validateaddress";
}

// CValidateAddressResult::CAddressdata
CValidateAddressResult::CAddressdata::CAddressdata() {}
CValidateAddressResult::CAddressdata::CAddressdata(const CRPCString& strAddress, const CRPCBool& fIsmine, const CRPCString& strType, const CRPCString& strPubkey, const CRPCString& strTemplate, const CTemplateResponse& templatedata)
	: strAddress(strAddress), fIsmine(fIsmine), strType(strType), strPubkey(strPubkey), strTemplate(strTemplate), templatedata(templatedata)
{
}
CValidateAddressResult::CAddressdata::CAddressdata(const CRPCType& null)
	: strAddress(null), fIsmine(null), strType(null), strPubkey(null), strTemplate(null), templatedata(null)
{
}
Value CValidateAddressResult::CAddressdata::ToJSON() const
{
	Object ret;
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));
	CheckIsValid(fIsmine, "fIsmine");
	ret.push_back(Pair("ismine", bool(fIsmine)));
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	if (strType == "pubkey")
	{
		CheckIsValid(strPubkey, "strPubkey");
		ret.push_back(Pair("pubkey", std::string(strPubkey)));
	}
	if (strType == "template")
	{
		CheckIsValid(strTemplate, "strTemplate");
		ret.push_back(Pair("template", std::string(strTemplate)));
	}
	if (fIsmine == true)
	{
		if (templatedata.IsValid())
		{
			ret.push_back(Pair("templatedata", templatedata.ToJSON()));
		}
	}

	return ret;
}
CValidateAddressResult::CAddressdata& CValidateAddressResult::CAddressdata::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CValidateAddressResult::CAddressdata");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	auto valIsmine = find_value(obj, "ismine");
	CheckJSONType(valIsmine, "bool", "ismine");
	fIsmine = valIsmine.get_bool();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	if (strType == "pubkey")
	{
		auto valPubkey = find_value(obj, "pubkey");
		CheckJSONType(valPubkey, "string", "pubkey");
		strPubkey = valPubkey.get_str();
	}
	if (strType == "template")
	{
		auto valTemplate = find_value(obj, "template");
		CheckJSONType(valTemplate, "string", "template");
		strTemplate = valTemplate.get_str();
	}
	if (fIsmine == true)
	{
		auto valTemplatedata = find_value(obj, "templatedata");
		if (!valTemplatedata.is_null())
		{
			CheckJSONType(valTemplatedata, "object", "templatedata");
			templatedata.FromJSON(valTemplatedata.get_obj());
		}
	}
	return *this;
}
bool CValidateAddressResult::CAddressdata::IsValid() const
{
	if (!strAddress.IsValid()) { return false; }
	if (!fIsmine.IsValid()) { return false; }
	if (!strType.IsValid()) { return false; }
	if (strType == "pubkey")
	{
		if (!strPubkey.IsValid()) { return false; }
	}
	if (strType == "template")
	{
		if (!strTemplate.IsValid()) { return false; }
	}
	return true;
}

// CValidateAddressResult
CValidateAddressResult::CValidateAddressResult() {}
CValidateAddressResult::CValidateAddressResult(const CRPCBool& fIsvalid, const CAddressdata& addressdata)
	: fIsvalid(fIsvalid), addressdata(addressdata)
{
}
Value CValidateAddressResult::ToJSON() const
{
	Object ret;
	CheckIsValid(fIsvalid, "fIsvalid");
	ret.push_back(Pair("isvalid", bool(fIsvalid)));
	if (fIsvalid == true)
	{
		CheckIsValid(addressdata, "addressdata");
		ret.push_back(Pair("addressdata", addressdata.ToJSON()));
	}

	return ret;
}
CValidateAddressResult& CValidateAddressResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "validateaddress");
	auto obj = v.get_obj();
	auto valIsvalid = find_value(obj, "isvalid");
	CheckJSONType(valIsvalid, "bool", "isvalid");
	fIsvalid = valIsvalid.get_bool();
	if (fIsvalid == true)
	{
		auto valAddressdata = find_value(obj, "addressdata");
		CheckJSONType(valAddressdata, "object", "addressdata");
		addressdata.FromJSON(valAddressdata.get_obj());
	}
	return *this;
}
string CValidateAddressResult::Method() const
{
	return "validateaddress";
}

// CValidateAddressConfig
CValidateAddressConfig::CValidateAddressConfig()
{
}
bool CValidateAddressConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strAddress;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[address] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[address] is required");
	}
	return true;
}
string CValidateAddressConfig::ListConfig() const
{
	return "";
}
string CValidateAddressConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        validateaddress <\"address\">\n";
	oss << "\n";
	oss << "Return information about <address>.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"address\"                      (string, required) wallet address\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"address\": \"\"                (string, required) wallet address\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"isvalid\": true|false,       (bool, required) is valid\n";
	oss << "   (if \"isvalid\" is true)\n";
	oss << "   \"addressdata\":               (object, required) address data\n";
	oss << "   {\n";
	oss << "     \"address\": \"\",             (string, required) wallet address\n";
	oss << "     \"ismine\": true|false,      (bool, required) is mine\n";
	oss << "     \"type\": \"\",                (string, required) type, pubkey or template\n";
	oss << "     (if \"type\" is \"pubkey\")\n";
	oss << "     \"pubkey\": \"\",              (string, required) public key\n";
	oss << "     (if \"type\" is \"template\")\n";
	oss << "     \"template\": \"\",            (string, required) template type name\n";
	oss << "     (if \"ismine\" is true)\n";
	oss << "     \"templatedata\":            (object, optional) template data\n";
	oss << "     {\n";
	oss << "       \"type\": \"\",              (string, required) template type\n";
	oss << "       \"hex\": \"\",               (string, required) temtplate data\n";
	oss << "       (if \"type\" is \"delegate\")\n";
	oss << "       \"delegate\":              (object, required) delegate template struct\n";
	oss << "       {\n";
	oss << "         \"delegate\": \"\",        (string, required) delegate public key\n";
	oss << "         \"owner\": \"\"            (string, required) owner address\n";
	oss << "       }\n";
	oss << "       (if \"type\" is \"fork\")\n";
	oss << "       \"fork\":                  (object, required) fork template struct\n";
	oss << "       {\n";
	oss << "         \"redeem\": \"\",          (string, required) redeem address\n";
	oss << "         \"fork\": \"\"             (string, required) fork hash\n";
	oss << "       }\n";
	oss << "       (if \"type\" is \"mint\")\n";
	oss << "       \"mint\":                  (object, required) mint template struct\n";
	oss << "       {\n";
	oss << "         \"mint\": \"\",            (string, required) mint public key\n";
	oss << "         \"spent\": \"\"            (string, required) spent address\n";
	oss << "       }\n";
	oss << "       (if \"type\" is \"multisig\")\n";
	oss << "       \"multisig\":              (object, required) multisig template struct\n";
	oss << "       {\n";
	oss << "         \"sigsrequired\": 0,     (int, required) required weight\n";
	oss << "         [\n";
	oss << "           \"key\": \"\"            (string, required) public key\n";
	oss << "         ]\n";
	oss << "       }\n";
	oss << "       (if \"type\" is \"weighted\")\n";
	oss << "       \"weighted\":              (object, required) weighted template struct\n";
	oss << "       {\n";
	oss << "         \"sigsrequired\": 0,     (int, required) required weight\n";
	oss << "         [\n";
	oss << "           \"pubkey\":            (object, required) public key\n";
	oss << "           {\n";
	oss << "             \"key\": \"\",         (string, required) public key\n";
	oss << "             \"weight\": 0        (int, required) weight\n";
	oss << "           }\n";
	oss << "         ]\n";
	oss << "       }\n";
	oss << "     }\n";
	oss << "   }\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli validateaddress 20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26\n";
	oss << "<< {\"isvalid\":true,\"addressdata\":{\"address\":\"20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26\",\"ismine\":true,\"type\":\"template\",\"template\":\"mint\"}}\n";
	oss << "\n>> curl -d '{\"id\":2,\"method\":\"validateaddress\",\"jsonrpc\":\"2.0\",\"params\":{\"address\":\"20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":2,\"jsonrpc\":\"2.0\",\"result\":{\"isvalid\":true,\"addressdata\":{\"address\":\"20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26\",\"ismine\":true,\"type\":\"template\",\"template\":\"mint\"}}}\n";
	oss << "\n>> multiverse-cli validateaddress 123\n";
	oss << "<< {\"isvalid\":false}\n";
	oss << "\n>> curl -d '{\"id\":3,\"method\":\"validateaddress\",\"jsonrpc\":\"2.0\",\"params\":{\"address\":\"123\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":3,\"jsonrpc\":\"2.0\",\"result\":{\"isvalid\":false}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// resyncwallet

// CResyncWalletParam
CResyncWalletParam::CResyncWalletParam() {}
CResyncWalletParam::CResyncWalletParam(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CResyncWalletParam::ToJSON() const
{
	Object ret;
	if (strAddress.IsValid())
	{
		ret.push_back(Pair("address", std::string(strAddress)));
	}

	return ret;
}
CResyncWalletParam& CResyncWalletParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "resyncwallet");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	if (!valAddress.is_null())
	{
		CheckJSONType(valAddress, "string", "address");
		strAddress = valAddress.get_str();
	}
	return *this;
}
string CResyncWalletParam::Method() const
{
	return "resyncwallet";
}

// CResyncWalletResult
CResyncWalletResult::CResyncWalletResult() {}
CResyncWalletResult::CResyncWalletResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CResyncWalletResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CResyncWalletResult& CResyncWalletResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CResyncWalletResult::Method() const
{
	return "resyncwallet";
}

// CResyncWalletConfig
CResyncWalletConfig::CResyncWalletConfig()
{
}
bool CResyncWalletConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strAddress;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[address] type error, needs string");
		}
	}
	return true;
}
string CResyncWalletConfig::ListConfig() const
{
	return "";
}
string CResyncWalletConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        resyncwallet (\"address\")\n";
	oss << "\n";
	oss << "If (address) is not specified, resync wallet's tx for each address.\n"
	       "If (address) is specified, resync wallet's tx for the address.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"address\"                      (string, optional) tx address\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"address\": \"\"                (string, optional) tx address\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) resync wallet result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli resyncwallet\n";
	oss << "<< Resync wallet successfully.\n";
	oss << "\n>> curl -d '{\"id\":38,\"method\":\"resyncwallet\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":38,\"jsonrpc\":\"2.0\",\"result\":\"Resync wallet successfully.\"}\n";
	oss << "\n>> multiverse-cli resyncwallet 1gbma6s21t4bcwymqz6h1dn1t7qy45019b1t00ywfyqymbvp90mqc1wmq\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid address\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to resync wallet tx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getbalance

// CGetBalanceParam
CGetBalanceParam::CGetBalanceParam() {}
CGetBalanceParam::CGetBalanceParam(const CRPCString& strFork, const CRPCString& strAddress)
	: strFork(strFork), strAddress(strAddress)
{
}
Value CGetBalanceParam::ToJSON() const
{
	Object ret;
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}
	if (strAddress.IsValid())
	{
		ret.push_back(Pair("address", std::string(strAddress)));
	}

	return ret;
}
CGetBalanceParam& CGetBalanceParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getbalance");
	auto obj = v.get_obj();
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	auto valAddress = find_value(obj, "address");
	if (!valAddress.is_null())
	{
		CheckJSONType(valAddress, "string", "address");
		strAddress = valAddress.get_str();
	}
	return *this;
}
string CGetBalanceParam::Method() const
{
	return "getbalance";
}

// CGetBalanceResult::CBalance
CGetBalanceResult::CBalance::CBalance() {}
CGetBalanceResult::CBalance::CBalance(const CRPCString& strAddress, const CRPCDouble& fAvail, const CRPCDouble& fLocked, const CRPCDouble& fUnconfirmed)
	: strAddress(strAddress), fAvail(fAvail), fLocked(fLocked), fUnconfirmed(fUnconfirmed)
{
}
CGetBalanceResult::CBalance::CBalance(const CRPCType& null)
	: strAddress(null), fAvail(null), fLocked(null), fUnconfirmed(null)
{
}
Value CGetBalanceResult::CBalance::ToJSON() const
{
	Object ret;
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));
	CheckIsValid(fAvail, "fAvail");
	ret.push_back(Pair("avail", double(fAvail)));
	CheckIsValid(fLocked, "fLocked");
	ret.push_back(Pair("locked", double(fLocked)));
	CheckIsValid(fUnconfirmed, "fUnconfirmed");
	ret.push_back(Pair("unconfirmed", double(fUnconfirmed)));

	return ret;
}
CGetBalanceResult::CBalance& CGetBalanceResult::CBalance::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CGetBalanceResult::CBalance");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	auto valAvail = find_value(obj, "avail");
	CheckJSONType(valAvail, "double", "avail");
	fAvail = valAvail.get_real();
	auto valLocked = find_value(obj, "locked");
	CheckJSONType(valLocked, "double", "locked");
	fLocked = valLocked.get_real();
	auto valUnconfirmed = find_value(obj, "unconfirmed");
	CheckJSONType(valUnconfirmed, "double", "unconfirmed");
	fUnconfirmed = valUnconfirmed.get_real();
	return *this;
}
bool CGetBalanceResult::CBalance::IsValid() const
{
	if (!strAddress.IsValid()) { return false; }
	if (!fAvail.IsValid()) { return false; }
	if (!fLocked.IsValid()) { return false; }
	if (!fUnconfirmed.IsValid()) { return false; }
	return true;
}

// CGetBalanceResult
CGetBalanceResult::CGetBalanceResult() {}
CGetBalanceResult::CGetBalanceResult(const CRPCVector<CBalance>& vecBalance)
	: vecBalance(vecBalance)
{
}
Value CGetBalanceResult::ToJSON() const
{
	Array ret;
	for (auto& v : vecBalance)
	{
		ret.push_back(v.ToJSON());
	}
	return ret;
}
CGetBalanceResult& CGetBalanceResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "array", "balance");
	auto vecBalanceArray = v.get_array();
	for (auto& v : vecBalanceArray)
	{
		vecBalance.push_back(CRPCVector<CBalance>::value_type().FromJSON(v));
	}
	return *this;
}
string CGetBalanceResult::Method() const
{
	return "getbalance";
}

// CGetBalanceConfig
CGetBalanceConfig::CGetBalanceConfig()
{
	boost::program_options::options_description desc("CGetBalanceConfig");

	AddOpt<string>(desc, "f");
	AddOpt<string>(desc, "a");

	AddOptions(desc);
}
bool CGetBalanceConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	if (vm.find("a") != vm.end())
	{
		auto value = vm["a"];
		strAddress = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strAddress;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[address] type error, needs string");
			}
		}
	}
	return true;
}
string CGetBalanceConfig::ListConfig() const
{
	return "";
}
string CGetBalanceConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getbalance (-f=\"fork\") (-a=\"address\")\n";
	oss << "\n";
	oss << "Get balance of address.\n"
	       "If (address) is not specified, returns the balance for wallet's each address.\n"
	       "If (address) is specified, returns the balance in the address.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << " -a=\"address\"                   (string, optional) wallet address\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"fork\": \"\",                  (string, optional) fork hash\n";
	oss << "   \"address\": \"\"                (string, optional) wallet address\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << "   [\n";
	oss << "     \"balance\":                 (object, required) balance info\n";
	oss << "     {\n";
	oss << "       \"address\": \"\",           (string, required) wallet address\n";
	oss << "       \"avail\": 0.0,            (double, required) balance available amount\n";
	oss << "       \"locked\": 0.0,           (double, required) locked amount\n";
	oss << "       \"unconfirmed\": 0.0       (double, required) unconfirmed amount\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getbalance\n";
	oss << "<< [{\"address\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"avail\":30.00000000,\"locked\":0.00000000,\"unconfirmed\":0.00000000}]\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"getbalance\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":[{\"address\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"avail\":30.00000000,\"locked\":0.00000000,\"unconfirmed\":0.00000000}]}\n";
	oss << "\n>> multiverse-cli getbalance -a=20g0944xkyk8ybcmzhpv86vb5777jn1sfrdf3svzqn9phxftqth8116bm\n";
	oss << "<< [{\"address\":\"20g0944xkyk8ybcmzhpv86vb5777jn1sfrdf3svzqn9phxftqth8116bm\",\"avail\":58.99990000,\"locked\":0.00000000,\"unconfirmed\":13.99990000}]\n";
	oss << "\n>> curl -d '{\"id\":20,\"method\":\"getbalance\",\"jsonrpc\":\"2.0\",\"params\":{\"address\":\"20g0944xkyk8ybcmzhpv86vb5777jn1sfrdf3svzqn9phxftqth8116bm\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":20,\"jsonrpc\":\"2.0\",\"result\":[{\"address\":\"20g0944xkyk8ybcmzhpv86vb5777jn1sfrdf3svzqn9phxftqth8116bm\",\"avail\":58.99990000,\"locked\":0.00000000,\"unconfirmed\":13.99990000}]}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// listtransaction

// CListTransactionParam
CListTransactionParam::CListTransactionParam() {}
CListTransactionParam::CListTransactionParam(const CRPCUint64& nCount, const CRPCInt64& nOffset)
	: nCount(nCount), nOffset(nOffset)
{
}
Value CListTransactionParam::ToJSON() const
{
	Object ret;
	if (nCount.IsValid())
	{
		ret.push_back(Pair("count", uint64(nCount)));
	}
	if (nOffset.IsValid())
	{
		ret.push_back(Pair("offset", int64(nOffset)));
	}

	return ret;
}
CListTransactionParam& CListTransactionParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "listtransaction");
	auto obj = v.get_obj();
	auto valCount = find_value(obj, "count");
	if (!valCount.is_null())
	{
		CheckJSONType(valCount, "uint", "count");
		nCount = valCount.get_uint64();
	}
	auto valOffset = find_value(obj, "offset");
	if (!valOffset.is_null())
	{
		CheckJSONType(valOffset, "int", "offset");
		nOffset = valOffset.get_int64();
	}
	return *this;
}
string CListTransactionParam::Method() const
{
	return "listtransaction";
}

// CListTransactionResult
CListTransactionResult::CListTransactionResult() {}
CListTransactionResult::CListTransactionResult(const CRPCVector<CWalletTxData>& vecTransaction)
	: vecTransaction(vecTransaction)
{
}
Value CListTransactionResult::ToJSON() const
{
	Array ret;
	for (auto& v : vecTransaction)
	{
		ret.push_back(v.ToJSON());
	}
	return ret;
}
CListTransactionResult& CListTransactionResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "array", "transaction");
	auto vecTransactionArray = v.get_array();
	for (auto& v : vecTransactionArray)
	{
		vecTransaction.push_back(CRPCVector<CWalletTxData>::value_type().FromJSON(v));
	}
	return *this;
}
string CListTransactionResult::Method() const
{
	return "listtransaction";
}

// CListTransactionConfig
CListTransactionConfig::CListTransactionConfig()
{
	boost::program_options::options_description desc("CListTransactionConfig");

	AddOpt<uint64>(desc, "n");
	AddOpt<int64>(desc, "o");

	AddOptions(desc);
}
bool CListTransactionConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (vm.find("n") != vm.end())
	{
		auto value = vm["n"];
		nCount = value.as<uint64>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> nCount;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[count] type error, needs uint");
			}
		}
	}
	if (vm.find("o") != vm.end())
	{
		auto value = vm["o"];
		nOffset = value.as<int64>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> nOffset;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[offset] type error, needs int");
			}
		}
	}
	return true;
}
string CListTransactionConfig::ListConfig() const
{
	return "";
}
string CListTransactionConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        listtransaction (-n=count) (-o=offset)\n";
	oss << "\n";
	oss << "If (offset) < 0,returns last (count) transactions,\n"
	       "If (offset) >= 0,returns up to (count) most recent transactions skipping the first \n"
	       "(offset) transactions.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " -n=count                       (uint, optional) transaction count. If not set, \n"
	       "                                return 10 tx\n";
	oss << " -o=offset                      (int, optional) query offset. If not set, from 0\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"count\": 0,                  (uint, optional) transaction count. If not set, \n"
	       "                                return 10 tx\n";
	oss << "   \"offset\": 0                  (int, optional) query offset. If not set, from 0\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << "   [\n";
	oss << "     \"transaction\":             (object, required) wallet transaction data\n";
	oss << "     {\n";
	oss << "       \"txid\": \"\",              (string, required) transaction hash\n";
	oss << "       \"fork\": \"\",              (string, required) fork hash\n";
	oss << "       \"type\": \"\",              (string, required) transaction type\n";
	oss << "       \"send\": true|false,      (bool, required) is from me\n";
	oss << "       \"to\": \"\",                (string, required) to address\n";
	oss << "       \"amount\": 0.0,           (double, required) transaction amount\n";
	oss << "       \"fee\": 0.0,              (double, required) transaction fee\n";
	oss << "       \"lockuntil\": 0,          (uint, required) lockuntil\n";
	oss << "       \"blockheight\": 0,        (int, optional) block height\n";
	oss << "       \"from\": \"\"               (string, optional) from address\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli listtransaction\n";
	oss << "<< [{\"txid\":\"4a8e6035b575699cdb25d45beadd49f18fb1303f57ec55493139e65d811e74ff\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"blockheight\":31296,\"type\":\"work\",\"send\":false,\"to\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"amount\":15.00000000,\"fee\":0.00000000,\"lockuntil\":0},{\"txid\":\"0aa6954236382a6c1c46cce7fa3165b4d1718f5e03ca67cd5fe831616a9000da\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"blockheight\":31297,\"type\":\"work\",\"send\":false,\"to\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"amount\":15.00000000,\"fee\":0.00000000,\"lockuntil\":0}]\n";
	oss << "\n>> curl -d '{\"id\":2,\"method\":\"listtransaction\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":2,\"jsonrpc\":\"2.0\",\"result\":[{\"txid\":\"4a8e6035b575699cdb25d45beadd49f18fb1303f57ec55493139e65d811e74ff\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"blockheight\":31296,\"type\":\"work\",\"send\":false,\"to\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"amount\":15.00000000,\"fee\":0.00000000,\"lockuntil\":0},{\"txid\":\"0aa6954236382a6c1c46cce7fa3165b4d1718f5e03ca67cd5fe831616a9000da\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"blockheight\":31297,\"type\":\"work\",\"send\":false,\"to\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"amount\":15.00000000,\"fee\":0.00000000,\"lockuntil\":0}]}\n";
	oss << "\n>> multiverse-cli listtransaction 1 -1\n";
	oss << "<< [{\"txid\":\"5a1b7bf5e32a77ecb3c53782a8e06f2b12bdcb73b677d6f89b6f82f85f14373a\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"blockheight\":32086,\"type\":\"work\",\"send\":false,\"to\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"amount\":15.00000000,\"fee\":0.00000000,\"lockuntil\":0}]\n";
	oss << "\n>> curl -d '{\"id\":0,\"method\":\"listtransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"count\":1,\"offset\":-1}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":0,\"jsonrpc\":\"2.0\",\"result\":[{\"txid\":\"5a1b7bf5e32a77ecb3c53782a8e06f2b12bdcb73b677d6f89b6f82f85f14373a\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"blockheight\":32086,\"type\":\"work\",\"send\":false,\"to\":\"20g098nza351f53wppg0kfnsbxqf80h3x8fwp9vdmc98fbrgbv6mtjagy\",\"amount\":15.00000000,\"fee\":0.00000000,\"lockuntil\":0}]}\n";
	oss << "\n>> listtransaction -n=1 -o=-1\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Negative, zero or out of range count\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to list transactions\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// sendfrom

// CSendFromParam
CSendFromParam::CSendFromParam() {}
CSendFromParam::CSendFromParam(const CRPCString& strFrom, const CRPCString& strTo, const CRPCDouble& fAmount, const CRPCDouble& fTxfee, const CRPCString& strFork)
	: strFrom(strFrom), strTo(strTo), fAmount(fAmount), fTxfee(fTxfee), strFork(strFork)
{
}
Value CSendFromParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strFrom, "strFrom");
	ret.push_back(Pair("from", std::string(strFrom)));
	CheckIsValid(strTo, "strTo");
	ret.push_back(Pair("to", std::string(strTo)));
	CheckIsValid(fAmount, "fAmount");
	ret.push_back(Pair("amount", double(fAmount)));
	if (fTxfee.IsValid())
	{
		ret.push_back(Pair("txfee", double(fTxfee)));
	}
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}

	return ret;
}
CSendFromParam& CSendFromParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "sendfrom");
	auto obj = v.get_obj();
	auto valFrom = find_value(obj, "from");
	CheckJSONType(valFrom, "string", "from");
	strFrom = valFrom.get_str();
	auto valTo = find_value(obj, "to");
	CheckJSONType(valTo, "string", "to");
	strTo = valTo.get_str();
	auto valAmount = find_value(obj, "amount");
	CheckJSONType(valAmount, "double", "amount");
	fAmount = valAmount.get_real();
	auto valTxfee = find_value(obj, "txfee");
	if (!valTxfee.is_null())
	{
		CheckJSONType(valTxfee, "double", "txfee");
		fTxfee = valTxfee.get_real();
	}
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	return *this;
}
string CSendFromParam::Method() const
{
	return "sendfrom";
}

// CSendFromResult
CSendFromResult::CSendFromResult() {}
CSendFromResult::CSendFromResult(const CRPCString& strTransaction)
	: strTransaction(strTransaction)
{
}
Value CSendFromResult::ToJSON() const
{
	CheckIsValid(strTransaction, "strTransaction");
	Value val;
	val = Value(strTransaction);
	return val;
}
CSendFromResult& CSendFromResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "transaction");
	strTransaction = v.get_str();
	return *this;
}
string CSendFromResult::Method() const
{
	return "sendfrom";
}

// CSendFromConfig
CSendFromConfig::CSendFromConfig()
{
	boost::program_options::options_description desc("CSendFromConfig");

	AddOpt<string>(desc, "f");

	AddOptions(desc);
}
bool CSendFromConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 6)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strFrom;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[from] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[from] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTo;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[to] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[to] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> fAmount;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[amount] type error, needs double");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[amount] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> fTxfee;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txfee] type error, needs double");
		}
	}
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	return true;
}
string CSendFromConfig::ListConfig() const
{
	return "";
}
string CSendFromConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        sendfrom <\"from\"> <\"to\"> <$amount$> ($txfee$) (-f=\"fork\")\n";
	oss << "\n";
	oss << "<amount> and <txfee> are real and rounded to the nearest 0.000001\n"
	       "Returns transaction id\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"from\"                         (string, required) from address\n";
	oss << " \"to\"                           (string, required) to address\n";
	oss << " $amount$                       (double, required) amount\n";
	oss << " $txfee$                        (double, optional) transaction fee\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"from\": \"\",                  (string, required) from address\n";
	oss << "   \"to\": \"\",                    (string, required) to address\n";
	oss << "   \"amount\": 0.0,               (double, required) amount\n";
	oss << "   \"txfee\": 0.0,                (double, optional) transaction fee\n";
	oss << "   \"fork\": \"\"                   (string, optional) fork hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"transaction\"        (string, required) transaction hash\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli sendfrom 20g0944xkyk8ybcmzhpv86vb5777jn1sfrdf3svzqn9phxftqth8116bm 1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q 1\n";
	oss << "<< 01a9f3bb967f24396293903c856e99896a514756a220266afa347a8b8c7f0038\n";
	oss << "\n>> curl -d '{\"id\":18,\"method\":\"sendfrom\",\"jsonrpc\":\"2.0\",\"params\":{\"from\":\"20g0944xkyk8ybcmzhpv86vb5777jn1sfrdf3svzqn9phxftqth8116bm\",\"to\":\"1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q\",\"amount\":1.00000000}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":18,\"jsonrpc\":\"2.0\",\"result\":\"01a9f3bb967f24396293903c856e99896a514756a220266afa347a8b8c7f0038\"}\n";
	oss << "\n>> multiverse-cli sendfrom 20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26 1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q 1 0.1 -f=a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\n";
	oss << "<< 8f92969642024234481e104481f36145736b465ead2d52a6657cf38bd52bdf59\n";
	oss << "\n>> curl -d '{\"id\":53,\"method\":\"sendfrom\",\"jsonrpc\":\"2.0\",\"params\":{\"from\":\"20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26\",\"to\":\"1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q\",\"amount\":1.00000000,\"txfee\":0.10000000,\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":53,\"jsonrpc\":\"2.0\",\"result\":\"8f92969642024234481e104481f36145736b465ead2d52a6657cf38bd52bdf59\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid address\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to create transaction\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sign transaction\"}\n";
	oss << "* {\"code\":-401,\"message\":\"The signature is not completed\"}\n";
	oss << "* {\"code\":-10,\"message\":\"Tx rejected : xxx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// createtransaction

// CCreateTransactionParam
CCreateTransactionParam::CCreateTransactionParam() {}
CCreateTransactionParam::CCreateTransactionParam(const CRPCString& strFrom, const CRPCString& strTo, const CRPCDouble& fAmount, const CRPCDouble& fTxfee, const CRPCString& strFork, const CRPCString& strData)
	: strFrom(strFrom), strTo(strTo), fAmount(fAmount), fTxfee(fTxfee), strFork(strFork), strData(strData)
{
}
Value CCreateTransactionParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strFrom, "strFrom");
	ret.push_back(Pair("from", std::string(strFrom)));
	CheckIsValid(strTo, "strTo");
	ret.push_back(Pair("to", std::string(strTo)));
	CheckIsValid(fAmount, "fAmount");
	ret.push_back(Pair("amount", double(fAmount)));
	if (fTxfee.IsValid())
	{
		ret.push_back(Pair("txfee", double(fTxfee)));
	}
	if (strFork.IsValid())
	{
		ret.push_back(Pair("fork", std::string(strFork)));
	}
	if (strData.IsValid())
	{
		ret.push_back(Pair("data", std::string(strData)));
	}

	return ret;
}
CCreateTransactionParam& CCreateTransactionParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "createtransaction");
	auto obj = v.get_obj();
	auto valFrom = find_value(obj, "from");
	CheckJSONType(valFrom, "string", "from");
	strFrom = valFrom.get_str();
	auto valTo = find_value(obj, "to");
	CheckJSONType(valTo, "string", "to");
	strTo = valTo.get_str();
	auto valAmount = find_value(obj, "amount");
	CheckJSONType(valAmount, "double", "amount");
	fAmount = valAmount.get_real();
	auto valTxfee = find_value(obj, "txfee");
	if (!valTxfee.is_null())
	{
		CheckJSONType(valTxfee, "double", "txfee");
		fTxfee = valTxfee.get_real();
	}
	auto valFork = find_value(obj, "fork");
	if (!valFork.is_null())
	{
		CheckJSONType(valFork, "string", "fork");
		strFork = valFork.get_str();
	}
	auto valData = find_value(obj, "data");
	if (!valData.is_null())
	{
		CheckJSONType(valData, "string", "data");
		strData = valData.get_str();
	}
	return *this;
}
string CCreateTransactionParam::Method() const
{
	return "createtransaction";
}

// CCreateTransactionResult
CCreateTransactionResult::CCreateTransactionResult() {}
CCreateTransactionResult::CCreateTransactionResult(const CRPCString& strTransaction)
	: strTransaction(strTransaction)
{
}
Value CCreateTransactionResult::ToJSON() const
{
	CheckIsValid(strTransaction, "strTransaction");
	Value val;
	val = Value(strTransaction);
	return val;
}
CCreateTransactionResult& CCreateTransactionResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "transaction");
	strTransaction = v.get_str();
	return *this;
}
string CCreateTransactionResult::Method() const
{
	return "createtransaction";
}

// CCreateTransactionConfig
CCreateTransactionConfig::CCreateTransactionConfig()
{
	boost::program_options::options_description desc("CCreateTransactionConfig");

	AddOpt<string>(desc, "f");
	AddOpt<string>(desc, "d");

	AddOptions(desc);
}
bool CCreateTransactionConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 7)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strFrom;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[from] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[from] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTo;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[to] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[to] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> fAmount;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[amount] type error, needs double");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[amount] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> fTxfee;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txfee] type error, needs double");
		}
	}
	if (vm.find("f") != vm.end())
	{
		auto value = vm["f"];
		strFork = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strFork;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[fork] type error, needs string");
			}
		}
	}
	if (vm.find("d") != vm.end())
	{
		auto value = vm["d"];
		strData = value.as<string>();
	}
	else
	{
		if (next(it, 1) != vecCommand.end())
		{
			istringstream iss(*++it);
			iss >> strData;
			if (!iss.eof() || iss.fail())
			{
				throw CRPCException(RPC_PARSE_ERROR, "[data] type error, needs string");
			}
		}
	}
	return true;
}
string CCreateTransactionConfig::ListConfig() const
{
	return "";
}
string CCreateTransactionConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        createtransaction <\"from\"> <\"to\"> <$amount$> ($txfee$) (-f=\"fork\") (-d=\"data\")\n";
	oss << "\n";
	oss << "<amount> and <txfee> are real and rounded to the nearest 0.000001.\n"
	       "Returns serialized tx.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"from\"                         (string, required) from address\n";
	oss << " \"to\"                           (string, required) to address\n";
	oss << " $amount$                       (double, required) amount\n";
	oss << " $txfee$                        (double, optional) transaction fee\n";
	oss << " -f=\"fork\"                      (string, optional) fork hash\n";
	oss << " -d=\"data\"                      (string, optional) output data\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"from\": \"\",                  (string, required) from address\n";
	oss << "   \"to\": \"\",                    (string, required) to address\n";
	oss << "   \"amount\": 0.0,               (double, required) amount\n";
	oss << "   \"txfee\": 0.0,                (double, optional) transaction fee\n";
	oss << "   \"fork\": \"\",                  (string, optional) fork hash\n";
	oss << "   \"data\": \"\"                   (string, optional) output data\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"transaction\"        (string, required) transaction data\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli createtransaction 20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26 1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q 1 0.1 -f=a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0 -d=12345\n";
	oss << "<< 01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a08601000000000002123400\n";
	oss << "\n>> curl -d '{\"id\":59,\"method\":\"createtransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"from\":\"20g0753dp5b817d7v0hbag6a4neetzfdgbcyt2pkx93hrzn97epzbyn26\",\"to\":\"1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q\",\"amount\":1.00000000,\"txfee\":0.10000000,\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"data\":\"12345\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":59,\"jsonrpc\":\"2.0\",\"result\":\"01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a08601000000000002123400\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid address\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to create transaction\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// signtransaction

// CSignTransactionParam
CSignTransactionParam::CSignTransactionParam() {}
CSignTransactionParam::CSignTransactionParam(const CRPCString& strTxdata)
	: strTxdata(strTxdata)
{
}
Value CSignTransactionParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxdata, "strTxdata");
	ret.push_back(Pair("txdata", std::string(strTxdata)));

	return ret;
}
CSignTransactionParam& CSignTransactionParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "signtransaction");
	auto obj = v.get_obj();
	auto valTxdata = find_value(obj, "txdata");
	CheckJSONType(valTxdata, "string", "txdata");
	strTxdata = valTxdata.get_str();
	return *this;
}
string CSignTransactionParam::Method() const
{
	return "signtransaction";
}

// CSignTransactionResult
CSignTransactionResult::CSignTransactionResult() {}
CSignTransactionResult::CSignTransactionResult(const CRPCString& strHex, const CRPCBool& fComplete)
	: strHex(strHex), fComplete(fComplete)
{
}
Value CSignTransactionResult::ToJSON() const
{
	Object ret;
	CheckIsValid(strHex, "strHex");
	ret.push_back(Pair("hex", std::string(strHex)));
	CheckIsValid(fComplete, "fComplete");
	ret.push_back(Pair("complete", bool(fComplete)));

	return ret;
}
CSignTransactionResult& CSignTransactionResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "signtransaction");
	auto obj = v.get_obj();
	auto valHex = find_value(obj, "hex");
	CheckJSONType(valHex, "string", "hex");
	strHex = valHex.get_str();
	auto valComplete = find_value(obj, "complete");
	CheckJSONType(valComplete, "bool", "complete");
	fComplete = valComplete.get_bool();
	return *this;
}
string CSignTransactionResult::Method() const
{
	return "signtransaction";
}

// CSignTransactionConfig
CSignTransactionConfig::CSignTransactionConfig()
{
}
bool CSignTransactionConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTxdata;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txdata] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[txdata] is required");
	}
	return true;
}
string CSignTransactionConfig::ListConfig() const
{
	return "";
}
string CSignTransactionConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        signtransaction <\"txdata\">\n";
	oss << "\n";
	oss << "Returns json object with keys:\n"
	       "  hex : raw transaction with signature(s) (hex-encoded string)\n"
	       "  complete : true if transaction has a complete set of signature (false if not)\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"txdata\"                       (string, required) transaction data(hex string)\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"txdata\": \"\"                 (string, required) transaction data(hex string)\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"hex\": \"\",                   (string, required) hex of transaction data\n";
	oss << "   \"complete\": true|false       (bool, required) transaction completed or not\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli signtransaction 01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a08601000000000002123400\n";
	oss << "<< {\"hex\":\"01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a0860100000000000212348182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e0182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052ed494d90cd96c252446b4a10459fea8c06186154b2bee2ce2182556e9ba40e7e69ddae2501862e4251bba2abf11c90d6f1fd0dec48a1419e81bb8c7d922cf3e03\",\"complete\":true}\n";
	oss << "\n>> curl -d '{\"id\":62,\"method\":\"signtransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"txdata\":\"01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a08601000000000002123400\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":62,\"jsonrpc\":\"2.0\",\"result\":{\"hex\":\"01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a0860100000000000212348182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e0182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052ed494d90cd96c252446b4a10459fea8c06186154b2bee2ce2182556e9ba40e7e69ddae2501862e4251bba2abf11c90d6f1fd0dec48a1419e81bb8c7d922cf3e03\",\"complete\":true}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-8,\"message\":\"TX decode failed\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sign transaction\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// signmessage

// CSignMessageParam
CSignMessageParam::CSignMessageParam() {}
CSignMessageParam::CSignMessageParam(const CRPCString& strPubkey, const CRPCString& strMessage)
	: strPubkey(strPubkey), strMessage(strMessage)
{
}
Value CSignMessageParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));
	CheckIsValid(strMessage, "strMessage");
	ret.push_back(Pair("message", std::string(strMessage)));

	return ret;
}
CSignMessageParam& CSignMessageParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "signmessage");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	auto valMessage = find_value(obj, "message");
	CheckJSONType(valMessage, "string", "message");
	strMessage = valMessage.get_str();
	return *this;
}
string CSignMessageParam::Method() const
{
	return "signmessage";
}

// CSignMessageResult
CSignMessageResult::CSignMessageResult() {}
CSignMessageResult::CSignMessageResult(const CRPCString& strSignature)
	: strSignature(strSignature)
{
}
Value CSignMessageResult::ToJSON() const
{
	CheckIsValid(strSignature, "strSignature");
	Value val;
	val = Value(strSignature);
	return val;
}
CSignMessageResult& CSignMessageResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "signature");
	strSignature = v.get_str();
	return *this;
}
string CSignMessageResult::Method() const
{
	return "signmessage";
}

// CSignMessageConfig
CSignMessageConfig::CSignMessageConfig()
{
}
bool CSignMessageConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 3)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strMessage;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[message] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[message] is required");
	}
	return true;
}
string CSignMessageConfig::ListConfig() const
{
	return "";
}
string CSignMessageConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        signmessage <\"pubkey\"> <\"message\">\n";
	oss << "\n";
	oss << "Sign a message with the private key of an pubkey\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) public key\n";
	oss << " \"message\"                      (string, required) message to be signed\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\",                (string, required) public key\n";
	oss << "   \"message\": \"\"                (string, required) message to be signed\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"signature\"          (string, required) signature of message\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli signmessage 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882 123456\n";
	oss << "<< 045977f8c07e6d846d6055357f36a70c16c071cb85115e3ffb498e171a9ac3f4aed1292203a0c8e42c4becafad3ced0d9874abd2a8b788fda9f07099a1e71707\n";
	oss << "\n>> curl -d '{\"id\":4,\"method\":\"signmessage\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\",\"message\":\"123456\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":4,\"jsonrpc\":\"2.0\",\"result\":\"045977f8c07e6d846d6055357f36a70c16c071cb85115e3ffb498e171a9ac3f4aed1292203a0c8e42c4becafad3ced0d9874abd2a8b788fda9f07099a1e71707\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-4,\"message\":\"Unknown key\"}\n";
	oss << "* {\"code\":-405,\"message\":\"Key is locked\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sign message\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// listaddress

// CListAddressParam
CListAddressParam::CListAddressParam() {}
Value CListAddressParam::ToJSON() const
{
	Object ret;

	return ret;
}
CListAddressParam& CListAddressParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "listaddress");
	auto obj = v.get_obj();
	return *this;
}
string CListAddressParam::Method() const
{
	return "listaddress";
}

// CListAddressResult::CAddressdata
CListAddressResult::CAddressdata::CAddressdata() {}
CListAddressResult::CAddressdata::CAddressdata(const CRPCString& strType, const CTemplatePubKey& pubkey, const CRPCString& strTemplate, const CTemplateResponse& templatedata)
	: strType(strType), pubkey(pubkey), strTemplate(strTemplate), templatedata(templatedata)
{
}
CListAddressResult::CAddressdata::CAddressdata(const CRPCType& null)
	: strType(null), pubkey(null), strTemplate(null), templatedata(null)
{
}
Value CListAddressResult::CAddressdata::ToJSON() const
{
	Object ret;
	CheckIsValid(strType, "strType");
	ret.push_back(Pair("type", std::string(strType)));
	if (strType == "pubkey")
	{
		CheckIsValid(pubkey, "pubkey");
		ret.push_back(Pair("pubkey", pubkey.ToJSON()));
	}
	if (strType == "template")
	{
		CheckIsValid(strTemplate, "strTemplate");
		ret.push_back(Pair("template", std::string(strTemplate)));
	}
	if (strType == "template")
	{
		if (templatedata.IsValid())
		{
			ret.push_back(Pair("templatedata", templatedata.ToJSON()));
		}
	}

	return ret;
}
CListAddressResult::CAddressdata& CListAddressResult::CAddressdata::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CListAddressResult::CAddressdata");
	auto obj = v.get_obj();
	auto valType = find_value(obj, "type");
	CheckJSONType(valType, "string", "type");
	strType = valType.get_str();
	if (strType == "pubkey")
	{
		auto valPubkey = find_value(obj, "pubkey");
		CheckJSONType(valPubkey, "object", "pubkey");
		pubkey.FromJSON(valPubkey.get_obj());
	}
	if (strType == "template")
	{
		auto valTemplate = find_value(obj, "template");
		CheckJSONType(valTemplate, "string", "template");
		strTemplate = valTemplate.get_str();
	}
	if (strType == "template")
	{
		auto valTemplatedata = find_value(obj, "templatedata");
		if (!valTemplatedata.is_null())
		{
			CheckJSONType(valTemplatedata, "object", "templatedata");
			templatedata.FromJSON(valTemplatedata.get_obj());
		}
	}
	return *this;
}
bool CListAddressResult::CAddressdata::IsValid() const
{
	if (!strType.IsValid()) { return false; }
	if (strType == "pubkey")
	{
		if (!pubkey.IsValid()) { return false; }
	}
	if (strType == "template")
	{
		if (!strTemplate.IsValid()) { return false; }
	}
	return true;
}

// CListAddressResult
CListAddressResult::CListAddressResult() {}
CListAddressResult::CListAddressResult(const CRPCVector<CAddressdata>& vecAddressdata)
	: vecAddressdata(vecAddressdata)
{
}
Value CListAddressResult::ToJSON() const
{
	Array ret;
	for (auto& v : vecAddressdata)
	{
		ret.push_back(v.ToJSON());
	}
	return ret;
}
CListAddressResult& CListAddressResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "array", "addressdata");
	auto vecAddressdataArray = v.get_array();
	for (auto& v : vecAddressdataArray)
	{
		vecAddressdata.push_back(CRPCVector<CAddressdata>::value_type().FromJSON(v));
	}
	return *this;
}
string CListAddressResult::Method() const
{
	return "listaddress";
}

// CListAddressConfig
CListAddressConfig::CListAddressConfig()
{
}
bool CListAddressConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CListAddressConfig::ListConfig() const
{
	return "";
}
string CListAddressConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        listaddress\n";
	oss << "\n";
	oss << "List all of addresses from pub keys and template ids\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << "   [\n";
	oss << "     \"addressdata\":             (object, required) address data\n";
	oss << "     {\n";
	oss << "       \"type\": \"\",              (string, required) type, pubkey or template\n";
	oss << "       (if \"type\" is \"pubkey\")\n";
	oss << "       \"pubkey\":                (object, required) public key\n";
	oss << "       {\n";
	oss << "         \"key\": \"\",             (string, required) public key\n";
	oss << "         \"address\": \"\"          (string, required) public key address\n";
	oss << "       }\n";
	oss << "       (if \"type\" is \"template\")\n";
	oss << "       \"template\": \"\",          (string, required) template type name\n";
	oss << "       (if \"type\" is \"template\")\n";
	oss << "       \"templatedata\":          (object, optional) template data\n";
	oss << "       {\n";
	oss << "         \"type\": \"\",            (string, required) template type\n";
	oss << "         \"hex\": \"\",             (string, required) temtplate data\n";
	oss << "         (if \"type\" is \"delegate\")\n";
	oss << "         \"delegate\":            (object, required) delegate template struct\n";
	oss << "         {\n";
	oss << "           \"delegate\": \"\",      (string, required) delegate public key\n";
	oss << "           \"owner\": \"\"          (string, required) owner address\n";
	oss << "         }\n";
	oss << "         (if \"type\" is \"fork\")\n";
	oss << "         \"fork\":                (object, required) fork template struct\n";
	oss << "         {\n";
	oss << "           \"redeem\": \"\",        (string, required) redeem address\n";
	oss << "           \"fork\": \"\"           (string, required) fork hash\n";
	oss << "         }\n";
	oss << "         (if \"type\" is \"mint\")\n";
	oss << "         \"mint\":                (object, required) mint template struct\n";
	oss << "         {\n";
	oss << "           \"mint\": \"\",          (string, required) mint public key\n";
	oss << "           \"spent\": \"\"          (string, required) spent address\n";
	oss << "         }\n";
	oss << "         (if \"type\" is \"multisig\")\n";
	oss << "         \"multisig\":            (object, required) multisig template struct\n";
	oss << "         {\n";
	oss << "           \"sigsrequired\": 0,   (int, required) required weight\n";
	oss << "           [\n";
	oss << "             \"key\": \"\"          (string, required) public key\n";
	oss << "           ]\n";
	oss << "         }\n";
	oss << "         (if \"type\" is \"weighted\")\n";
	oss << "         \"weighted\":            (object, required) weighted template struct\n";
	oss << "         {\n";
	oss << "           \"sigsrequired\": 0,   (int, required) required weight\n";
	oss << "           [\n";
	oss << "             \"pubkey\":          (object, required) public key\n";
	oss << "             {\n";
	oss << "               \"key\": \"\",       (string, required) public key\n";
	oss << "               \"weight\": 0      (int, required) weight\n";
	oss << "             }\n";
	oss << "           ]\n";
	oss << "         }\n";
	oss << "       }\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli listaddress\n";
	oss << "<< [{\"type\":\"pubkey\",\"pubkey\":{\"key\":\"182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e\",\"address\":\"1gbma6s21t4bcwymqz6h1dn1t7qy45019b1t00ywfyqymbvp90mqc1wmq\"}},{\"type\":\"pubkey\",\"pubkey\":{\"key\":\"1051ac9b153196c736adf61ad71b51fe9659f8770df643af4d27616ca85c0b365\",\"address\":\"10mdckcak35p76tpzc6pq3d8zx5jsz1vgvxj3nx6jerbcn1e0pdjjfcr6\"}}]\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"listaddress\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":[{\"type\":\"pubkey\",\"pubkey\":{\"key\":\"182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e\",\"address\":\"1gbma6s21t4bcwymqz6h1dn1t7qy45019b1t00ywfyqymbvp90mqc1wmq\"}},{\"type\":\"pubkey\",\"pubkey\":{\"key\":\"1051ac9b153196c736adf61ad71b51fe9659f8770df643af4d27616ca85c0b365\",\"address\":\"10mdckcak35p76tpzc6pq3d8zx5jsz1vgvxj3nx6jerbcn1e0pdjjfcr6\"}}]}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// exportwallet

// CExportWalletParam
CExportWalletParam::CExportWalletParam() {}
CExportWalletParam::CExportWalletParam(const CRPCString& strPath)
	: strPath(strPath)
{
}
Value CExportWalletParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPath, "strPath");
	ret.push_back(Pair("path", std::string(strPath)));

	return ret;
}
CExportWalletParam& CExportWalletParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "exportwallet");
	auto obj = v.get_obj();
	auto valPath = find_value(obj, "path");
	CheckJSONType(valPath, "string", "path");
	strPath = valPath.get_str();
	return *this;
}
string CExportWalletParam::Method() const
{
	return "exportwallet";
}

// CExportWalletResult
CExportWalletResult::CExportWalletResult() {}
CExportWalletResult::CExportWalletResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CExportWalletResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CExportWalletResult& CExportWalletResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CExportWalletResult::Method() const
{
	return "exportwallet";
}

// CExportWalletConfig
CExportWalletConfig::CExportWalletConfig()
{
}
bool CExportWalletConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPath;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[path] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[path] is required");
	}
	return true;
}
string CExportWalletConfig::ListConfig() const
{
	return "";
}
string CExportWalletConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        exportwallet <\"path\">\n";
	oss << "\n";
	oss << "Export all of keys and templates from wallet to a specified file in json format.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"path\"                         (string, required) save file path\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"path\": \"\"                   (string, required) save file path\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) export result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli exportwallet /Users/Loading/a.txt\n";
	oss << "<< Wallet file has been saved at: /Users/Loading/a.txt\n";
	oss << "\n>> {\"id\":4,\"method\":\"exportwallet\",\"jsonrpc\":\"2.0\",\"params\":{\"path\":\"/Users/Loading/a.txt\"}}\n";
	oss << "<< {\"id\":4,\"jsonrpc\":\"2.0\",\"result\":\"Wallet file has been saved at: /Users/Loading/a.txt\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-401,\"message\":\"Must be an absolute path.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Cannot export to a folder.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"File has been existed.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to create directories.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"filesystem_error\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// importwallet

// CImportWalletParam
CImportWalletParam::CImportWalletParam() {}
CImportWalletParam::CImportWalletParam(const CRPCString& strPath)
	: strPath(strPath)
{
}
Value CImportWalletParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPath, "strPath");
	ret.push_back(Pair("path", std::string(strPath)));

	return ret;
}
CImportWalletParam& CImportWalletParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "importwallet");
	auto obj = v.get_obj();
	auto valPath = find_value(obj, "path");
	CheckJSONType(valPath, "string", "path");
	strPath = valPath.get_str();
	return *this;
}
string CImportWalletParam::Method() const
{
	return "importwallet";
}

// CImportWalletResult
CImportWalletResult::CImportWalletResult() {}
CImportWalletResult::CImportWalletResult(const CRPCString& strResult)
	: strResult(strResult)
{
}
Value CImportWalletResult::ToJSON() const
{
	CheckIsValid(strResult, "strResult");
	Value val;
	val = Value(strResult);
	return val;
}
CImportWalletResult& CImportWalletResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "result");
	strResult = v.get_str();
	return *this;
}
string CImportWalletResult::Method() const
{
	return "importwallet";
}

// CImportWalletConfig
CImportWalletConfig::CImportWalletConfig()
{
}
bool CImportWalletConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPath;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[path] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[path] is required");
	}
	return true;
}
string CImportWalletConfig::ListConfig() const
{
	return "";
}
string CImportWalletConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        importwallet <\"path\">\n";
	oss << "\n";
	oss << "Import keys and templates from archived file in json format to wallet.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"path\"                         (string, required) save file path\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"path\": \"\"                   (string, required) save file path\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"result\"             (string, required) export result\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli importwallet /Users/Loading/a.txt\n";
	oss << "<< Imported 0 keys and 0 templates.\n";
	oss << "\n>> {\"id\":5,\"method\":\"importwallet\",\"jsonrpc\":\"2.0\",\"params\":{\"path\":\"/Users/Loading/a.txt\"}}\n";
	oss << "<< {\"id\":5,\"jsonrpc\":\"2.0\",\"result\":\"Imported 0 keys and 0 templates.\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-401,\"message\":\"Must be an absolute path.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"File name is invalid.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Filesystem_error - failed to read.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Data format is not correct, check it and try again.\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to verify serialized key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to add key\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to sync wallet tx\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Invalid parameters,failed to make template\"}\n";
	oss << "* {\"code\":-401,\"message\":\"Failed to add template\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// verifymessage

// CVerifyMessageParam
CVerifyMessageParam::CVerifyMessageParam() {}
CVerifyMessageParam::CVerifyMessageParam(const CRPCString& strPubkey, const CRPCString& strMessage, const CRPCString& strSig)
	: strPubkey(strPubkey), strMessage(strMessage), strSig(strSig)
{
}
Value CVerifyMessageParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));
	CheckIsValid(strMessage, "strMessage");
	ret.push_back(Pair("message", std::string(strMessage)));
	CheckIsValid(strSig, "strSig");
	ret.push_back(Pair("sig", std::string(strSig)));

	return ret;
}
CVerifyMessageParam& CVerifyMessageParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "verifymessage");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	auto valMessage = find_value(obj, "message");
	CheckJSONType(valMessage, "string", "message");
	strMessage = valMessage.get_str();
	auto valSig = find_value(obj, "sig");
	CheckJSONType(valSig, "string", "sig");
	strSig = valSig.get_str();
	return *this;
}
string CVerifyMessageParam::Method() const
{
	return "verifymessage";
}

// CVerifyMessageResult
CVerifyMessageResult::CVerifyMessageResult() {}
CVerifyMessageResult::CVerifyMessageResult(const CRPCBool& fResult)
	: fResult(fResult)
{
}
Value CVerifyMessageResult::ToJSON() const
{
	CheckIsValid(fResult, "fResult");
	Value val;
	val = Value(fResult);
	return val;
}
CVerifyMessageResult& CVerifyMessageResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "bool", "result");
	fResult = v.get_bool();
	return *this;
}
string CVerifyMessageResult::Method() const
{
	return "verifymessage";
}

// CVerifyMessageConfig
CVerifyMessageConfig::CVerifyMessageConfig()
{
}
bool CVerifyMessageConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 4)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strMessage;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[message] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[message] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strSig;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[sig] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[sig] is required");
	}
	return true;
}
string CVerifyMessageConfig::ListConfig() const
{
	return "";
}
string CVerifyMessageConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        verifymessage <\"pubkey\"> <\"message\"> <\"sig\">\n";
	oss << "\n";
	oss << "Verify a signed message\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) public key\n";
	oss << " \"message\"                      (string, required) message to be verified\n";
	oss << " \"sig\"                          (string, required) sign\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\",                (string, required) public key\n";
	oss << "   \"message\": \"\",               (string, required) message to be verified\n";
	oss << "   \"sig\": \"\"                    (string, required) sign\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": result               (bool, required) message verified result.\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli verifymessage 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882 123456 045977f8c07e6d846d6055357f36a70c16c071cb85115e3ffb498e171a9ac3f4aed1292203a0c8e42c4becafad3ced0d9874abd2a8b788fda9f07099a1e71707\n";
	oss << "<< true\n";
	oss << "\n>> curl -d '{\"id\":5,\"method\":\"verifymessage\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\",\"message\":\"123456\",\"sig\":\"045977f8c07e6d846d6055357f36a70c16c071cb85115e3ffb498e171a9ac3f4aed1292203a0c8e42c4becafad3ced0d9874abd2a8b788fda9f07099a1e71707\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":5,\"jsonrpc\":\"2.0\",\"result\":true}\n";
	oss << "\n>> multiverse-cli verifymessage 2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882 12345 045977f8c07e6d846d6055357f36a70c16c071cb85115e3ffb498e171a9ac3f4aed1292203a0c8e42c4becafad3ced0d9874abd2a8b788fda9f07099a1e71707\n";
	oss << "<< false\n";
	oss << "\n>> curl -d '{\"id\":6,\"method\":\"verifymessage\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\",\"message\":\"12345\",\"sig\":\"045977f8c07e6d846d6055357f36a70c16c071cb85115e3ffb498e171a9ac3f4aed1292203a0c8e42c4becafad3ced0d9874abd2a8b788fda9f07099a1e71707\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":6,\"jsonrpc\":\"2.0\",\"result\":false}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// makekeypair

// CMakeKeyPairParam
CMakeKeyPairParam::CMakeKeyPairParam() {}
Value CMakeKeyPairParam::ToJSON() const
{
	Object ret;

	return ret;
}
CMakeKeyPairParam& CMakeKeyPairParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "makekeypair");
	auto obj = v.get_obj();
	return *this;
}
string CMakeKeyPairParam::Method() const
{
	return "makekeypair";
}

// CMakeKeyPairResult
CMakeKeyPairResult::CMakeKeyPairResult() {}
CMakeKeyPairResult::CMakeKeyPairResult(const CRPCString& strPrivkey, const CRPCString& strPubkey)
	: strPrivkey(strPrivkey), strPubkey(strPubkey)
{
}
Value CMakeKeyPairResult::ToJSON() const
{
	Object ret;
	CheckIsValid(strPrivkey, "strPrivkey");
	ret.push_back(Pair("privkey", std::string(strPrivkey)));
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));

	return ret;
}
CMakeKeyPairResult& CMakeKeyPairResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "makekeypair");
	auto obj = v.get_obj();
	auto valPrivkey = find_value(obj, "privkey");
	CheckJSONType(valPrivkey, "string", "privkey");
	strPrivkey = valPrivkey.get_str();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	return *this;
}
string CMakeKeyPairResult::Method() const
{
	return "makekeypair";
}

// CMakeKeyPairConfig
CMakeKeyPairConfig::CMakeKeyPairConfig()
{
}
bool CMakeKeyPairConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 1)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	return true;
}
string CMakeKeyPairConfig::ListConfig() const
{
	return "";
}
string CMakeKeyPairConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        makekeypair\n";
	oss << "\n";
	oss << "Make a public/private key pair.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << "\tnone\n\n";
	oss << "Request:\n";
	oss << " \"param\" : {}\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"privkey\": \"\",               (string, required) private key\n";
	oss << "   \"pubkey\": \"\"                 (string, required) public key\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli makekeypair\n";
	oss << "<< {\"privkey\":\"833a5d51d2db84debc0eb3a40d7d41b2723452d211d7e81ce489a95ef48b2324\",\"pubkey\":\"e8e3770e774d5ad84a8ea65ed08cc7c5c30b42e045623604d5c5c6be95afb4f9\"}\n";
	oss << "\n>> curl -d '{\"id\":42,\"method\":\"makekeypair\",\"jsonrpc\":\"2.0\",\"params\":{}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":42,\"jsonrpc\":\"2.0\",\"result\":{\"privkey\":\"833a5d51d2db84debc0eb3a40d7d41b2723452d211d7e81ce489a95ef48b2324\",\"pubkey\":\"e8e3770e774d5ad84a8ea65ed08cc7c5c30b42e045623604d5c5c6be95afb4f9\"}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getpubkeyaddress

// CGetPubkeyAddressParam
CGetPubkeyAddressParam::CGetPubkeyAddressParam() {}
CGetPubkeyAddressParam::CGetPubkeyAddressParam(const CRPCString& strPubkey)
	: strPubkey(strPubkey)
{
}
Value CGetPubkeyAddressParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPubkey, "strPubkey");
	ret.push_back(Pair("pubkey", std::string(strPubkey)));

	return ret;
}
CGetPubkeyAddressParam& CGetPubkeyAddressParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getpubkeyaddress");
	auto obj = v.get_obj();
	auto valPubkey = find_value(obj, "pubkey");
	CheckJSONType(valPubkey, "string", "pubkey");
	strPubkey = valPubkey.get_str();
	return *this;
}
string CGetPubkeyAddressParam::Method() const
{
	return "getpubkeyaddress";
}

// CGetPubkeyAddressResult
CGetPubkeyAddressResult::CGetPubkeyAddressResult() {}
CGetPubkeyAddressResult::CGetPubkeyAddressResult(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CGetPubkeyAddressResult::ToJSON() const
{
	CheckIsValid(strAddress, "strAddress");
	Value val;
	val = Value(strAddress);
	return val;
}
CGetPubkeyAddressResult& CGetPubkeyAddressResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "address");
	strAddress = v.get_str();
	return *this;
}
string CGetPubkeyAddressResult::Method() const
{
	return "getpubkeyaddress";
}

// CGetPubkeyAddressConfig
CGetPubkeyAddressConfig::CGetPubkeyAddressConfig()
{
}
bool CGetPubkeyAddressConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPubkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[pubkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[pubkey] is required");
	}
	return true;
}
string CGetPubkeyAddressConfig::ListConfig() const
{
	return "";
}
string CGetPubkeyAddressConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getpubkeyaddress <\"pubkey\">\n";
	oss << "\n";
	oss << "Returns encoded address for the given pubkey.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"pubkey\"                       (string, required) public key\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"pubkey\": \"\"                 (string, required) public key\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"address\"            (string, required) address of public key\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getpubkeyaddress e8e3770e774d5ad84a8ea65ed08cc7c5c30b42e045623604d5c5c6be95afb4f9\n";
	oss << "<< 1z6taz5dyrv2xa11pc92y0ggbrf2wf36gbtk8wjprb96qe3kqwfm3ayc1\n";
	oss << "\n>> curl -d '{\"id\":44,\"method\":\"getpubkeyaddress\",\"jsonrpc\":\"2.0\",\"params\":{\"pubkey\":\"e8e3770e774d5ad84a8ea65ed08cc7c5c30b42e045623604d5c5c6be95afb4f9\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":44,\"jsonrpc\":\"2.0\",\"result\":\"1z6taz5dyrv2xa11pc92y0ggbrf2wf36gbtk8wjprb96qe3kqwfm3ayc1\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// gettemplateaddress

// CGetTemplateAddressParam
CGetTemplateAddressParam::CGetTemplateAddressParam() {}
CGetTemplateAddressParam::CGetTemplateAddressParam(const CRPCString& strTid)
	: strTid(strTid)
{
}
Value CGetTemplateAddressParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strTid, "strTid");
	ret.push_back(Pair("tid", std::string(strTid)));

	return ret;
}
CGetTemplateAddressParam& CGetTemplateAddressParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "gettemplateaddress");
	auto obj = v.get_obj();
	auto valTid = find_value(obj, "tid");
	CheckJSONType(valTid, "string", "tid");
	strTid = valTid.get_str();
	return *this;
}
string CGetTemplateAddressParam::Method() const
{
	return "gettemplateaddress";
}

// CGetTemplateAddressResult
CGetTemplateAddressResult::CGetTemplateAddressResult() {}
CGetTemplateAddressResult::CGetTemplateAddressResult(const CRPCString& strAddress)
	: strAddress(strAddress)
{
}
Value CGetTemplateAddressResult::ToJSON() const
{
	CheckIsValid(strAddress, "strAddress");
	Value val;
	val = Value(strAddress);
	return val;
}
CGetTemplateAddressResult& CGetTemplateAddressResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "address");
	strAddress = v.get_str();
	return *this;
}
string CGetTemplateAddressResult::Method() const
{
	return "gettemplateaddress";
}

// CGetTemplateAddressConfig
CGetTemplateAddressConfig::CGetTemplateAddressConfig()
{
}
bool CGetTemplateAddressConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTid;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[tid] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[tid] is required");
	}
	return true;
}
string CGetTemplateAddressConfig::ListConfig() const
{
	return "";
}
string CGetTemplateAddressConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        gettemplateaddress <\"tid\">\n";
	oss << "\n";
	oss << "Returns encoded address for the given template id.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"tid\"                          (string, required) template id\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"tid\": \"\"                    (string, required) template id\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"address\"            (string, required) address of template\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli gettemplateaddress 2040fpytdr4k5h8tk0nferr7zb51tkccrkgqf341s6tg05q9xe6hth1m4\n";
	oss << "<< 21w2040000000000000000000000000000000000000000000000epcek\n";
	oss << "\n>> curl -d '{\"id\":53,\"method\":\"gettemplateaddress\",\"jsonrpc\":\"2.0\",\"params\":{\"tid\":\"2040fpytdr4k5h8tk0nferr7zb51tkccrkgqf341s6tg05q9xe6hth1m4\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":53,\"jsonrpc\":\"2.0\",\"result\":\"21w2040000000000000000000000000000000000000000000000epcek\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// maketemplate

// CMakeTemplateParam
CMakeTemplateParam::CMakeTemplateParam() {}
CMakeTemplateParam::CMakeTemplateParam(const CTemplateRequest& data)
	: data(data)
{
}
Value CMakeTemplateParam::ToJSON() const
{
	CheckIsValid(data, "data");
	return data.ToJSON();
}
CMakeTemplateParam& CMakeTemplateParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "data");
	data.FromJSON(v.get_obj());
	return *this;
}
string CMakeTemplateParam::Method() const
{
	return "maketemplate";
}

// CMakeTemplateResult
CMakeTemplateResult::CMakeTemplateResult() {}
CMakeTemplateResult::CMakeTemplateResult(const CRPCString& strAddress, const CRPCString& strHex)
	: strAddress(strAddress), strHex(strHex)
{
}
Value CMakeTemplateResult::ToJSON() const
{
	Object ret;
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));
	CheckIsValid(strHex, "strHex");
	ret.push_back(Pair("hex", std::string(strHex)));

	return ret;
}
CMakeTemplateResult& CMakeTemplateResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "maketemplate");
	auto obj = v.get_obj();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	auto valHex = find_value(obj, "hex");
	CheckJSONType(valHex, "string", "hex");
	strHex = valHex.get_str();
	return *this;
}
string CMakeTemplateResult::Method() const
{
	return "maketemplate";
}

// CMakeTemplateConfig
CMakeTemplateConfig::CMakeTemplateConfig()
{
}
bool CMakeTemplateConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 7)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> data.strType;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[type] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[type] is required");
	}
	if (data.strType == "delegate")
	{
		string strOriginDelegate;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginDelegate = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[delegate] is required");
		}
		Value valOriginDelegate;
		if (!read_string(strOriginDelegate, valOriginDelegate))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.delegate.FromJSON(valOriginDelegate);
	}
	if (data.strType == "fork")
	{
		string strOriginFork;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginFork = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[fork] is required");
		}
		Value valOriginFork;
		if (!read_string(strOriginFork, valOriginFork))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.fork.FromJSON(valOriginFork);
	}
	if (data.strType == "mint")
	{
		string strOriginMint;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginMint = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[mint] is required");
		}
		Value valOriginMint;
		if (!read_string(strOriginMint, valOriginMint))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.mint.FromJSON(valOriginMint);
	}
	if (data.strType == "multisig")
	{
		string strOriginMultisig;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginMultisig = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[multisig] is required");
		}
		Value valOriginMultisig;
		if (!read_string(strOriginMultisig, valOriginMultisig))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.multisig.FromJSON(valOriginMultisig);
	}
	if (data.strType == "weighted")
	{
		string strOriginWeighted;
		if (next(it, 1) != vecCommand.end())
		{
			strOriginWeighted = *++it;
		}
		else
		{
			throw CRPCException(RPC_PARSE_ERROR, "[weighted] is required");
		}
		Value valOriginWeighted;
		if (!read_string(strOriginWeighted, valOriginWeighted))
		{
			throw CRPCException(RPC_PARSE_ERROR, "parse json error");
		}
		data.weighted.FromJSON(valOriginWeighted);
	}
	return true;
}
string CMakeTemplateConfig::ListConfig() const
{
	return "";
}
string CMakeTemplateConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        maketemplate <\"type\"> <{delegate}>|<{fork}>|<{mint}>|<{multisig}>|<{weighted}>\n";
	oss << "\n";
	oss << "Returns encoded address for the given template id.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"type\"                         (string, required) template type\n";
	oss << "  (if type is \"delegate\")\n";
	oss << " {delegate}                     (object, required) a delegate template\n";
	oss << "  (if type is \"fork\")\n";
	oss << " {fork}                         (object, required) a new fork template\n";
	oss << "  (if type is \"mint\")\n";
	oss << " {mint}                         (object, required) a mint template\n";
	oss << "  (if type is \"multisig\")\n";
	oss << " {multisig}                     (object, required) a multiple sign template\n";
	oss << "  (if type is \"weighted\")\n";
	oss << " {weighted}                     (object, required) a weighted multiple sign template\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"type\": \"\",                  (string, required) template type\n";
	oss << "   (if \"type\" is \"delegate\")\n";
	oss << "   \"delegate\":                  (object, required) a delegate template\n";
	oss << "   {\n";
	oss << "     \"delegate\": \"\",            (string, required) delegate public key\n";
	oss << "     \"owner\": \"\"                (string, required) owner address\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"fork\")\n";
	oss << "   \"fork\":                      (object, required) a new fork template\n";
	oss << "   {\n";
	oss << "     \"redeem\": \"\",              (string, required) redeem address\n";
	oss << "     \"fork\": \"\"                 (string, required) fork hash\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"mint\")\n";
	oss << "   \"mint\":                      (object, required) a mint template\n";
	oss << "   {\n";
	oss << "     \"mint\": \"\",                (string, required) mint public key\n";
	oss << "     \"spent\": \"\"                (string, required) spent address\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"multisig\")\n";
	oss << "   \"multisig\":                  (object, required) a multiple sign template\n";
	oss << "   {\n";
	oss << "     \"required\": 0,             (int, required) required weight > 0\n";
	oss << "     [\n";
	oss << "       \"key\": \"\"                (string, required) public key\n";
	oss << "     ]\n";
	oss << "   }\n";
	oss << "   (if \"type\" is \"weighted\")\n";
	oss << "   \"weighted\":                  (object, required) a weighted multiple sign template\n";
	oss << "   {\n";
	oss << "     \"required\": 0,             (int, required) required weight\n";
	oss << "     [\n";
	oss << "       \"pubkey\":                (object, required) public key\n";
	oss << "       {\n";
	oss << "         \"key\": \"\",             (string, required) public key\n";
	oss << "         \"weight\": 0            (int, required) weight\n";
	oss << "       }\n";
	oss << "     ]\n";
	oss << "   }\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"address\": \"\",               (string, required) address of template\n";
	oss << "   \"hex\": \"\"                    (string, required) template data hex string\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli maketemplate multisig '{\"required\": 1, \"pubkeys\": [\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\", \"f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\"]}'\n";
	oss << "<< {\"address\":\"208043ht3c51qztrdfa0f3349pe2m8ajjw1mdb2py68fbckaa2s24tq55\",\"hex\":\"0200010282e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052eb9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f4\"}\n";
	oss << "\n>> curl -d '{\"id\":54,\"method\":\"maketemplate\",\"jsonrpc\":\"2.0\",\"params\":{\"type\":\"multisig\",\"multisig\":{\"required\":1,\"pubkeys\":[\"2e05c9ee45fdf58f7b007458298042fc3d3ad416a2f9977ace16d14164a3e882\",\"f4124c636d37b1308ba95c14b2487134030d5817f7fa93f11bcbc616aab7c3b9\"]}}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":54,\"jsonrpc\":\"2.0\",\"result\":{\"address\":\"208043ht3c51qztrdfa0f3349pe2m8ajjw1mdb2py68fbckaa2s24tq55\",\"hex\":\"0200010282e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052eb9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f4\"}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameters,failed to make template\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing weight\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing redeem address\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing spent address\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Invalid parameter, missing owner address\"}\n";
	oss << "* {\"code\":-6,\"message\":\"template type error. type: xxx\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// decodetransaction

// CDecodeTransactionParam
CDecodeTransactionParam::CDecodeTransactionParam() {}
CDecodeTransactionParam::CDecodeTransactionParam(const CRPCString& strTxdata)
	: strTxdata(strTxdata)
{
}
Value CDecodeTransactionParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strTxdata, "strTxdata");
	ret.push_back(Pair("txdata", std::string(strTxdata)));

	return ret;
}
CDecodeTransactionParam& CDecodeTransactionParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "decodetransaction");
	auto obj = v.get_obj();
	auto valTxdata = find_value(obj, "txdata");
	CheckJSONType(valTxdata, "string", "txdata");
	strTxdata = valTxdata.get_str();
	return *this;
}
string CDecodeTransactionParam::Method() const
{
	return "decodetransaction";
}

// CDecodeTransactionResult
CDecodeTransactionResult::CDecodeTransactionResult() {}
CDecodeTransactionResult::CDecodeTransactionResult(const CTransactionData& transaction)
	: transaction(transaction)
{
}
Value CDecodeTransactionResult::ToJSON() const
{
	CheckIsValid(transaction, "transaction");
	return transaction.ToJSON();
}
CDecodeTransactionResult& CDecodeTransactionResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "transaction");
	transaction.FromJSON(v.get_obj());
	return *this;
}
string CDecodeTransactionResult::Method() const
{
	return "decodetransaction";
}

// CDecodeTransactionConfig
CDecodeTransactionConfig::CDecodeTransactionConfig()
{
}
bool CDecodeTransactionConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strTxdata;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[txdata] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[txdata] is required");
	}
	return true;
}
string CDecodeTransactionConfig::ListConfig() const
{
	return "";
}
string CDecodeTransactionConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        decodetransaction <\"txdata\">\n";
	oss << "\n";
	oss << "Return a JSON object representing the serialized, hex-encoded transaction.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"txdata\"                       (string, required) transaction raw data\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"txdata\": \"\"                 (string, required) transaction raw data\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"txid\": \"\",                  (string, required) transaction hash\n";
	oss << "   \"version\": 0,                (uint, required) version\n";
	oss << "   \"type\": \"\",                  (string, required) transaction type\n";
	oss << "   \"lockuntil\": 0,              (uint, required) unlock time\n";
	oss << "   \"anchor\": \"\",                (string, required) anchor hash\n";
	oss << "   [\n";
	oss << "     \"vin\":                     (object, required) vin struct\n";
	oss << "     {\n";
	oss << "       \"txid\": \"\",              (string, required) pre-vout transaction hash\n";
	oss << "       \"vout\": 0                (uint, required) pre-vout number\n";
	oss << "     }\n";
	oss << "   ]\n";
	oss << "   \"sendto\": \"\",                (string, required) send to address\n";
	oss << "   \"amount\": 0.0,               (double, required) amount\n";
	oss << "   \"txfee\": 0.0,                (double, required) transaction fee\n";
	oss << "   \"data\": \"\",                  (string, required) data\n";
	oss << "   \"sig\": \"\",                   (string, required) sign\n";
	oss << "   \"fork\": \"\",                  (string, required) fork hash\n";
	oss << "   \"confirmations\": 0           (int, optional) confirmations\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli decodetransaction 01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a08601000000000002123400\n";
	oss << "<< {\"txid\":\"b492ea1de2d540288f6e45fd21bc4ac2cd2fcfeb63ec43c50acdb69debfad10a\",\"version\":1,\"type\":\"token\",\"lockuntil\":0,\"anchor\":\"25439b778af5e310a13f2310b48ecb329674ba1dc2054affccef8b73247e742b\",\"vin\":[{\"txid\":\"8119097ebc65abd55889e90be6de69c218082d05299951573a455701e55bf40e\",\"vout\":0}],\"sendto\":\"1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q\",\"amount\":1.00000000,\"txfee\":0.10000000,\"data\":\"1234\",\"sig\":\"\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\"}\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"decodetransaction\",\"jsonrpc\":\"2.0\",\"params\":{\"txdata\":\"01000000000000002b747e24738befccff4a05c21dba749632cb8eb410233fa110e3f58a779b4325010ef45be50157453a57519929052d0818c269dee60be98958d5ab65bc7e0919810001b9c3b7aa16c6cb1bf193faf717580d03347148b2145ca98b30b1376d634c12f440420f0000000000a08601000000000002123400\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":{\"txid\":\"b492ea1de2d540288f6e45fd21bc4ac2cd2fcfeb63ec43c50acdb69debfad10a\",\"version\":1,\"type\":\"token\",\"lockuntil\":0,\"anchor\":\"25439b778af5e310a13f2310b48ecb329674ba1dc2054affccef8b73247e742b\",\"vin\":[{\"txid\":\"8119097ebc65abd55889e90be6de69c218082d05299951573a455701e55bf40e\",\"vout\":0}],\"sendto\":\"1q71vfagprv5hqwckzbvhep0d0ct72j5j2heak2sgp4vptrtc2btdje3q\",\"amount\":1.00000000,\"txfee\":0.10000000,\"data\":\"1234\",\"sig\":\"\",\"fork\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\"}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-8,\"message\":\"TX decode failed\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Unknown anchor block\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// makeorigin

// CMakeOriginParam
CMakeOriginParam::CMakeOriginParam() {}
CMakeOriginParam::CMakeOriginParam(const CRPCString& strPrev, const CRPCString& strAddress, const CRPCDouble& fAmount, const CRPCString& strIdent)
	: strPrev(strPrev), strAddress(strAddress), fAmount(fAmount), strIdent(strIdent)
{
}
Value CMakeOriginParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strPrev, "strPrev");
	ret.push_back(Pair("prev", std::string(strPrev)));
	CheckIsValid(strAddress, "strAddress");
	ret.push_back(Pair("address", std::string(strAddress)));
	CheckIsValid(fAmount, "fAmount");
	ret.push_back(Pair("amount", double(fAmount)));
	CheckIsValid(strIdent, "strIdent");
	ret.push_back(Pair("ident", std::string(strIdent)));

	return ret;
}
CMakeOriginParam& CMakeOriginParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "makeorigin");
	auto obj = v.get_obj();
	auto valPrev = find_value(obj, "prev");
	CheckJSONType(valPrev, "string", "prev");
	strPrev = valPrev.get_str();
	auto valAddress = find_value(obj, "address");
	CheckJSONType(valAddress, "string", "address");
	strAddress = valAddress.get_str();
	auto valAmount = find_value(obj, "amount");
	CheckJSONType(valAmount, "double", "amount");
	fAmount = valAmount.get_real();
	auto valIdent = find_value(obj, "ident");
	CheckJSONType(valIdent, "string", "ident");
	strIdent = valIdent.get_str();
	return *this;
}
string CMakeOriginParam::Method() const
{
	return "makeorigin";
}

// CMakeOriginResult
CMakeOriginResult::CMakeOriginResult() {}
CMakeOriginResult::CMakeOriginResult(const CRPCString& strHash, const CRPCString& strHex)
	: strHash(strHash), strHex(strHex)
{
}
Value CMakeOriginResult::ToJSON() const
{
	Object ret;
	CheckIsValid(strHash, "strHash");
	ret.push_back(Pair("hash", std::string(strHash)));
	CheckIsValid(strHex, "strHex");
	ret.push_back(Pair("hex", std::string(strHex)));

	return ret;
}
CMakeOriginResult& CMakeOriginResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "makeorigin");
	auto obj = v.get_obj();
	auto valHash = find_value(obj, "hash");
	CheckJSONType(valHash, "string", "hash");
	strHash = valHash.get_str();
	auto valHex = find_value(obj, "hex");
	CheckJSONType(valHex, "string", "hex");
	strHex = valHex.get_str();
	return *this;
}
string CMakeOriginResult::Method() const
{
	return "makeorigin";
}

// CMakeOriginConfig
CMakeOriginConfig::CMakeOriginConfig()
{
}
bool CMakeOriginConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 5)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPrev;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[prev] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[prev] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strAddress;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[address] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[address] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> fAmount;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[amount] type error, needs double");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[amount] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strIdent;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[ident] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[ident] is required");
	}
	return true;
}
string CMakeOriginConfig::ListConfig() const
{
	return "";
}
string CMakeOriginConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        makeorigin <\"prev\"> <\"address\"> <$amount$> <\"ident\">\n";
	oss << "\n";
	oss << "Return hex-encoded block.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"prev\"                         (string, required) prev block hash\n";
	oss << " \"address\"                      (string, required) address\n";
	oss << " $amount$                       (double, required) amount\n";
	oss << " \"ident\"                        (string, required) indent\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"prev\": \"\",                  (string, required) prev block hash\n";
	oss << "   \"address\": \"\",               (string, required) address\n";
	oss << "   \"amount\": 0.0,               (double, required) amount\n";
	oss << "   \"ident\": \"\"                  (string, required) indent\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   \"hash\": \"\",                  (string, required) block hash\n";
	oss << "   \"hex\": \"\"                    (string, required) block data hex\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli makeorigin a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0 1gbma6s21t4bcwymqz6h1dn1t7qy45019b1t00ywfyqymbvp90mqc1wmq 1 POW\n";
	oss << "<< {\"hash\":\"c80cad6f2e8c0b0cee5182fcb70e0da40149b5740223ea17814d70bf8740fdab\",\"hex\":\"010000ffc06f585ac0afdc176980a0a1ebe849d09b51e4b4c1dd46fb939579d71bdc55809d6f3da6000000000000000000000000000000000000000000000000000000000000000003504f5701000001000000000000000000000000000000000000000000000000000000000000000000000000000182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e40420f0000000000000000000000000000000000\"}\n";
	oss << "\n>> curl -d '{\"id\":7,\"method\":\"makeorigin\",\"jsonrpc\":\"2.0\",\"params\":{\"prev\":\"a63d6f9d8055dc1bd7799593fb46ddc1b4e4519bd049e8eba1a0806917dcafc0\",\"address\":\"1gbma6s21t4bcwymqz6h1dn1t7qy45019b1t00ywfyqymbvp90mqc1wmq\",\"amount\":1,\"ident\":\"POW\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":7,\"jsonrpc\":\"2.0\",\"result\":{\"hash\":\"c80cad6f2e8c0b0cee5182fcb70e0da40149b5740223ea17814d70bf8740fdab\",\"hex\":\"010000ffc06f585ac0afdc176980a0a1ebe849d09b51e4b4c1dd46fb939579d71bdc55809d6f3da6000000000000000000000000000000000000000000000000000000000000000003504f5701000001000000000000000000000000000000000000000000000000000000000000000000000000000182e8a36441d116ce7a97f9a216d43a3dfc4280295874007b8ff5fd45eec9052e40420f0000000000000000000000000000000000\"}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "* {\"code\":-8,\"message\":\"TX decode failed\"}\n";
	oss << "* {\"code\":-6,\"message\":\"Unknown anchor block\"}\n";
	oss << "\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// getwork

// CGetWorkParam
CGetWorkParam::CGetWorkParam() {}
CGetWorkParam::CGetWorkParam(const CRPCString& strPrev)
	: strPrev(strPrev)
{
}
Value CGetWorkParam::ToJSON() const
{
	Object ret;
	if (strPrev.IsValid())
	{
		ret.push_back(Pair("prev", std::string(strPrev)));
	}

	return ret;
}
CGetWorkParam& CGetWorkParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getwork");
	auto obj = v.get_obj();
	auto valPrev = find_value(obj, "prev");
	if (!valPrev.is_null())
	{
		CheckJSONType(valPrev, "string", "prev");
		strPrev = valPrev.get_str();
	}
	return *this;
}
string CGetWorkParam::Method() const
{
	return "getwork";
}

// CGetWorkResult::CWork
CGetWorkResult::CWork::CWork() {}
CGetWorkResult::CWork::CWork(const CRPCString& strPrevblockhash, const CRPCUint64& nPrevblocktime, const CRPCInt64& nAlgo, const CRPCInt64& nBits, const CRPCString& strData)
	: strPrevblockhash(strPrevblockhash), nPrevblocktime(nPrevblocktime), nAlgo(nAlgo), nBits(nBits), strData(strData)
{
}
CGetWorkResult::CWork::CWork(const CRPCType& null)
	: strPrevblockhash(null), nPrevblocktime(null), nAlgo(null), nBits(null), strData(null)
{
}
Value CGetWorkResult::CWork::ToJSON() const
{
	Object ret;
	CheckIsValid(strPrevblockhash, "strPrevblockhash");
	ret.push_back(Pair("prevblockhash", std::string(strPrevblockhash)));
	CheckIsValid(nPrevblocktime, "nPrevblocktime");
	ret.push_back(Pair("prevblocktime", uint64(nPrevblocktime)));
	CheckIsValid(nAlgo, "nAlgo");
	ret.push_back(Pair("algo", int64(nAlgo)));
	CheckIsValid(nBits, "nBits");
	ret.push_back(Pair("bits", int64(nBits)));
	CheckIsValid(strData, "strData");
	ret.push_back(Pair("data", std::string(strData)));

	return ret;
}
CGetWorkResult::CWork& CGetWorkResult::CWork::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "CGetWorkResult::CWork");
	auto obj = v.get_obj();
	auto valPrevblockhash = find_value(obj, "prevblockhash");
	CheckJSONType(valPrevblockhash, "string", "prevblockhash");
	strPrevblockhash = valPrevblockhash.get_str();
	auto valPrevblocktime = find_value(obj, "prevblocktime");
	CheckJSONType(valPrevblocktime, "uint", "prevblocktime");
	nPrevblocktime = valPrevblocktime.get_uint64();
	auto valAlgo = find_value(obj, "algo");
	CheckJSONType(valAlgo, "int", "algo");
	nAlgo = valAlgo.get_int64();
	auto valBits = find_value(obj, "bits");
	CheckJSONType(valBits, "int", "bits");
	nBits = valBits.get_int64();
	auto valData = find_value(obj, "data");
	CheckJSONType(valData, "string", "data");
	strData = valData.get_str();
	return *this;
}
bool CGetWorkResult::CWork::IsValid() const
{
	if (!strPrevblockhash.IsValid()) { return false; }
	if (!nPrevblocktime.IsValid()) { return false; }
	if (!nAlgo.IsValid()) { return false; }
	if (!nBits.IsValid()) { return false; }
	if (!strData.IsValid()) { return false; }
	return true;
}

// CGetWorkResult
CGetWorkResult::CGetWorkResult() {}
CGetWorkResult::CGetWorkResult(const CRPCBool& fResult, const CWork& work)
	: fResult(fResult), work(work)
{
}
Value CGetWorkResult::ToJSON() const
{
	Object ret;
	if (fResult.IsValid())
	{
		ret.push_back(Pair("result", bool(fResult)));
	}
	if (work.IsValid())
	{
		ret.push_back(Pair("work", work.ToJSON()));
	}

	return ret;
}
CGetWorkResult& CGetWorkResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "getwork");
	auto obj = v.get_obj();
	auto valResult = find_value(obj, "result");
	if (!valResult.is_null())
	{
		CheckJSONType(valResult, "bool", "result");
		fResult = valResult.get_bool();
	}
	auto valWork = find_value(obj, "work");
	if (!valWork.is_null())
	{
		CheckJSONType(valWork, "object", "work");
		work.FromJSON(valWork.get_obj());
	}
	return *this;
}
string CGetWorkResult::Method() const
{
	return "getwork";
}

// CGetWorkConfig
CGetWorkConfig::CGetWorkConfig()
{
}
bool CGetWorkConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 2)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPrev;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[prev] type error, needs string");
		}
	}
	return true;
}
string CGetWorkConfig::ListConfig() const
{
	return "";
}
string CGetWorkConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        getwork (\"prev\")\n";
	oss << "\n";
	oss << "If (prev hash) is matched with the current primary chain,returns true\n"
	       "If next block is not generated by proof-of-work,return false\n"
	       "Otherwise, returns formatted proof-of-work parameters to work on:\n"
	       "  \"prevblockhash\" : prevblock hash\n"
	       "  \"prevblocktime\" : prevblock timestamp\n"
	       "  \"algo\" : proof-of-work algorithm: blake2b=1,...\n"
	       "  \"bits\" : proof-of-work difficulty nbits\n"
	       "  \"data\" : work data\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"prev\"                         (string, optional) prev block hash\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"prev\": \"\"                   (string, optional) prev block hash\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\" :\n";
	oss << " {\n";
	oss << "   (if \"prev\" is matched or block is not generated by POW)\n";
	oss << "   \"result\": true|false,        (bool, optional) result\n";
	oss << "   (if \"next block\" is generated by POW)\n";
	oss << "   \"work\":                      (object, optional) work data\n";
	oss << "   {\n";
	oss << "     \"prevblockhash\": \"\",       (string, required) prev block hash\n";
	oss << "     \"prevblocktime\": 0,        (uint, required) prev block time\n";
	oss << "     \"algo\": 0,                 (int, required) algo\n";
	oss << "     \"bits\": 0,                 (int, required) bits\n";
	oss << "     \"data\": \"\"                 (string, required) work data\n";
	oss << "   }\n";
	oss << " }\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli getwork 7ee748e9a827d476d1b4ddb77dc8f9bad779f7b71593d5c5bf73b535e1cc2446\n";
	oss << "<< {\"work\":{\"prevblockhash\":\"f734bb6bc12ab4058532113cfe6a3412d1036eae25f60a97ee1b17effc6e74de\",\"prevblocktime\":1538142032,\"algo\":1,\"bits\":25,\"data\":\"01000100822fae5bde746efcef171bee970af625ae6e03d112346afe3c11328505b42ac16bbb34f74300000000000000000000000000000000000000000000000000000000000000000001190000000000000000000000000000000000000000000000000000000000000000\"}}\n";
	oss << "\n>> curl -d '{\"id\":1,\"method\":\"getwork\",\"jsonrpc\":\"2.0\",\"params\":{\"prev\":\"7ee748e9a827d476d1b4ddb77dc8f9bad779f7b71593d5c5bf73b535e1cc2446\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":1,\"jsonrpc\":\"2.0\",\"result\":{\"work\":{\"prevblockhash\":\"f734bb6bc12ab4058532113cfe6a3412d1036eae25f60a97ee1b17effc6e74de\",\"prevblocktime\":1538142032,\"algo\":1,\"bits\":25,\"data\":\"01000100822fae5bde746efcef171bee970af625ae6e03d112346afe3c11328505b42ac16bbb34f74300000000000000000000000000000000000000000000000000000000000000000001190000000000000000000000000000000000000000000000000000000000000000\"}}}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}

/////////////////////////////////////////////////////
// submitwork

// CSubmitWorkParam
CSubmitWorkParam::CSubmitWorkParam() {}
CSubmitWorkParam::CSubmitWorkParam(const CRPCString& strData, const CRPCString& strSpent, const CRPCString& strPrivkey)
	: strData(strData), strSpent(strSpent), strPrivkey(strPrivkey)
{
}
Value CSubmitWorkParam::ToJSON() const
{
	Object ret;
	CheckIsValid(strData, "strData");
	ret.push_back(Pair("data", std::string(strData)));
	CheckIsValid(strSpent, "strSpent");
	ret.push_back(Pair("spent", std::string(strSpent)));
	CheckIsValid(strPrivkey, "strPrivkey");
	ret.push_back(Pair("privkey", std::string(strPrivkey)));

	return ret;
}
CSubmitWorkParam& CSubmitWorkParam::FromJSON(const Value& v)
{
	CheckJSONType(v, "object", "submitwork");
	auto obj = v.get_obj();
	auto valData = find_value(obj, "data");
	CheckJSONType(valData, "string", "data");
	strData = valData.get_str();
	auto valSpent = find_value(obj, "spent");
	CheckJSONType(valSpent, "string", "spent");
	strSpent = valSpent.get_str();
	auto valPrivkey = find_value(obj, "privkey");
	CheckJSONType(valPrivkey, "string", "privkey");
	strPrivkey = valPrivkey.get_str();
	return *this;
}
string CSubmitWorkParam::Method() const
{
	return "submitwork";
}

// CSubmitWorkResult
CSubmitWorkResult::CSubmitWorkResult() {}
CSubmitWorkResult::CSubmitWorkResult(const CRPCString& strHash)
	: strHash(strHash)
{
}
Value CSubmitWorkResult::ToJSON() const
{
	CheckIsValid(strHash, "strHash");
	Value val;
	val = Value(strHash);
	return val;
}
CSubmitWorkResult& CSubmitWorkResult::FromJSON(const Value& v)
{
	CheckJSONType(v, "string", "hash");
	strHash = v.get_str();
	return *this;
}
string CSubmitWorkResult::Method() const
{
	return "submitwork";
}

// CSubmitWorkConfig
CSubmitWorkConfig::CSubmitWorkConfig()
{
}
bool CSubmitWorkConfig::PostLoad()
{
	if (fHelp)
	{
		return true;
	}

	if (vecCommand.size() > 4)
	{
		throw CRPCException(RPC_PARSE_ERROR, string("too arguments given."));
	}
	auto it = vecCommand.begin();
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strData;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[data] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[data] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strSpent;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[spent] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[spent] is required");
	}
	if (next(it, 1) != vecCommand.end())
	{
		istringstream iss(*++it);
		iss >> strPrivkey;
		if (!iss.eof() || iss.fail())
		{
			throw CRPCException(RPC_PARSE_ERROR, "[privkey] type error, needs string");
		}
	}
	else
	{
		throw CRPCException(RPC_PARSE_ERROR, "[privkey] is required");
	}
	return true;
}
string CSubmitWorkConfig::ListConfig() const
{
	return "";
}
string CSubmitWorkConfig::Help() const
{
	std::ostringstream oss;
	oss << "\nUsage:\n";
	oss << "        submitwork <\"data\"> <\"spent\"> <\"privkey\">\n";
	oss << "\n";
	oss << "Attempts to construct and submit new block to network\n"
	       "Return hash of new block.\n";
	oss << "\n";
	oss << "Arguments:\n";
	oss << " \"data\"                         (string, required) work data\n";
	oss << " \"spent\"                        (string, required) spent address\n";
	oss << " \"privkey\"                      (string, required) private key\n";
	oss << "\n";
	oss << "Request:\n";
	oss << " \"param\" :\n";
	oss << " {\n";
	oss << "   \"data\": \"\",                  (string, required) work data\n";
	oss << "   \"spent\": \"\",                 (string, required) spent address\n";
	oss << "   \"privkey\": \"\"                (string, required) private key\n";
	oss << " }\n";
	oss << "\n";
	oss << "Response:\n";
	oss << " \"result\": \"hash\"               (string, required) block hash\n";
	oss << "\n";
	oss << "Examples:\n";
	oss << ">> multiverse-cli submitwork 01000100502fae5b4624cce135b573bfc5d59315b7f779d7baf9c87db7ddb4d176d427a8e948e77e43000000000000000000000000000000000000000000000000000000000000000000011acfff020000000000000000000000000000000000000000000000000000000000 1dj5qcjst7eh4tems36n1m500hhyba3vx436t4a8hgdm7r7jrdbf2yqp9 41a9f94395ced97d5066e2d099df4f1e2bd96057f9c38e8ea3f8a02eccd0a98e\n";
	oss << "<< f734bb6bc12ab4058532113cfe6a3412d1036eae25f60a97ee1b17effc6e74de\n";
	oss << "\n>> curl -d '{\"id\":2,\"method\":\"submitwork\",\"jsonrpc\":\"2.0\",\"params\":{\"data\":\"01000100502fae5b4624cce135b573bfc5d59315b7f779d7baf9c87db7ddb4d176d427a8e948e77e43000000000000000000000000000000000000000000000000000000000000000000011acfff020000000000000000000000000000000000000000000000000000000000\",\"spent\":\"1dj5qcjst7eh4tems36n1m500hhyba3vx436t4a8hgdm7r7jrdbf2yqp9\",\"privkey\":\"41a9f94395ced97d5066e2d099df4f1e2bd96057f9c38e8ea3f8a02eccd0a98e\"}}' http://127.0.0.1:6812\n";
	oss << "<< {\"id\":2,\"jsonrpc\":\"2.0\",\"result\":\"f734bb6bc12ab4058532113cfe6a3412d1036eae25f60a97ee1b17effc6e74de\"}\n";
	oss << "\n";
	oss << "Errors:\n";
	oss << "\tnone\n\n";
	return oss.str();
}


}  // namespace rpc

}  // namespace multiverse

