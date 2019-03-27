#include "RobotsTxt.h"
#define BOOST_TEST_MODULE DirectoryRuleTest
#include <boost/test/included/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_CASE( SubmitRobotsTxtSimple )
{
    HTTPRequest request;
    request.host = "tinder.com";
    string path = "robotsTxt/tinder.com/robots.txt"; //todo replace with real
    //todo finish
}

BOOST_AUTO_TEST_CASE( GetRuleFromCache )
{
    //todo implement once we can submit
}

BOOST_AUTO_TEST_CASE( GetRuleFromDisc )
{
    //WRITE FILE TO DISC. THIS FUNCTION IS TESTED IN DIRECTORYRULESTEST.CPP
    //**directory and all children directories should change (unless hasRule == true)**
    //create directories
    string directory = "/";
    DirectoryRules rules(directory);
    string dirName = "/Dennis/Li/IsA/really/coolguy.yaml";
    DirectoryRules *dir = rules.FindOrCreateChild(dirName);
    dirName = "/Dennis/Li/IsA/Goon/ShouldBeTrue/ShouldAlsoBeTrue";
    dir = rules.FindOrCreateChild(dirName);

    //set permissions
    dirName = "/Dennis/Li/IsA/really";
    dir = rules.FindOrCreateChild(dirName);
    dir->SetAllowed(true);

    dirName = "/Dennis/Li";
    dir = rules.FindOrCreateChild(dirName);
    dir->SetAllowed(false);

    dirName = "/Dennis/Li/IsA/Goon/ShouldBeTrue";
    dir = rules.FindOrCreateChild(dirName);
    dir->SetAllowed(true);

    //save rules tree to file
    FILE *fp = fopen("dennisli.com.txt", "w+");
    rules.SaveToFile(fp);
    string path = "/Dennis/Li/IsA/Goon/ShouldBeTrue.http";
    string domain = "dennisli.com";

    //test retrieval:
    RobotsTxt robots;
    bool isAllowed = robots.GetRule(path, domain);
    BOOST_CHECK_EQUAL(isAllowed, true); 
}
