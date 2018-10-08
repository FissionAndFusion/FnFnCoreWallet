# command line options generator

## scope
The options configuration of command line. e.g. *-rpcport*, *-dbhost* ...

## files
- **script/options.json**: configuration file.
- **srcipt/options_generator.py**: generator script.
- **src/mode/auto_options.h**: target cpp file.

## usage
1. configure **script/options.json**
2. run **srcipt/options_generator.py** or **srcipt/generator.py**(all generator entry)
3. check **src/mode/auto_options.h**.
```c++
// class CMvNetworkConfig in src/mode/network_config.h
class CMvNetworkConfig : virtual public CMvBasicConfig,
                         virtual public CMvNetworkConfigOption

// class CMvNetworkConfigOption in src/mode/auto_options.h
class CMvNetworkConfigOption
```
4. build project

### **script/options.json**
```json
// one option class
"CMvRPCServerConfigOption": [   // Class name, content is array
    // first option
    {
        "access": "public",             // (optional, default="public"). The access modifier of c++ class
        "name": "nRPCMaxConnections",   // (required) parameter name
        "type": "unsigned int",         // (required) parameter type
        "opt": "rpcmaxconnections",     // (required) option of command-line
        "default": "DEFAULT_RPC_MAX_CONNECTIONS",           // (optional) default value of parameter
        "format": "-rpcmaxconnections=<num>",               // (required) prefix formatting in help
        "desc": "Set max connections to <num> (default: 5)" // (optional) description in help
    },
    // second option
    {
        "name": "vRPCAllowIP",
        "type": "vector<string>",
        "opt": "rpcallowip",
        "format": "-rpcallowip=<ip>",
        "desc": "Allow JSON-RPC connections from specified <ip> address"
    }
],
// another option class
CMvRPCBasicConfigOption": [           // Class name, content is array
  {
      "access": "protected",          // (optional, default="public"). The access modifier of c++ class
      "name": "nRPCPortInt",          // (required) parameter name
      "type": "int",                  // (required) parameter type
      "opt": "rpcport",               // (required) option of command-line
      "default": 0,                   // (optional) default value of parameter
      "format": "-rpcport=port",      // (required) prefix formatting in help
      "desc": "Listen for JSON-RPC"   // (optional) description in help
  },
  ...     // next option
]
```

### **src/mode/auto_options.h**
```cpp
class CMvRPCServerConfigOption
{
public:
	unsigned int nRPCMaxConnections;
	vector<string> vRPCAllowIP;
protected:
protected:
    // called by CMvRPCServerConfig.Help()
	string HelpImpl() const
	{
        ostringstream oss;
        // corresponding to "format"        "desc" in json
		oss << "  -rpcmaxconnections=<num>                      Set max connections to <num> (default: 5)\n";
		oss << "  -rpcallowip=<ip>                              Allow JSON-RPC connections from specified <ip> address\n";
		return oss.str();
	}
    // called by CMvRPCServerConfig.CMvRPCServerConfig()
	void AddOptionsImpl(boost::program_options::options_description& desc)
	{
		AddOpt(desc, "rpcmaxconnections", nRPCMaxConnections, (unsigned int)(DEFAULT_RPC_MAX_CONNECTIONS));
		AddOpt(desc, "rpcallowip", vRPCAllowIP);
	}
    // called by CMvRPCServerConfig.ListConfig()
	string ListConfigImpl() const
	{
		ostringstream oss;
		oss << "-rpcmaxconnections: " << nRPCMaxConnections << "\n";
		oss << "-rpcallowip: ";
		for (auto& s: vRPCAllowIP)
		{
			oss << s << " ";
		}
		oss << "\n";
		return oss.str();
	}
};
```

### **src/mode/rpc_config.h**
```cpp
// virtual public inherited CMvRPCServerConfigOption
class CMvRPCServerConfig : virtual public CMvRPCBasicConfig, 
                           virtual public CMvRPCServerConfigOption
{
public:
    CMvRPCServerConfig();
    virtual ~CMvRPCServerConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    boost::asio::ip::tcp::endpoint epRPC;
};
```

### **src/mode/rpc_config.cpp**
```cpp
// in construct, call CMvRPCServerConfigOption::AddOptionsImpl()
CMvRPCServerConfig::CMvRPCServerConfig()
{
    po::options_description desc("RPCServer");

    CMvRPCServerConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

// in ListConfig, call CMvRPCServerConfigOption::ListConfigImpl()
std::string CMvRPCServerConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvRPCServerConfigOption::ListConfigImpl();
    oss << "epRPC: " << epRPC << "\n";
    return CMvRPCBasicConfig::ListConfig() + oss.str();
}

// in Help, call CMvRPCServerConfigOption::HelpImpl()
std::string CMvRPCServerConfig::Help() const
{
    return CMvRPCBasicConfig::Help() + CMvRPCServerConfigOption::HelpImpl();
}
```