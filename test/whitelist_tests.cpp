#include <boost/test/unit_test.hpp>
#include <boost/regex.hpp>
#include <exception>

#include "test_fnfn.h"

BOOST_FIXTURE_TEST_SUITE(whitelist_tests, BasicUtfSetup)

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

BOOST_AUTO_TEST_SUITE_END()