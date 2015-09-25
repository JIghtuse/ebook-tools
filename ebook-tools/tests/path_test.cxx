#include <CppUTest/TestHarness.h>

#include <path.h>

TEST_GROUP(PathNormalization)
{};

void check_remains_the_same(const char* path)
{
    char* result = canonicalize_filename(path);
    STRCMP_EQUAL(path, result);
    free(result);
}

TEST(PathNormalization, NormalizedPathsRemainsTheSame)
{
    check_remains_the_same("");
    check_remains_the_same("some/nested/path");
    check_remains_the_same("toc.ncx");
}

TEST(PathNormalization, EatLeadingSlashes)
{
    const char* path;
    char* result;

    path = "/////";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("", result);
    free(result);

    path = "///some/path";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("some/path", result);
    free(result);
}

TEST(PathNormalization, ReplaceMultipleSlahesWithOne)
{
    const char* path;
    char* result;

    path = "some///nested/////path";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("some/nested/path", result);
    free(result);
}

TEST(PathNormalization, EatsLeadingTwoDots)
{
    const char* path;
    char* result;

    path = "../elements/toc.ncx";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("elements/toc.ncx", result);
    free(result);

    path = "../../elements/toc.ncx";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("elements/toc.ncx", result);
    free(result);
}

TEST(PathNormalization, HandlesTwoDots)
{
    const char* path;
    char* result;

    path = "root/elements/../elements/toc.ncx";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("root/elements/toc.ncx", result);
    free(result);

    path = "root/nested/path/../../elements/toc.ncx";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("root/elements/toc.ncx", result);
    free(result);

    path = "root/nested/path/../elements/../toc.ncx";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("root/nested/toc.ncx", result);
    free(result);

    path = "root/nested/path/../../../toc.ncx";
    result = canonicalize_filename(path);
    STRCMP_EQUAL("toc.ncx", result);
    free(result);
}

/*
  printf("%s\n", canonicalize_filename("/dog/elements/../elements/toc.ncx"));
  printf("%s\n", canonicalize_filename("//elements/../elements/toc.ncx"));
  printf("%s\n", canonicalize_filename("elements/../elements/toc.ncx"));
  printf("%s\n", canonicalize_filename("../elements/toc.ncx"));
  printf("%s\n", canonicalize_filename("/../elements/toc.ncx"));
  printf("%s\n", canonicalize_filename("//dog///../////elements/cat/../toc.ncx"));
  printf("%s\n", canonicalize_filename("//dog///../////elements/dog/../cat/../toc.ncx"));

*/
