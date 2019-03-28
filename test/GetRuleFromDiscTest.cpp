#include "RobotsTxt.h"
#include <assert.h>
using namespace std;

int main()
{
    string directory = "/";
    DirectoryRules rules(directory);
    string dirName = "/Dennis/Li/IsA/really/coolguy.yaml";
    DirectoryRules *dir = rules.FindOrCreateChild(dirName);
    dirName = "/Dennis/Li/IsA/Goon/ShouldBeTrue/ShouldAlsoBeTrue/Swag/true/false";
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

    dirName = "/Dennis/Li/IsA/Goon/ShouldBeTrue/ShouldAlsoBeTrue/Swag/true/false";
    dir = rules.FindOrCreateChild(dirName);
    dir->SetAllowed(false);

    //save rules tree to file
    FILE *fp = fopen("dennisli.com.txt", "w+");
    rules.SaveToFile(fp);
    string path = "/Dennis/Li/IsA/Goon/ShouldBeTrue.http";
    string domain = "dennisli.com";
    fclose(fp);

    //test retrieval:
    RobotsTxt robots;
    bool isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == true); 

    path = "/Dennis/Li/IsA";
    isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == false); 

    path = "/Dennis/Li/IsA/really";
    isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == true); 

    path = "/Dennis/Li/IsA/Goon";
    isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == false); 

    path = "/Dennis/Li/IsA/Goon/ShouldBeTrue";
    isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == true); 

    path = "/Dennis/Li/IsA/Goon/ShouldBeTrue/ShouldAlsoBeTrue/Swag/true";
    isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == true); 
    
    path = "/Dennis/Li/IsA/Goon/ShouldBeTrue/ShouldAlsoBeTrue/Swag/true/false";
    isAllowed = robots.GetRule(path, domain);
    assert(isAllowed == false); 
}