#define BOOST_TEST_MODULE DirectoryRuleTest
#include <boost/test/included/unit_test.hpp>
#include "../../include/DirectoryRules.h"
using namespace std;

BOOST_AUTO_TEST_CASE( createEmptyDirectoryRule )
{
    string directory = "/";
    DirectoryRules rules(directory);
    BOOST_CHECK_EQUAL(rules.GetAllowed(), true);
}

BOOST_AUTO_TEST_CASE( addDirectory )
{
    string directory = "/";
    DirectoryRules rules(directory);
    directory = "/Dennis.txt";
    DirectoryRules *dir = rules.FindOrCreateChild(directory);
    BOOST_CHECK_EQUAL(dir->GetDirectoryName(), "Dennis.txt"); 
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 
}

BOOST_AUTO_TEST_CASE( createMultipleDirectoryRules )
{
    string directory = "/";
    DirectoryRules rules(directory);
    string dirName = "/Dennis/IsA/coolguy.yaml";
    DirectoryRules *dir = rules.FindOrCreateChild(dirName);

    BOOST_CHECK_EQUAL(dir->GetDirectoryName(), "coolguy.yaml"); 
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis/IsA";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetDirectoryName(), "IsA"); 
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetDirectoryName(), "Dennis"); 
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetDirectoryName(), "/"); 
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 
}

BOOST_AUTO_TEST_CASE( changePermissionsOfDirectory )
{
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

    //Test permissions
    dirName = "/";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis/Li";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), false); 

    dirName = "/Dennis/Li/IsA";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), false); 

    dirName = "/Dennis/Li/IsA/Goon";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), false); 

    dirName = "/Dennis/Li/IsA/Goon/ShouldBeTrue";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis/Li/IsA/Goon/ShouldBeTrue/ShouldAlsoBeTrue";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis/Li/IsA/really";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 

    dirName = "/Dennis/Li/IsA/really/coolguy.yaml";
    dir = rules.FindOrCreateChild(dirName);
    BOOST_CHECK_EQUAL(dir->GetAllowed(), true); 
}

BOOST_AUTO_TEST_CASE( saveAndRestoreFile )
{
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
    FILE *fp = fopen("saveAndRestoreFileTest.txt", "w+");
    rules.SaveToFile(fp);


}