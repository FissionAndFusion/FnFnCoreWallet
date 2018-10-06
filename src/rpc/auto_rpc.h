// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_RPC_AUTO_RPC_H
#define MULTIVERSE_RPC_AUTO_RPC_H

#include <tuple>
#include "mode/mode_type.h"
#include "mode/mode_impl.h"
#include "rpc/rpc_error.h"
#include "rpc/rpc_req.h"
#include "rpc/rpc_resp.h"
#include "rpc/auto_protocol.h"

namespace multiverse
{
namespace rpc
{
// Create family class shared ptr of CRPCParam
CRPCParamPtr CreateCRPCParam(const std::string& cmd, const json_spirit::Value& valParam);

// Create family class shared ptr of CRPCResult
CRPCResultPtr CreateCRPCResult(const std::string& cmd, const json_spirit::Value& valResult);

// Return help info by mode type and sub command
std::string Help(EModeType type, const std::string& subCmd, const std::string& options = "");

// Dynamic create combin config with rpc command
template <typename... T>
CMvBasicConfig* CreateConfig(const std::string& cmd)
{
	if (cmd == "help")
	{
		return new mode_impl::CCombinConfig<CHelpConfig, T...>;
	}
	else if (cmd == "stop")
	{
		return new mode_impl::CCombinConfig<CStopConfig, T...>;
	}
	else if (cmd == "getpeercount")
	{
		return new mode_impl::CCombinConfig<CGetPeerCountConfig, T...>;
	}
	else if (cmd == "listpeer")
	{
		return new mode_impl::CCombinConfig<CListPeerConfig, T...>;
	}
	else if (cmd == "addnode")
	{
		return new mode_impl::CCombinConfig<CAddNodeConfig, T...>;
	}
	else if (cmd == "removenode")
	{
		return new mode_impl::CCombinConfig<CRemoveNodeConfig, T...>;
	}
	else if (cmd == "getforkcount")
	{
		return new mode_impl::CCombinConfig<CGetForkCountConfig, T...>;
	}
	else if (cmd == "listfork")
	{
		return new mode_impl::CCombinConfig<CListForkConfig, T...>;
	}
	else if (cmd == "getgenealogy")
	{
		return new mode_impl::CCombinConfig<CGetGenealogyConfig, T...>;
	}
	else if (cmd == "getblocklocation")
	{
		return new mode_impl::CCombinConfig<CGetBlockLocationConfig, T...>;
	}
	else if (cmd == "getblockcount")
	{
		return new mode_impl::CCombinConfig<CGetBlockCountConfig, T...>;
	}
	else if (cmd == "getblockhash")
	{
		return new mode_impl::CCombinConfig<CGetBlockHashConfig, T...>;
	}
	else if (cmd == "getblock")
	{
		return new mode_impl::CCombinConfig<CGetBlockConfig, T...>;
	}
	else if (cmd == "gettxpool")
	{
		return new mode_impl::CCombinConfig<CGetTxPoolConfig, T...>;
	}
	else if (cmd == "removependingtx")
	{
		return new mode_impl::CCombinConfig<CRemovePendingTxConfig, T...>;
	}
	else if (cmd == "gettransaction")
	{
		return new mode_impl::CCombinConfig<CGetTransactionConfig, T...>;
	}
	else if (cmd == "sendtransaction")
	{
		return new mode_impl::CCombinConfig<CSendTransactionConfig, T...>;
	}
	else if (cmd == "listkey")
	{
		return new mode_impl::CCombinConfig<CListKeyConfig, T...>;
	}
	else if (cmd == "getnewkey")
	{
		return new mode_impl::CCombinConfig<CGetNewKeyConfig, T...>;
	}
	else if (cmd == "encryptkey")
	{
		return new mode_impl::CCombinConfig<CEncryptKeyConfig, T...>;
	}
	else if (cmd == "lockkey")
	{
		return new mode_impl::CCombinConfig<CLockKeyConfig, T...>;
	}
	else if (cmd == "unlockkey")
	{
		return new mode_impl::CCombinConfig<CUnlockKeyConfig, T...>;
	}
	else if (cmd == "importprivkey")
	{
		return new mode_impl::CCombinConfig<CImportPrivKeyConfig, T...>;
	}
	else if (cmd == "importkey")
	{
		return new mode_impl::CCombinConfig<CImportKeyConfig, T...>;
	}
	else if (cmd == "exportkey")
	{
		return new mode_impl::CCombinConfig<CExportKeyConfig, T...>;
	}
	else if (cmd == "addnewtemplate")
	{
		return new mode_impl::CCombinConfig<CAddNewTemplateConfig, T...>;
	}
	else if (cmd == "importtemplate")
	{
		return new mode_impl::CCombinConfig<CImportTemplateConfig, T...>;
	}
	else if (cmd == "exporttemplate")
	{
		return new mode_impl::CCombinConfig<CExportTemplateConfig, T...>;
	}
	else if (cmd == "validateaddress")
	{
		return new mode_impl::CCombinConfig<CValidateAddressConfig, T...>;
	}
	else if (cmd == "resyncwallet")
	{
		return new mode_impl::CCombinConfig<CResyncWalletConfig, T...>;
	}
	else if (cmd == "getbalance")
	{
		return new mode_impl::CCombinConfig<CGetBalanceConfig, T...>;
	}
	else if (cmd == "listtransaction")
	{
		return new mode_impl::CCombinConfig<CListTransactionConfig, T...>;
	}
	else if (cmd == "sendfrom")
	{
		return new mode_impl::CCombinConfig<CSendFromConfig, T...>;
	}
	else if (cmd == "createtransaction")
	{
		return new mode_impl::CCombinConfig<CCreateTransactionConfig, T...>;
	}
	else if (cmd == "signtransaction")
	{
		return new mode_impl::CCombinConfig<CSignTransactionConfig, T...>;
	}
	else if (cmd == "signmessage")
	{
		return new mode_impl::CCombinConfig<CSignMessageConfig, T...>;
	}
	else if (cmd == "listaddress")
	{
		return new mode_impl::CCombinConfig<CListAddressConfig, T...>;
	}
	else if (cmd == "exportwallet")
	{
		return new mode_impl::CCombinConfig<CExportWalletConfig, T...>;
	}
	else if (cmd == "importwallet")
	{
		return new mode_impl::CCombinConfig<CImportWalletConfig, T...>;
	}
	else if (cmd == "makeorigin")
	{
		return new mode_impl::CCombinConfig<CMakeOriginConfig, T...>;
	}
	else if (cmd == "verifymessage")
	{
		return new mode_impl::CCombinConfig<CVerifyMessageConfig, T...>;
	}
	else if (cmd == "makekeypair")
	{
		return new mode_impl::CCombinConfig<CMakeKeyPairConfig, T...>;
	}
	else if (cmd == "getpubkeyaddress")
	{
		return new mode_impl::CCombinConfig<CGetPubkeyAddressConfig, T...>;
	}
	else if (cmd == "gettemplateaddress")
	{
		return new mode_impl::CCombinConfig<CGetTemplateAddressConfig, T...>;
	}
	else if (cmd == "maketemplate")
	{
		return new mode_impl::CCombinConfig<CMakeTemplateConfig, T...>;
	}
	else if (cmd == "decodetransaction")
	{
		return new mode_impl::CCombinConfig<CDecodeTransactionConfig, T...>;
	}
	else if (cmd == "getwork")
	{
		return new mode_impl::CCombinConfig<CGetWorkConfig, T...>;
	}
	else if (cmd == "submitwork")
	{
		return new mode_impl::CCombinConfig<CSubmitWorkConfig, T...>;
	}
	else 
	{
		return new mode_impl::CCombinConfig<T...>();
	}
}

// All rpc command list
const std::vector<std::string>& RPCCmdList();

// help tips used when error occured or help
static const string strHelpTips = "\n\nRun 'help COMMAND' for more information on a command.\n";

}  // namespace rpc

}  // namespace multiverse

#endif  // MULTIVERSE_RPC_AUTO_RPC_H
