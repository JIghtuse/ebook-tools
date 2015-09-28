#include <CppUTest/TestHarness.h>

#include <string>
#include <url.h>

using namespace std;

TEST_GROUP(UrlDecode)
{};

#define EMPTY ""
#define SOME "some string with % symbol"

TEST(UrlDecode, DoNotTouchStringWithoutCodes)
{
    char empty[] = EMPTY;
    char some[] = SOME;
 
    url_decode(empty, 0);
    STRCMP_EQUAL(EMPTY, empty);
    url_decode(some, sizeof(SOME));
    STRCMP_EQUAL(SOME, some);
}

TEST(UrlDecode, WorksForSpace)
{
    char spaces[] = "%20string%20with%20encoded%20spaces%20";

    url_decode(spaces, sizeof(spaces));
    STRCMP_EQUAL(" string with encoded spaces ", spaces);
}