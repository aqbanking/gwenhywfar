/* Unit tests for <gwenhywfar/text.h> XML unescaping and UTF-8 counting. */

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
  GWEN_BUFFER *fixed;
  char *heapStr;
  char raw[16];
  int rv;

  buf=GWEN_Buffer_new(0, 64, 0, 1);
  test_assert(buf != NULL);

  /* Null argument handling */
  test_assert(GWEN_Text_UnescapeXmlToBuffer(NULL, buf) < 0);
  test_assert(GWEN_Text_UnescapeXmlToBuffer("abc", NULL) < 0);

  /* Empty input */
  rv=GWEN_Text_UnescapeXmlToBuffer("", buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "");
  GWEN_Buffer_Reset(buf);

  /* Heap-allocated input ending with a valid numeric entity (previously read past NUL) */
  heapStr=strdup("Ref &#65;");
  test_assert(heapStr != NULL);
  rv=GWEN_Text_UnescapeXmlToBuffer(heapStr, buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "Ref A");
  free(heapStr);
  GWEN_Buffer_Reset(buf);

  /* Unterminated numeric entities and bare '&' must stay literal without OOB read */
  heapStr=strdup("Ref &#65");
  test_assert(heapStr != NULL);
  rv=GWEN_Text_UnescapeXmlToBuffer(heapStr, buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "Ref &#65");
  free(heapStr);
  GWEN_Buffer_Reset(buf);

  heapStr=strdup("Hex &#x41");
  test_assert(heapStr != NULL);
  rv=GWEN_Text_UnescapeXmlToBuffer(heapStr, buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "Hex &#x41");
  free(heapStr);
  GWEN_Buffer_Reset(buf);

  rv=GWEN_Text_UnescapeXmlToBuffer("A & B &#; &#xyz;", buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "A & B &#; &#xyz;");
  GWEN_Buffer_Reset(buf);

  /* Named and multi-byte Unicode decimal/hex entities decode to UTF-8 */
  rv=GWEN_Text_UnescapeXmlToBuffer("&amp;&lt;&gt;&quot;&apos; &#228; &#8364; &#x20AC; &#x1F600;", buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "&<>\"' \xC3\xA4 \xE2\x82\xAC \xE2\x82\xAC \xF0\x9F\x98\x80");
  GWEN_Buffer_Reset(buf);

  /* Invalid scalar values, surrogates, NUL, and overflows remain literal */
  rv=GWEN_Text_UnescapeXmlToBuffer("a&#0;b&#x0;c&#xD800;d&#x110000;e&#99999999999999;f", buf);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(buf), "a&#0;b&#x0;c&#xD800;d&#x110000;e&#99999999999999;f");
  GWEN_Buffer_free(buf);

  /* Canary-guarded exact-fit fixed buffer ("Ref A" + NUL = 6 bytes) */
  memset(raw, 'Z', sizeof(raw));
  fixed=GWEN_Buffer_new(raw, 6, 0, 0);
  test_assert(fixed != NULL);
  GWEN_Buffer_SubMode(fixed, GWEN_BUFFER_MODE_DYNAMIC | GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  rv=GWEN_Text_UnescapeXmlToBuffer("Ref &#65;", fixed);
  test_assert(rv == 0);
  test_strcmp(GWEN_Buffer_GetStart(fixed), "Ref A");
  test_assert(memcmp(raw + 6, "ZZZZZZZZZZ", 10) == 0);
  GWEN_Buffer_free(fixed);

  /* Canary-guarded overflow on undersized fixed buffer returns error without corrupting tail */
  memset(raw, 'Z', sizeof(raw));
  fixed=GWEN_Buffer_new(raw, 4, 0, 0);
  test_assert(fixed != NULL);
  GWEN_Buffer_SubMode(fixed, GWEN_BUFFER_MODE_DYNAMIC | GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  rv=GWEN_Text_UnescapeXmlToBuffer("Ref &#65;", fixed);
  test_assert(rv < 0);
  test_assert(memcmp(raw + 4, "ZZZZZZZZZZZZ", 12) == 0);
  GWEN_Buffer_free(fixed);

  /* Zero-capacity fixed buffer fails cleanly on non-empty input and succeeds on empty */
  memset(raw, 'Z', sizeof(raw));
  fixed=GWEN_Buffer_new(raw, 0, 0, 0);
  test_assert(fixed != NULL);
  GWEN_Buffer_SubMode(fixed, GWEN_BUFFER_MODE_DYNAMIC | GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  test_assert(GWEN_Text_UnescapeXmlToBuffer("", fixed) == 0);
  test_assert(GWEN_Text_UnescapeXmlToBuffer("x", fixed) < 0);
  test_assert(memcmp(raw, "ZZZZZZZZZZZZZZZZ", 16) == 0);
  GWEN_Buffer_free(fixed);

  /* UTF-8 continuation bytes, overlongs, surrogates, >U+10FFFF, and incomplete tails */
  test_assert(GWEN_Text_CountUtf8Chars("\xC3\xA4\xE2\x82\xAC\xF0\x9F\x98\x80", 0) == 3);
  test_assert(GWEN_Text_CountUtf8Chars("", 0) == 0);
  test_assert(GWEN_Text_CountUtf8Chars("\xC3x", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xE2\xC3\xA4", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xC3", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xC0\xAF", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xE0\x80\xAF", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xED\xA0\x80", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("\xF4\x90\x80\x80", 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars(NULL, 0) == -1);
  test_assert(GWEN_Text_CountUtf8Chars("abc", -1) == -1);

  return g_testSuccess;
}
