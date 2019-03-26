#define BOOST_TEST_MODULE LruCacheTest
#include <boost/test/included/unit_test.hpp>
#include "LRUCache.hpp"

using std::string;

BOOST_AUTO_TEST_CASE( putGetNoReplacement )
{
    //necessary because everything is passed by ref
    string dennis = "dennis";
    string ay = "ay";
    string space = " ";

    int five = 5;
    int nine = 9;
    int negFour = -4;

    LRUCache<string, int> cache(3);
    cache.put(dennis, five);
    cache.put(ay, nine);
    cache.put(space, negFour);

    bool searchFailed = false;
    try
    {
        cache.get(dennis);
    }
    catch (int exception)
    {
        searchFailed = true;
    }

    BOOST_CHECK_EQUAL(cache.get(dennis), 5);
    BOOST_CHECK_EQUAL(cache.get(space), -4);
    BOOST_CHECK_EQUAL(cache.get(ay), 9);
    BOOST_CHECK_EQUAL(searchFailed, false);
}

BOOST_AUTO_TEST_CASE( putGetWithReplacement )
{
    //necessary because everything is passed by ref
    string dennis = "dennis";
    string ay = "ay";
    string space = " ";
    string ga = "ga";

    int five = 5;
    int nine = 9;
    int negFour = -4;
    int ten = 10;

    LRUCache<string, int> cache(3);
    cache.put(dennis, five);
    cache.put(ay, nine);
    cache.put(space, negFour);
    cache.put(ga, ten);

    bool searchFailed = false;
    try
    {
        cache.get(dennis);
    }
    catch (int exception)
    {
        searchFailed = true;
    }

    BOOST_CHECK_EQUAL(searchFailed, true);
    BOOST_CHECK_EQUAL(cache.get(space), -4);
    BOOST_CHECK_EQUAL(cache.get(ay), 9);
    BOOST_CHECK_EQUAL(cache.get(ga), 10);
}

BOOST_AUTO_TEST_CASE( updateRecency )
{
    //necessary because everything is passed by ref
    string dennis = "dennis";
    string ay = "ay";
    string space = " ";
    string ga = "ga";

    int five = 5;
    int nine = 9;
    int negFour = -4;
    int ten = 10;
    int eleven = 11;

    LRUCache<string, int> cache(3);
    cache.put(dennis, five);
    cache.put(ay, nine);
    cache.put(space, negFour);
    int junk = cache.get(dennis);
    cache.put(ga, ten);
    cache.put(ga, eleven);

    bool searchFailed = false;
    try
    {
        cache.get(ay);
    }
    catch (int exception)
    {
        searchFailed = true;
    }

    BOOST_CHECK_EQUAL(searchFailed, true);
    BOOST_CHECK_EQUAL(cache.get(dennis), 5);
    BOOST_CHECK_EQUAL(cache.get(space), -4);
    BOOST_CHECK_EQUAL(cache.get(ga), 11);
}