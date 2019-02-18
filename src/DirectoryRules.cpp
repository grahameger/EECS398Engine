#include "DirectoryRules.h"

using std::set;
using std::string_view;

DirectoryRules::DirectoryRules(string_view name, bool allowed)
: directoryName(name), isAllowed(allowed), hasRule(false)
{
}
