// This is a unittest for the <gwenhywfar/urlfns.h> function, notably
// GWEN_Url_fromString

#include <stdio.h>
#include <string.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/urlfns.h>

// Global variable for storing the test result
int g_testSuccess = 0;

// Macro to test for one assertion
#define test_assert(expr) if (!(expr)) { printf("%s: Failed test in line %d\n", __FILE__, (int)__LINE__); g_testSuccess = -1; }

// Macro to test for string equality. Since this is C, we also check
// for NULL pointers already here.
#define test_strcmp(str1, str2) if (!(str1) || !(str2) || strcmp((str1), (str2)) != 0) { printf("%s: Failed strcmp test in line %d, \"%s\" != \"%s\"\n", __FILE__, (int)__LINE__, (str1) ? (str1) : "NULL", (str2) ? (str2) : "NULL"); g_testSuccess = -1; }

// Convenience function to check the interesting fields in one single
// GWEN_URL, which is also free'd here in this function already.
int verify_url(GWEN_URL *url,
               const char *path, int port,
               const char *protocol, const char *server)
{
  int prev_g_testSuccess = g_testSuccess;
  g_testSuccess = 0;

  if (path) {
    test_strcmp(GWEN_Url_GetPath(url), path);
  }
  else {
    test_assert(GWEN_Url_GetPath(url) == NULL);
  }

  test_assert(GWEN_Url_GetPort(url) == port);

  if (protocol) {
    test_strcmp(GWEN_Url_GetProtocol(url), protocol);
  }
  else {
    test_assert(GWEN_Url_GetProtocol(url) == NULL);
  }

  if (server) {
    test_strcmp(GWEN_Url_GetServer(url), server);
  }
  else {
    test_assert(GWEN_Url_GetServer(url) == NULL);
  }

  GWEN_Url_free(url);

  {
    int result = !g_testSuccess;
    g_testSuccess = g_testSuccess || prev_g_testSuccess;
    return result;
  }
}

