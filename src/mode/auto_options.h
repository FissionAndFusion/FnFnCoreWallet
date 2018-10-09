// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_AUTO_OPTIONS_H
#define MULTIVERSE_AUTO_OPTIONS_H

#include <string>
#include <vector>
#include <sstream>

#include "mode/config_macro.h"

using std::string;
using std::vector;
using std::ostringstream;

namespace multiverse
{
class CMvBasicConfigOption
{
public:
	bool fTestNet;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -testnet                      Use the test network\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "testnet", fTestNet, bool(false));
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -testnet: " << (fTestNet ? "Y" : "N") << "\n";
		return oss.str();
	}
};
class CMvMintConfigOption
{
public:
	string strAddressMPVss;
	string strKeyMPVss;
	string strAddressBlake512;
	string strKeyBlake512;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -mpvssaddress=<address>       MPVSS address\n";
		oss << "  -mpvsskey=<key>               MPVSS key\n";
		oss << "  -blake512address=<address>    POW blake512 address\n";
		oss << "  -blake512key=<key>            POW blake512 key\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "mpvssaddress", strAddressMPVss);
		AddOpt(desc, "mpvsskey", strKeyMPVss);
		AddOpt(desc, "blake512address", strAddressBlake512);
		AddOpt(desc, "blake512key", strKeyBlake512);
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -mpvssaddress: " << strAddressMPVss << "\n";
		oss << " -mpvsskey: " << strKeyMPVss << "\n";
		oss << " -blake512address: " << strAddressBlake512 << "\n";
		oss << " -blake512key: " << strKeyBlake512 << "\n";
		return oss.str();
	}
};
class CMvRPCClientConfigOption
{
public:
	string strRPCConnect;
	unsigned int nRPCConnectTimeout;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -rpchost=<ip>                 Send commands to node running on <ip> (default: 127.0.0.1)\n";
		oss << "  -rpctimeout=<time>            Connection timeout <time> seconds (default: 120)\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "rpchost", strRPCConnect, string("127.0.0.1"));
		AddOpt(desc, "rpctimeout", nRPCConnectTimeout, (unsigned int)(DEFAULT_RPC_CONNECT_TIMEOUT));
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -rpchost: " << strRPCConnect << "\n";
		oss << " -rpctimeout: " << nRPCConnectTimeout << "\n";
		return oss.str();
	}
};
class CMvDbpServerConfigOption
{
public:
	unsigned int nDbpMaxConnections;
	unsigned int nDbpSessionTimeout;
	vector<string> vDbpAllowIP;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -dbpmaxconnections=<num>      Set dbp max connections to <num> (default: 20)\n";
		oss << "  -dbpsessiontimeout=<num>      Set max timeout to <num>  seconds (default: 60*5)\n";
		oss << "  -dbpallowip=<ip>              Allow DBP connections from specified <ip> address\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "dbpmaxconnections", nDbpMaxConnections, (unsigned int)(DEFAULT_DBP_MAX_CONNECTIONS));
		AddOpt(desc, "dbpsessiontimeout", nDbpSessionTimeout, (unsigned int)(DEFAULT_DBP_SESSION_TIMEOUT));
		AddOpt(desc, "dbpallowip", vDbpAllowIP);
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -dbpmaxconnections: " << nDbpMaxConnections << "\n";
		oss << " -dbpsessiontimeout: " << nDbpSessionTimeout << "\n";
		oss << " -dbpallowip: ";
		for (auto& s: vDbpAllowIP)
		{
			oss << s << " ";
		}
		oss << "\n";
		return oss.str();
	}
};
class CMvRPCServerConfigOption
{
public:
	unsigned int nRPCMaxConnections;
	vector<string> vRPCAllowIP;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -rpcmaxconnections=<num>      Set max connections to <num> (default: 5)\n";
		oss << "  -rpcallowip=<ip>              Allow JSON-RPC connections from specified <ip> address\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "rpcmaxconnections", nRPCMaxConnections, (unsigned int)(DEFAULT_RPC_MAX_CONNECTIONS));
		AddOpt(desc, "rpcallowip", vRPCAllowIP);
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -rpcmaxconnections: " << nRPCMaxConnections << "\n";
		oss << " -rpcallowip: ";
		for (auto& s: vRPCAllowIP)
		{
			oss << s << " ";
		}
		oss << "\n";
		return oss.str();
	}
};
class CMvDbpBasicConfigOption
{
public:
	int nDbpPortInt;
	string strDbpUser;
	string strDbpPass;
	bool fDbpSSLEnable;
	bool fDbpSSLVerify;
	string strDbpCAFile;
	string strDbpCertFile;
	string strDbpPKFile;
	string strDbpCiphers;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -dbpport=port                 Listen for DBP connections on <port> (default: 6815)\n";
		oss << "  -dbpuser=<user>               <user> name for DBP connections\n";
		oss << "  -dbppassword=<password>       <password> for DBP connections\n";
		oss << "  -dbpssl                       Use OpenSSL (https) for DBP connections or not (default false)\n";
		oss << "  -nodbpsslverify               Verify SSL or not (default no)\n";
		oss << "  -dbpcafile=<file.cert>        SSL CA file name (default dbpca.crt)\n";
		oss << "  -dbpcertfile=<file.cert>      Server certificate file (default: dbpserver.cert)\n";
		oss << "  -dbppkfile=<file.pem>         Server private key (default: dbpserver.pem)\n";
		oss << "  -dbpciphers=<ciphers>         Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "dbpport", nDbpPortInt, int(DEFAULT_DBPPORT));
		AddOpt(desc, "dbpuser", strDbpUser);
		AddOpt(desc, "dbppassword", strDbpPass);
		AddOpt(desc, "dbpssl", fDbpSSLEnable, bool(false));
		AddOpt(desc, "dbpsslverify", fDbpSSLVerify, bool(false));
		AddOpt(desc, "dbpcafile", strDbpCAFile, string("dbpca.crt"));
		AddOpt(desc, "dbpcertfile", strDbpCertFile, string("dbpserver.crt"));
		AddOpt(desc, "dbppkfile", strDbpPKFile, string("dbpserver.key"));
		AddOpt(desc, "dbpciphers", strDbpCiphers, string("TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH"));
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -dbpport: " << nDbpPortInt << "\n";
		oss << " -dbpuser: " << strDbpUser << "\n";
		oss << " -dbppassword: " << strDbpPass << "\n";
		oss << " -dbpssl: " << (fDbpSSLEnable ? "Y" : "N") << "\n";
		oss << " -dbpsslverify: " << (fDbpSSLVerify ? "Y" : "N") << "\n";
		oss << " -dbpcafile: " << strDbpCAFile << "\n";
		oss << " -dbpcertfile: " << strDbpCertFile << "\n";
		oss << " -dbppkfile: " << strDbpPKFile << "\n";
		oss << " -dbpciphers: " << strDbpCiphers << "\n";
		return oss.str();
	}
};
class CMvRPCBasicConfigOption
{
public:
	int nRPCPortInt;
	string strRPCUser;
	string strRPCPass;
	bool fRPCSSLEnable;
	bool fRPCSSLVerify;
	string strRPCCAFile;
	string strRPCCertFile;
	string strRPCPKFile;
	string strRPCCiphers;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -rpcport=port                 Listen for JSON-RPC connections on <port> (default: 6802)\n";
		oss << "  -rpcuser=<user>               <user> name for JSON-RPC connections\n";
		oss << "  -rpcpassword=<password>       <password> for JSON-RPC connections\n";
		oss << "  -rpcssl                       Use OpenSSL (https) for JSON-RPC connections or not (default false)\n";
		oss << "  -norpcsslverify               Verify SSL or not (default yes)\n";
		oss << "  -rpccafile=<file.cert>        SSL CA file name (default ca.crt)\n";
		oss << "  -rpccertfile=<file.cert>      Server certificate file (default: server.cert)\n";
		oss << "  -rpcpkfile=<file.pem>         Server private key (default: server.pem)\n";
		oss << "  -rpcciphers=<ciphers>         Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "rpcport", nRPCPortInt, int(0));
		AddOpt(desc, "rpcuser", strRPCUser);
		AddOpt(desc, "rpcpassword", strRPCPass);
		AddOpt(desc, "rpcssl", fRPCSSLEnable, bool(false));
		AddOpt(desc, "rpcsslverify", fRPCSSLVerify, bool(false));
		AddOpt(desc, "rpccafile", strRPCCAFile, string("ca.crt"));
		AddOpt(desc, "rpccertfile", strRPCCertFile, string("server.crt"));
		AddOpt(desc, "rpcpkfile", strRPCPKFile, string("server.key"));
		AddOpt(desc, "rpcciphers", strRPCCiphers, string("TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH"));
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -rpcport: " << nRPCPortInt << "\n";
		oss << " -rpcuser: " << strRPCUser << "\n";
		oss << " -rpcpassword: " << strRPCPass << "\n";
		oss << " -rpcssl: " << (fRPCSSLEnable ? "Y" : "N") << "\n";
		oss << " -rpcsslverify: " << (fRPCSSLVerify ? "Y" : "N") << "\n";
		oss << " -rpccafile: " << strRPCCAFile << "\n";
		oss << " -rpccertfile: " << strRPCCertFile << "\n";
		oss << " -rpcpkfile: " << strRPCPKFile << "\n";
		oss << " -rpcciphers: " << strRPCCiphers << "\n";
		return oss.str();
	}
};
class CMvStorageConfigOption
{
public:
	string strDBHost;
	string strDBName;
	string strDBUser;
	string strDBPass;
	int nDBPort;
	int nDBConn;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -dbhost=<host>                Set mysql host (default: localhost)\n";
		oss << "  -dbname=<name>                Set mysql database name (default: multiverse)\n";
		oss << "  -dbuser=<user>                Set mysql user's name (default: multiverse)\n";
		oss << "  -dbpass=<pwd>                 Set mysql user's password (default: multiverse)\n";
		oss << "  -dbport=<n>                   Set mysql port (default: 0)\n";
		oss << "  -dbconn=<n>                   Set mysql connections count (default: 8)\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "dbhost", strDBHost, string("localhost"));
		AddOpt(desc, "dbname", strDBName, string("multiverse"));
		AddOpt(desc, "dbuser", strDBUser, string("multiverse"));
		AddOpt(desc, "dbpass", strDBPass, string("multiverse"));
		AddOpt(desc, "dbport", nDBPort, int(0));
		AddOpt(desc, "dbconn", nDBConn, int(DEFAULT_DB_CONNECTION));
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -dbhost: " << strDBHost << "\n";
		oss << " -dbname: " << strDBName << "\n";
		oss << " -dbuser: " << strDBUser << "\n";
		oss << " -dbpass: " << strDBPass << "\n";
		oss << " -dbport: " << nDBPort << "\n";
		oss << " -dbconn: " << nDBConn << "\n";
		return oss.str();
	}
};
class CMvNetworkConfigOption
{
public:
	bool fListen;
	bool fBloom;
	int nPortInt;
	int nMaxConnection;
	unsigned int nConnectTimeout;
	vector<string> vNode;
	vector<string> vConnectTo;
	int nDNSeedPort;
	unsigned int nMaxTimes2ConnectFail;
	string strTrustAddress;
protected:
protected:
	string HelpImpl() const
	{
		ostringstream oss;
		oss << "  -listen                       Accept connections from outside (default: 1)\n";
		oss << "  -nobloom                      \n";
		oss << "  -port=<port>                  Listen for connections on <port> (default: 6801 or testnet: 6803)\n";
		oss << "  -maxconnections=<n>           Maintain at most <n> connections to peers (default: 125)\n";
		oss << "  -timeout=<n>                  Specify connection timeout (in milliseconds)\n";
		oss << "  -addnode=<ip>                 Add a node to connect to and attempt to keep the connection open\n";
		oss << "  -connect=<ip>                 Connect only to the specified node\n";
		oss << "  -dnseedport=<port>            Listen for dnseed connections on <port> (default: 6816)\n";
		oss << "  -dnseedmaxtimes=<times>       Max <times> dnseed attempt to connect node\n";
		oss << "  -confidentAddress=<address>   Trust node address\n";
		return oss.str();
	}
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "listen", fListen, bool(false));
		AddOpt(desc, "bloom", fBloom, bool(true));
		AddOpt(desc, "port", nPortInt, int(0));
		AddOpt(desc, "maxconnections", nMaxConnection, int(DEFAULT_MAX_OUTBOUNDS + DEFAULT_MAX_INBOUNDS));
		AddOpt(desc, "timeout", nConnectTimeout, (unsigned int)(DEFAULT_CONNECT_TIMEOUT));
		AddOpt(desc, "addnode", vNode);
		AddOpt(desc, "connect", vConnectTo);
		AddOpt(desc, "dnseedport", nDNSeedPort, int(DEFAULT_DNSEED_PORT));
		AddOpt(desc, "dnseedmaxtimes", nMaxTimes2ConnectFail, (unsigned int)(DNSEED_DEFAULT_MAX_TIMES_CONNECT_FAIL));
		AddOpt(desc, "confidentAddress", strTrustAddress);
	}
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << " -listen: " << (fListen ? "Y" : "N") << "\n";
		oss << " -bloom: " << (fBloom ? "Y" : "N") << "\n";
		oss << " -port: " << nPortInt << "\n";
		oss << " -maxconnections: " << nMaxConnection << "\n";
		oss << " -timeout: " << nConnectTimeout << "\n";
		oss << " -addnode: ";
		for (auto& s: vNode)
		{
			oss << s << " ";
		}
		oss << "\n";
		oss << " -connect: ";
		for (auto& s: vConnectTo)
		{
			oss << s << " ";
		}
		oss << "\n";
		oss << " -dnseedport: " << nDNSeedPort << "\n";
		oss << " -dnseedmaxtimes: " << nMaxTimes2ConnectFail << "\n";
		oss << " -confidentAddress: " << strTrustAddress << "\n";
		return oss.str();
	}
};
} // namespace multiverse

#endif // MULTIVERSE_AUTO_OPTIONS_H
