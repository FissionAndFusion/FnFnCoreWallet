#include <boost/test/unit_test.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <exception>

#include "test_fnfn.h"
#include "walleve/walleve.h"
#include "walleve/netio/nethost.h"

using namespace walleve;

BOOST_FIXTURE_TEST_SUITE(ipv6_tests, BasicUtfSetup)

std::vector<boost::regex> vWhiteList;

bool IsAllow(const std::string& address)
{
    for(const boost::regex& expr : vWhiteList)
    {
        if (boost::regex_match(address, expr))
        {
            return true;
        }
    }

    return false;
}

BOOST_AUTO_TEST_CASE( white_list_regex )
{
    const boost::regex expr("(\\*)|(\\?)|(\\.)");
    const std::string fmt = "(?1.*)(?2.)(?3\\\\.)";
   
    std::vector<std::string> vAllowMask{"192.168.199.*", "2001:db8:0:f101::*", 
        "fe80::95c3:6a70:f4e4:5c2a", "fe80::95c3:6a70:*:fc2a"};
    vWhiteList.clear();
    try{
        for(const auto& mask : vAllowMask)
        {
            std::string strRegex = boost::regex_replace(mask,expr,fmt,
                                                   boost::match_default | boost::format_all);
            vWhiteList.push_back(boost::regex(strRegex));
        }

        std::string targetAddress("192.168.199.1");
        BOOST_CHECK(IsAllow(targetAddress) == true);

        targetAddress = "192.168.196.34";
        BOOST_CHECK(IsAllow(targetAddress) == false);

        targetAddress = "2001:db8:0:f101::1";
        BOOST_CHECK(IsAllow(targetAddress) == true);

        targetAddress = "2001:db8:0:f102::23";
        BOOST_CHECK(IsAllow(targetAddress) == false);

        targetAddress = "::1";
        BOOST_CHECK(IsAllow(targetAddress) == false);

        targetAddress = "fe80::95c3:6a70:f4e4:5c2a";
        BOOST_CHECK(IsAllow(targetAddress) == true);

        targetAddress = "fe80::95c3:6a70:f4e4:5c2f";
        BOOST_CHECK(IsAllow(targetAddress) == false);

        targetAddress = "fe80::95c3:6a70:a231:fc2a";
        BOOST_CHECK(IsAllow(targetAddress) == true);

        targetAddress = "fe80::95c3:6a70:a231:fc21";
        BOOST_CHECK(IsAllow(targetAddress) == false);
    }
    catch(const std::exception& exp)
    {
       BOOST_FAIL("Regex Error.");
    }
   
}

std::string ResolveHost(const CNetHost& host)
{
    
    boost::asio::io_service ioService;
    
    std::stringstream ss;
    ss << host.nPort;
    boost::asio::ip::tcp::resolver::query query(host.strHost,ss.str());
    
    boost::asio::ip::tcp::resolver resolverHost(ioService);

    boost::system::error_code err;
    auto iter = resolverHost.resolve(query,err);
    if(!err)
    {
        return (*iter).endpoint().address().to_string();
    }
    else
    {
        return std::string();
    }
    
}

BOOST_AUTO_TEST_CASE ( dns_query )
{
    CNetHost localhost("localhost",55);
    BOOST_CHECK(ResolveHost(localhost) == "127.0.0.1");

    CNetHost testHost("www.multiverse-test.com",55);
    BOOST_CHECK(ResolveHost(testHost) == "");

    // configure dnsmasq: /etc/dnsmasq.conf (see conf path of file: --conf-dir --conf-file) 
    // OPTION address=/domain.com/[ipv4 | ipv6]
    // refers to http://www.thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html

    // address=/ipv4-multiverse.com/127.0.0.1
    CNetHost v4Host("www.ipv4-multiverse.com",55);
    BOOST_CHECK(ResolveHost(v4Host) == "127.0.0.1");

    // address=/ipv6-multiverse.com/2001:db8:0:23:8:800:200c:417a
    CNetHost v6Host("www.ipv6-multiverse.com",55);
    BOOST_CHECK(ResolveHost(v6Host) == "2001:db8:0:23:8:800:200c:417a");
}

BOOST_AUTO_TEST_SUITE_END()