int main(GWEN_UNUSED int argc, GWEN_UNUSED char **argv)
{
  GWEN_URL *url = GWEN_Url_fromString("http://www.aqbanking.de/foo/bar");
  test_assert(GWEN_Url_GetPassword(url) == NULL);
  test_strcmp(GWEN_Url_GetPath(url), "/foo/bar");
  test_assert(GWEN_Url_GetPort(url) == 80);
  test_strcmp(GWEN_Url_GetProtocol(url), "http");
  test_strcmp(GWEN_Url_GetServer(url), "www.aqbanking.de");
  test_assert(GWEN_Url_GetUserName(url) == NULL);
  GWEN_Url_free(url);

  // Check some error conditions: No Server, should return NULL
  test_assert(GWEN_Url_fromString("http://") == NULL);
  // No server after user, should return NULL
  test_assert(GWEN_Url_fromString("http://cs@") == NULL);
  // No numerical port number, should return NULL
  test_assert(GWEN_Url_fromString("http://a.b.c:aa/foo") == NULL);
  DBG_WARN(GWEN_LOGDOMAIN, "The 3 error messages above are just fine - all works as expected.");

  // And some very weird URL
  url = GWEN_Url_fromString("rsync://foo:bar@a.b.c.d.e.f:4711/some space /in here");
  test_strcmp(GWEN_Url_GetProtocol(url), "rsync");
  test_strcmp(GWEN_Url_GetUserName(url), "foo");
  test_strcmp(GWEN_Url_GetPassword(url), "bar");
  test_strcmp(GWEN_Url_GetServer(url), "a.b.c.d.e.f");
  test_assert(GWEN_Url_GetPort(url) == 4711);
  test_strcmp(GWEN_Url_GetPath(url), "/some space /in here");
  GWEN_Url_free(url);

  url = GWEN_Url_fromString("file:/home/aquamaniac");
  test_strcmp(GWEN_Url_GetPath(url), "/home/aquamaniac");
  test_assert(GWEN_Url_GetPort(url) == 0);
  test_assert(GWEN_Url_GetProtocol(url) == NULL); // no "file" here? Probably correct because of missing extra slashes.
  test_strcmp(GWEN_Url_GetServer(url), "file");
  GWEN_Url_free(url);

  test_assert((verify_url(GWEN_Url_fromString("file:///home/aquamaniac"),
                          /*GetPath*/ "/home/aquamaniac",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ "file", // now the protocol it is here
                          /*GetServer*/   NULL)));

  test_assert((verify_url(GWEN_Url_fromString("/home/aquamaniac"),
                          /*GetPath*/ "/home/aquamaniac",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ NULL,
                          /*GetServer*/   NULL)));

  test_assert((verify_url(GWEN_Url_fromString("dir:///home/aquamaniac/.aqbanking/settings"),
                          /*GetPath*/ "/home/aquamaniac/.aqbanking/settings",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ "dir",
                          /*GetServer*/   NULL)));

  // Also some windows paths
  test_assert((verify_url(GWEN_Url_fromString("c:/home/aquamaniac"),
                          /*GetPath*/ "c:/home/aquamaniac",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ NULL,
                          /*GetServer*/   NULL)));

  test_assert((verify_url(GWEN_Url_fromString("c:\\home\\aquamaniac"),
                          /*GetPath*/ "c:\\home\\aquamaniac",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ NULL,
                          /*GetServer*/   NULL)));

  // This path caused the crash in the windows gnucash binary; is
  // fixed now.
  test_assert((verify_url(GWEN_Url_fromString("dir://c:\\home\\aquamaniac"),
                          /*GetPath*/ "c:\\home\\aquamaniac",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ "dir",
                          /*GetServer*/   NULL)));

  test_assert((verify_url(GWEN_Url_fromString("file://c:\\home\\aquamaniac"),
                          /*GetPath*/ "c:\\home\\aquamaniac",
                          /*GetPort*/ 0,
                          /*GetProtocol*/ "file",
                          /*GetServer*/   NULL)));

  {
    GWEN_URL *url;

    /* Authority '@' must not cross into path, query, or fragment */
    url = GWEN_Url_fromString("https://legit.example/path/@attacker.invalid");
    test_assert(url != NULL);
    test_assert(GWEN_Url_GetUserName(url) == NULL);
    test_assert(GWEN_Url_GetPassword(url) == NULL);
    test_strcmp(GWEN_Url_GetProtocol(url), "https");
    test_strcmp(GWEN_Url_GetServer(url), "legit.example");
    test_assert(GWEN_Url_GetPort(url) == 443);
    test_strcmp(GWEN_Url_GetPath(url), "/path/@attacker.invalid");
    GWEN_Url_free(url);

    url = GWEN_Url_fromString("https://legit.example?user=@attacker.invalid&role=admin#frag");
    test_assert(url != NULL);
    test_assert(GWEN_Url_GetUserName(url) == NULL);
    test_strcmp(GWEN_Url_GetServer(url), "legit.example");
    test_assert(GWEN_Url_GetPort(url) == 443);
    test_strcmp(GWEN_DB_GetCharValue(GWEN_Url_GetVars(url), "user", 0, ""), "@attacker.invalid");
    test_strcmp(GWEN_DB_GetCharValue(GWEN_Url_GetVars(url), "role", 0, ""), "admin");
    GWEN_Url_free(url);

    url = GWEN_Url_fromString("https://legit.example:8443#@attacker.invalid");
    test_assert(url != NULL);
    test_assert(GWEN_Url_GetUserName(url) == NULL);
    test_strcmp(GWEN_Url_GetServer(url), "legit.example");
    test_assert(GWEN_Url_GetPort(url) == 8443);
    test_strcmp(GWEN_Url_GetPath(url), "/");
    GWEN_Url_free(url);

    url = GWEN_Url_fromString("https://legit.example#attacker.invalid");
    test_assert(url != NULL);
    test_strcmp(GWEN_Url_GetServer(url), "legit.example");
    test_assert(GWEN_Url_GetPort(url) == 443);
    GWEN_Url_free(url);

    /* RFC 3986 empty port after colon selects scheme default port */
    url = GWEN_Url_fromString("https://host:/path");
    test_assert(url != NULL);
    test_strcmp(GWEN_Url_GetProtocol(url), "https");
    test_strcmp(GWEN_Url_GetServer(url), "host");
    test_assert(GWEN_Url_GetPort(url) == 443);
    test_strcmp(GWEN_Url_GetPath(url), "/path");
    GWEN_Url_free(url);

    /* IPv6 literals and strict authority/port validation */
    url = GWEN_Url_fromString("https://[2001:db8::1]:8443/fints");
    test_assert(url != NULL);
    test_strcmp(GWEN_Url_GetServer(url), "[2001:db8::1]");
    test_assert(GWEN_Url_GetPort(url) == 8443);
    test_strcmp(GWEN_Url_GetPath(url), "/fints");
    GWEN_Url_free(url);

    url = GWEN_Url_fromString("http://[2001:db8::1]:/fints");
    test_assert(url != NULL);
    test_strcmp(GWEN_Url_GetServer(url), "[2001:db8::1]");
    test_assert(GWEN_Url_GetPort(url) == 80);
    test_strcmp(GWEN_Url_GetPath(url), "/fints");
    GWEN_Url_free(url);

    url = GWEN_Url_fromString("https://[fe80::1%eth0]:8443/fints");
    test_assert(url != NULL);
    test_strcmp(GWEN_Url_GetServer(url), "[fe80::1%eth0]");
    test_assert(GWEN_Url_GetPort(url) == 8443);
    test_strcmp(GWEN_Url_GetPath(url), "/fints");
    GWEN_Url_free(url);
  }

  test_assert(GWEN_Url_fromString(NULL) == NULL);
  test_assert(GWEN_Url_fromString("") == NULL);
  test_assert(GWEN_Url_fromString("https://user@/path") == NULL);
  test_assert(GWEN_Url_fromString("https://user@:443/path") == NULL);
  test_assert(GWEN_Url_fromString("https:///path") == NULL);
  test_assert(GWEN_Url_fromString("https://host:0/path") == NULL);
  test_assert(GWEN_Url_fromString("https://host:-80/path") == NULL);
  test_assert(GWEN_Url_fromString("https://host:70000/path") == NULL);
  test_assert(GWEN_Url_fromString("https://host:123456/path") == NULL);
  test_assert(GWEN_Url_fromString("https://host:443abc/path") == NULL);
  test_assert(GWEN_Url_fromString("https://[::1/path") == NULL);
  test_assert(GWEN_Url_fromString("https://[::1]evil/path") == NULL);
  test_assert(GWEN_Url_fromString("https://[[::1]/path") == NULL);
  test_assert(GWEN_Url_fromString("https://[fe80::1%]/path") == NULL);
  test_assert(GWEN_Url_fromString("https://[bad host]/path") == NULL);

  return g_testSuccess;
}
