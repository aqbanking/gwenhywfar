// This is a unittest for the <gwenhywfar/urlfns.h> function, notably
// GWEN_Url_fromString

#include <stdio.h>
#include <string.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/urlfns.h>

// Global variable for storing the test result
int g_testSuccess = 0;

// Macro to test for one assertion
#define test_assert(expr) if (!(expr)) { printf("%s: Failed test in line %d\n", __FILE__, (int)__LINE__); g_testSuccess = -1; }

// Macro to test for string equality. Since this is C, we also check
// for NULL pointers already here.
#define test_strcmp(str1, str2) if (!(str1) || !(str2) || strcmp((str1), (str2)) != 0) { printf("%s: Failed strcmp test in line %d, \"%s\" != \"%s\"\n", __FILE__, (int)__LINE__, (str1) ? (str1) : "NULL", (str2) ? (str2) : "NULL"); g_testSuccess = -1; }


int main(int argc, char** argv)
{
  GWEN_URL *url = GWEN_Url_fromString("http://www.aqbanking.de/foo/bar");
  test_assert(GWEN_Url_GetPassword(url) == NULL);
  test_strcmp(GWEN_Url_GetPath(url), "/foo/bar");
  test_assert(GWEN_Url_GetPort(url) == 80);
  test_strcmp(GWEN_Url_GetProtocol(url), "http");
  test_strcmp(GWEN_Url_GetServer(url), "www.aqbanking.de");
  test_assert(GWEN_Url_GetUserName(url) == NULL);
  GWEN_Url_free(url);

  url = GWEN_Url_fromString("file:/home/aquamaniac");
  test_strcmp(GWEN_Url_GetPath(url), "/home/aquamaniac");
  test_assert(GWEN_Url_GetPort(url) == 0);
  test_assert(GWEN_Url_GetProtocol(url) == NULL); // no "file" here?
  test_strcmp(GWEN_Url_GetServer(url), "file");
  GWEN_Url_free(url);

  url = GWEN_Url_fromString("/home/aquamaniac");
  test_strcmp(GWEN_Url_GetPath(url), "/home/aquamaniac");
  test_assert(GWEN_Url_GetPort(url) == 0);
  test_assert(GWEN_Url_GetProtocol(url) == NULL);
  test_assert(GWEN_Url_GetServer(url) == NULL); // different from the case above?
  GWEN_Url_free(url);

  return g_testSuccess;
}
