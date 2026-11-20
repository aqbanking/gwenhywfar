// This is a unittest for <gwenhywfar/text.h> XML unescaping and UTF-8 counting.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>

static int g_testSuccess = 0;

#define test_assert(expr) if (!(expr)) { printf("%s: Failed test in line %d\n", __FILE__, (int)__LINE__); g_testSuccess = -1; }
#define test_strcmp(str1, str2) if (!(str1) || !(str2) || strcmp((str1), (str2)) != 0) { printf("%s: Failed strcmp test in line %d, \"%s\" != \"%s\"\n", __FILE__, (int)__LINE__, (str1) ? (str1) : "NULL", (str2) ? (str2) : "NULL"); g_testSuccess = -1; }

int main(GWEN_UNUSED int argc, GWEN_UNUSED char **argv)
{
  GWEN_BUFFER *buf;
  char *heapStr;
  int rv;

  buf=GWEN_Buffer_new(0, 64, 0, 1);

  /* Heap-allocated input ending with a valid numeric entity (previously read past NUL) */
  heapStr=strdup("Ref &#65;");
  test_assert(heapStr != NULL);
  rv=GWEN_Text_UnescapeXmlToBuffer(heapStr, buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "Ref A");
  free(heapStr);
  GWEN_Buffer_Reset(buf);

  /* Unterminated numeric entity at end of heap buffer must stay literal and safe */
  heapStr=strdup("Ref &#65");
  test_assert(heapStr != NULL);
  rv=GWEN_Text_UnescapeXmlToBuffer(heapStr, buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "Ref &#65");
  free(heapStr);
  GWEN_Buffer_Reset(buf);

  /* Multi-byte Unicode decimal and hexadecimal entities encode to valid UTF-8 */
  rv=GWEN_Text_UnescapeXmlToBuffer("&#228; &#8364; &#x20AC; &#x1F600;", buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "\xC3\xA4 \xE2\x82\xAC \xE2\x82\xAC \xF0\x9F\x98\x80");
  GWEN_Buffer_Reset(buf);

  /* Invalid scalar values remain literal rather than emitting NUL or surrogates */
  rv=GWEN_Text_UnescapeXmlToBuffer("a&#0;b&#xD800;c", buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "a&#0;b&#xD800;c");
  GWEN_Buffer_free(buf);

  /* UTF-8 continuation bytes must satisfy (b & 0xc0) == 0x80 */
  test_assert(GWEN_Text_CountUtf8Chars("\xC3\xA4\xE2\x82\xAC", 0) == 2);
  test_assert(GWEN_Text_CountUtf8Chars("\xC3x", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xE2\xC3\xA4", 0) == -1);

  return g_testSuccess;
}
