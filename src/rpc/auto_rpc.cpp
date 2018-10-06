#include "auto_rpc.h"

#include "rpc/rpc_error.h"

namespace multiverse
{
namespace rpc
{
CRPCParamPtr CreateCRPCParam(const std::string& cmd, const json_spirit::Value& valParam)
{
	if (cmd == "help")
	{
		auto ptr = MakeCHelpParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "stop")
	{
		auto ptr = MakeCStopParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getpeercount")
	{
		auto ptr = MakeCGetPeerCountParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "listpeer")
	{
		auto ptr = MakeCListPeerParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "addnode")
	{
		auto ptr = MakeCAddNodeParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "removenode")
	{
		auto ptr = MakeCRemoveNodeParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getforkcount")
	{
		auto ptr = MakeCGetForkCountParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "listfork")
	{
		auto ptr = MakeCListForkParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getgenealogy")
	{
		auto ptr = MakeCGetGenealogyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getblocklocation")
	{
		auto ptr = MakeCGetBlockLocationParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getblockcount")
	{
		auto ptr = MakeCGetBlockCountParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getblockhash")
	{
		auto ptr = MakeCGetBlockHashParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getblock")
	{
		auto ptr = MakeCGetBlockParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "gettxpool")
	{
		auto ptr = MakeCGetTxPoolParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "removependingtx")
	{
		auto ptr = MakeCRemovePendingTxParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "gettransaction")
	{
		auto ptr = MakeCGetTransactionParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "sendtransaction")
	{
		auto ptr = MakeCSendTransactionParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "listkey")
	{
		auto ptr = MakeCListKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getnewkey")
	{
		auto ptr = MakeCGetNewKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "encryptkey")
	{
		auto ptr = MakeCEncryptKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "lockkey")
	{
		auto ptr = MakeCLockKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "unlockkey")
	{
		auto ptr = MakeCUnlockKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "importprivkey")
	{
		auto ptr = MakeCImportPrivKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "importkey")
	{
		auto ptr = MakeCImportKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "exportkey")
	{
		auto ptr = MakeCExportKeyParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "addnewtemplate")
	{
		auto ptr = MakeCAddNewTemplateParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "importtemplate")
	{
		auto ptr = MakeCImportTemplateParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "exporttemplate")
	{
		auto ptr = MakeCExportTemplateParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "validateaddress")
	{
		auto ptr = MakeCValidateAddressParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "resyncwallet")
	{
		auto ptr = MakeCResyncWalletParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getbalance")
	{
		auto ptr = MakeCGetBalanceParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "listtransaction")
	{
		auto ptr = MakeCListTransactionParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "sendfrom")
	{
		auto ptr = MakeCSendFromParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "createtransaction")
	{
		auto ptr = MakeCCreateTransactionParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "signtransaction")
	{
		auto ptr = MakeCSignTransactionParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "signmessage")
	{
		auto ptr = MakeCSignMessageParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "listaddress")
	{
		auto ptr = MakeCListAddressParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "exportwallet")
	{
		auto ptr = MakeCExportWalletParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "importwallet")
	{
		auto ptr = MakeCImportWalletParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "verifymessage")
	{
		auto ptr = MakeCVerifyMessageParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "makekeypair")
	{
		auto ptr = MakeCMakeKeyPairParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getpubkeyaddress")
	{
		auto ptr = MakeCGetPubkeyAddressParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "gettemplateaddress")
	{
		auto ptr = MakeCGetTemplateAddressParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "maketemplate")
	{
		auto ptr = MakeCMakeTemplateParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "decodetransaction")
	{
		auto ptr = MakeCDecodeTransactionParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "makeorigin")
	{
		auto ptr = MakeCMakeOriginParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "getwork")
	{
		auto ptr = MakeCGetWorkParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else if (cmd == "submitwork")
	{
		auto ptr = MakeCSubmitWorkParamPtr();
		ptr->FromJSON(valParam);
		return ptr;
	}
	else 
	{
		throw CRPCException(RPC_METHOD_NOT_FOUND, cmd + " not found!");
	}
}

CRPCResultPtr CreateCRPCResult(const std::string& cmd, const json_spirit::Value& valResult)
{
	if (cmd == "help")
	{
		auto ptr = MakeCHelpResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "stop")
	{
		auto ptr = MakeCStopResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getpeercount")
	{
		auto ptr = MakeCGetPeerCountResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "listpeer")
	{
		auto ptr = MakeCListPeerResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "addnode")
	{
		auto ptr = MakeCAddNodeResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "removenode")
	{
		auto ptr = MakeCRemoveNodeResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getforkcount")
	{
		auto ptr = MakeCGetForkCountResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "listfork")
	{
		auto ptr = MakeCListForkResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getgenealogy")
	{
		auto ptr = MakeCGetGenealogyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getblocklocation")
	{
		auto ptr = MakeCGetBlockLocationResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getblockcount")
	{
		auto ptr = MakeCGetBlockCountResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getblockhash")
	{
		auto ptr = MakeCGetBlockHashResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getblock")
	{
		auto ptr = MakeCGetBlockResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "gettxpool")
	{
		auto ptr = MakeCGetTxPoolResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "removependingtx")
	{
		auto ptr = MakeCRemovePendingTxResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "gettransaction")
	{
		auto ptr = MakeCGetTransactionResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "sendtransaction")
	{
		auto ptr = MakeCSendTransactionResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "listkey")
	{
		auto ptr = MakeCListKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getnewkey")
	{
		auto ptr = MakeCGetNewKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "encryptkey")
	{
		auto ptr = MakeCEncryptKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "lockkey")
	{
		auto ptr = MakeCLockKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "unlockkey")
	{
		auto ptr = MakeCUnlockKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "importprivkey")
	{
		auto ptr = MakeCImportPrivKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "importkey")
	{
		auto ptr = MakeCImportKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "exportkey")
	{
		auto ptr = MakeCExportKeyResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "addnewtemplate")
	{
		auto ptr = MakeCAddNewTemplateResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "importtemplate")
	{
		auto ptr = MakeCImportTemplateResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "exporttemplate")
	{
		auto ptr = MakeCExportTemplateResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "validateaddress")
	{
		auto ptr = MakeCValidateAddressResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "resyncwallet")
	{
		auto ptr = MakeCResyncWalletResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getbalance")
	{
		auto ptr = MakeCGetBalanceResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "listtransaction")
	{
		auto ptr = MakeCListTransactionResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "sendfrom")
	{
		auto ptr = MakeCSendFromResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "createtransaction")
	{
		auto ptr = MakeCCreateTransactionResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "signtransaction")
	{
		auto ptr = MakeCSignTransactionResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "signmessage")
	{
		auto ptr = MakeCSignMessageResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "listaddress")
	{
		auto ptr = MakeCListAddressResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "exportwallet")
	{
		auto ptr = MakeCExportWalletResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "importwallet")
	{
		auto ptr = MakeCImportWalletResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "verifymessage")
	{
		auto ptr = MakeCVerifyMessageResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "makekeypair")
	{
		auto ptr = MakeCMakeKeyPairResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getpubkeyaddress")
	{
		auto ptr = MakeCGetPubkeyAddressResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "gettemplateaddress")
	{
		auto ptr = MakeCGetTemplateAddressResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "maketemplate")
	{
		auto ptr = MakeCMakeTemplateResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "decodetransaction")
	{
		auto ptr = MakeCDecodeTransactionResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "makeorigin")
	{
		auto ptr = MakeCMakeOriginResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "getwork")
	{
		auto ptr = MakeCGetWorkResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else if (cmd == "submitwork")
	{
		auto ptr = MakeCSubmitWorkResultPtr();
		ptr->FromJSON(valResult);
		return ptr;
	}
	else 
	{
		auto ptr = std::make_shared<CRPCCommonResult>();
		ptr->FromJSON(valResult);
		return ptr;
	}
}

std::string Help(EModeType type, const std::string& subCmd, const std::string& options)
{
	std::ostringstream oss;
	if (type == EModeType::SERVER)
	{
		oss << "\nUsage:\n";
		oss << "        multiverse-server (OPTIONS)\n";
		oss << "\n";
		oss << "Run multiverse server\n";
		oss << "\n";
		oss << "Options:\n";
		oss << options << "\n";
	}
	else if (type == EModeType::CONSOLE)
	{
		if (subCmd.empty())
		{
			if (!options.empty())
			{
				oss << "\nUsage:\n";
				oss << "        multiverse-cli (OPTIONS) COMMAND\n";
				oss << "\n";
				oss << "Run multiverse RPC client\n";
				oss << "\n";
				oss << "Options:\n";
				oss << options << "\n";
			}
			oss << "Commands:\n";
			oss << "  help                          List commands, or get help for a command.\n";
			oss << "  stop                          Stop multiverse server.\n";
			oss << "  getpeercount                  Returns the number of connections to other nodes.\n";
			oss << "  listpeer                      Returns data about each connected network node.\n";
			oss << "  addnode                       Attempts add a node into the addnode list.\n";
			oss << "  removenode                    Attempts remove a node from the addnode list.\n";
			oss << "  getforkcount                  Returns the number of forks.\n";
			oss << "  listfork                      Returns the list of forks.\n";
			oss << "  getgenealogy                  Returns the list of ancestry and subline.\n";
			oss << "  getblocklocation              Returns the location with given block.\n";
			oss << "  getblockcount                 Returns the number of blocks in the given fork.\n";
			oss << "  getblockhash                  Returns hash of block in fork at <index>.\n";
			oss << "  getblock                      Returns details of a block with given block-hash.\n";
			oss << "  gettxpool                     Get transaction pool info\n";
			oss << "  removependingtx               Removes tx whose id is <txid> from txpool.\n";
			oss << "  gettransaction                get transaction info\n";
			oss << "  sendtransaction               Submits raw transaction (serialized, hex-encoded) \n"
			       "                                to local node and network.\n";
			oss << "  listkey                       Returns Object that has pubkey as keys, associated \n"
			       "                                status as values.\n";
			oss << "  getnewkey                     Returns a new pubkey for receiving payments.\n";
			oss << "  encryptkey                    Encrypts the key.\n";
			oss << "  lockkey                       Removes the encryption key from memory, locking \n"
			       "                                the key.\n";
			oss << "  unlockkey                     Unlock the key.\n";
			oss << "  importprivkey                 Adds a private key (as returned by dumpprivkey) \n"
			       "                                to your wallet.\n";
			oss << "  importkey                     Reveals the serialized key corresponding to <pubkey>.\n";
			oss << "  exportkey                     Reveals the serialized key corresponding to <pubkey>.\n";
			oss << "  addnewtemplate                Returns encoded address for the given template id.\n";
			oss << "  importtemplate                Returns encoded address for the given template.\n";
			oss << "  exporttemplate                Returns encoded address for the given template.\n";
			oss << "  validateaddress               Return information about <address>.\n";
			oss << "  resyncwallet                  Resync wallet's transactions.\n";
			oss << "  getbalance                    Get balance of address.\n";
			oss << "  listtransaction               Returns transactions list.\n";
			oss << "  sendfrom                      Send a transaction.\n";
			oss << "  createtransaction             Create a transaction.\n";
			oss << "  signtransaction               Sign a transaction.\n";
			oss << "  signmessage                   Sign a message with the private key of an pubkey\n";
			oss << "  listaddress                   list all of addresses from pub keys and template \n"
			       "                                ids\n";
			oss << "  exportwallet                  Export all of keys and templates from wallet to \n"
			       "                                a specified file in json format.\n";
			oss << "  importwallet                  Import keys and templates from archived file in \n"
			       "                                json format to wallet.\n";
			oss << "  verifymessage                 Verify a signed message\n";
			oss << "  makekeypair                   Make a public/private key pair.\n";
			oss << "  getpubkeyaddress              Returns encoded address for the given pubkey.\n";
			oss << "  gettemplateaddress            Returns encoded address for the given template id.\n";
			oss << "  maketemplate                  Returns encoded address for the given template id.\n";
			oss << "  decodetransaction             Return a JSON object representing the serialized,\n"
			       "                                 hex-encoded transaction.\n";
			oss << "  makeorigin                    Return hex-encoded block.\n";
			oss << "  getwork                       Get mint work\n";
			oss << "  submitwork                    Submit mint work\n";
		}
		if (subCmd == "all" || subCmd == "help")
		{
			oss << CHelpConfig().Help();
		}
		if (subCmd == "all" || subCmd == "stop")
		{
			oss << CStopConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getpeercount")
		{
			oss << CGetPeerCountConfig().Help();
		}
		if (subCmd == "all" || subCmd == "listpeer")
		{
			oss << CListPeerConfig().Help();
		}
		if (subCmd == "all" || subCmd == "addnode")
		{
			oss << CAddNodeConfig().Help();
		}
		if (subCmd == "all" || subCmd == "removenode")
		{
			oss << CRemoveNodeConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getforkcount")
		{
			oss << CGetForkCountConfig().Help();
		}
		if (subCmd == "all" || subCmd == "listfork")
		{
			oss << CListForkConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getgenealogy")
		{
			oss << CGetGenealogyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getblocklocation")
		{
			oss << CGetBlockLocationConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getblockcount")
		{
			oss << CGetBlockCountConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getblockhash")
		{
			oss << CGetBlockHashConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getblock")
		{
			oss << CGetBlockConfig().Help();
		}
		if (subCmd == "all" || subCmd == "gettxpool")
		{
			oss << CGetTxPoolConfig().Help();
		}
		if (subCmd == "all" || subCmd == "removependingtx")
		{
			oss << CRemovePendingTxConfig().Help();
		}
		if (subCmd == "all" || subCmd == "gettransaction")
		{
			oss << CGetTransactionConfig().Help();
		}
		if (subCmd == "all" || subCmd == "sendtransaction")
		{
			oss << CSendTransactionConfig().Help();
		}
		if (subCmd == "all" || subCmd == "listkey")
		{
			oss << CListKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getnewkey")
		{
			oss << CGetNewKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "encryptkey")
		{
			oss << CEncryptKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "lockkey")
		{
			oss << CLockKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "unlockkey")
		{
			oss << CUnlockKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "importprivkey")
		{
			oss << CImportPrivKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "importkey")
		{
			oss << CImportKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "exportkey")
		{
			oss << CExportKeyConfig().Help();
		}
		if (subCmd == "all" || subCmd == "addnewtemplate")
		{
			oss << CAddNewTemplateConfig().Help();
		}
		if (subCmd == "all" || subCmd == "importtemplate")
		{
			oss << CImportTemplateConfig().Help();
		}
		if (subCmd == "all" || subCmd == "exporttemplate")
		{
			oss << CExportTemplateConfig().Help();
		}
		if (subCmd == "all" || subCmd == "validateaddress")
		{
			oss << CValidateAddressConfig().Help();
		}
		if (subCmd == "all" || subCmd == "resyncwallet")
		{
			oss << CResyncWalletConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getbalance")
		{
			oss << CGetBalanceConfig().Help();
		}
		if (subCmd == "all" || subCmd == "listtransaction")
		{
			oss << CListTransactionConfig().Help();
		}
		if (subCmd == "all" || subCmd == "sendfrom")
		{
			oss << CSendFromConfig().Help();
		}
		if (subCmd == "all" || subCmd == "createtransaction")
		{
			oss << CCreateTransactionConfig().Help();
		}
		if (subCmd == "all" || subCmd == "signtransaction")
		{
			oss << CSignTransactionConfig().Help();
		}
		if (subCmd == "all" || subCmd == "signmessage")
		{
			oss << CSignMessageConfig().Help();
		}
		if (subCmd == "all" || subCmd == "listaddress")
		{
			oss << CListAddressConfig().Help();
		}
		if (subCmd == "all" || subCmd == "exportwallet")
		{
			oss << CExportWalletConfig().Help();
		}
		if (subCmd == "all" || subCmd == "importwallet")
		{
			oss << CImportWalletConfig().Help();
		}
		if (subCmd == "all" || subCmd == "verifymessage")
		{
			oss << CVerifyMessageConfig().Help();
		}
		if (subCmd == "all" || subCmd == "makekeypair")
		{
			oss << CMakeKeyPairConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getpubkeyaddress")
		{
			oss << CGetPubkeyAddressConfig().Help();
		}
		if (subCmd == "all" || subCmd == "gettemplateaddress")
		{
			oss << CGetTemplateAddressConfig().Help();
		}
		if (subCmd == "all" || subCmd == "maketemplate")
		{
			oss << CMakeTemplateConfig().Help();
		}
		if (subCmd == "all" || subCmd == "decodetransaction")
		{
			oss << CDecodeTransactionConfig().Help();
		}
		if (subCmd == "all" || subCmd == "makeorigin")
		{
			oss << CMakeOriginConfig().Help();
		}
		if (subCmd == "all" || subCmd == "getwork")
		{
			oss << CGetWorkConfig().Help();
		}
		if (subCmd == "all" || subCmd == "submitwork")
		{
			oss << CSubmitWorkConfig().Help();
		}
	}
	else if (type == EModeType::MINER)
	{
		oss << "\nUsage:\n";
		oss << "        multiverse-miner (OPTIONS)\n";
		oss << "\n";
		oss << "Run multiverse miner\n";
		oss << "\n";
		oss << "Options:\n";
		oss << options << "\n";
	}
	else if (type == EModeType::DNSEED)
	{
		oss << "\nUsage:\n";
		oss << "        multiverse-dnseed (OPTIONS)\n";
		oss << "\n";
		oss << "Run multiverse dnseed\n";
		oss << "\n";
		oss << "Options:\n";
		oss << options << "\n";
	}
	else 
	{
	}
	return oss.str();
}
const std::vector<std::string>& RPCCmdList()
{
	static std::vector<std::string> list = 
	{
		"help",
		"stop",
		"getpeercount",
		"listpeer",
		"addnode",
		"removenode",
		"getforkcount",
		"listfork",
		"getgenealogy",
		"getblocklocation",
		"getblockcount",
		"getblockhash",
		"getblock",
		"gettxpool",
		"removependingtx",
		"gettransaction",
		"sendtransaction",
		"listkey",
		"getnewkey",
		"encryptkey",
		"lockkey",
		"unlockkey",
		"importprivkey",
		"importkey",
		"exportkey",
		"addnewtemplate",
		"importtemplate",
		"exporttemplate",
		"validateaddress",
		"resyncwallet",
		"getbalance",
		"listtransaction",
		"sendfrom",
		"createtransaction",
		"signtransaction",
		"signmessage",
		"listaddress",
		"exportwallet",
		"importwallet",
		"verifymessage",
		"makekeypair",
		"getpubkeyaddress",
		"gettemplateaddress",
		"maketemplate",
		"decodetransaction",
		"makeorigin",
		"getwork",
		"submitwork",
	};
	return list;
}

}  // namespace rpc

}  // namespace multiverse
