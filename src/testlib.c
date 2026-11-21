

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/list.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gwendate.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/syncio_http.h>
#include <gwenhywfar/syncio_memory.h>
#include <gwenhywfar/syncio_buffered.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/json.h>
#include <gwenhywfar/json_read.h>
#include <gwenhywfar/json_dump.h>
#include <errno.h>
#include "gwenhywfar.h"



int check1()
{
  const char *testString="01234567890123456789";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  const char *p1, *p2;
  int i;
  int len;

  fprintf(stderr, "Check 1 ...");

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode((const unsigned char *)testString,
                        strlen(testString),
                        buf1, 0);
  if (rv) {
    fprintf(stderr, "FAILED: Could not encode.\n");
    return 2;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Decode((const unsigned char *)GWEN_Buffer_GetStart(buf1), 0,
                        buf2);
  if (rv) {
    fprintf(stderr, "FAILED: Could not decode.\n");
    return 2;
  }

  p1=testString;
  len=strlen(testString);
  p2=GWEN_Buffer_GetStart(buf2);
  if (GWEN_Buffer_GetUsedBytes(buf2)!=len) {
    fprintf(stderr, "Data differs in size\n");
    return 3;
  }
  rv=0;
  for (i=0; i<len; i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n%s\n", testString);
      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }

  fprintf(stderr, "PASSED.\n");

  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);

  return 0;
}



int test_gui(GWEN_UNUSED int test_with_interaction)
{
#if 0
  char buffer[50];
  int rv;
  GWEN_GUI *gui = GWEN_Gui_CGui_new();

  /* Set the static GUI object */
  assert(gui);
  GWEN_Gui_SetGui(gui);
  GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_NONINTERACTIVE);

  rv = GWEN_Gui_ShowBox(0,
                        "This is a ShowBox test title",
                        "This is a ShowBox test.",
                        0);
  printf("GWEN_Gui_ShowBox: rv=%d\n", rv);
  GWEN_Gui_HideBox(rv);
  printf("GWEN_Gui_HideBox called.\n\n");

  if (test_with_interaction) {
    rv = GWEN_Gui_InputBox(0,
                           "This is a InputBox test title",
                           "Just enter something.",
                           buffer,
                           1, 40,
                           0);
    printf("GWEN_Gui_InputBox: rv=%d, result=\"%s\"\n\n",
           rv, buffer);

    rv = GWEN_Gui_MessageBox(0,
                             "Third test title, this time MessageBox",
                             "Just press the first or second button.",
                             "First button.", "Second button", NULL,
                             0);
    printf("GWEN_Gui_MessageBox: rv=%d; button=%s\n", rv,
           (rv == 1 ? "first" : (rv == 2 ? "second" : "unknown")));
  }

  GWEN_Gui_free(gui);
#endif
  return 0;
}



#ifndef MAX_PATH
# define MAX_PATH 200
#endif
int check_directory()
{
  char tmpdir[MAX_PATH];
  GWEN_DIRECTORY *dir;
  int rv;

  GWEN_Directory_GetTmpDirectory(tmpdir, MAX_PATH);
  printf("GWEN_Directory_GetTmpDirectory returns \"%s\" as tmp directory\n",
         tmpdir);

  dir = GWEN_Directory_new();
  rv = GWEN_Directory_Open(dir, tmpdir);
  if (rv) {
    /* error */
    printf("Error on GWEN_Directory_Open(\"%s\"): errno=%d: %s\n",
           tmpdir, errno, strerror(errno));
  }
  else {
    rv = GWEN_Directory_Close(dir);
  }
  GWEN_Directory_free(dir);
  return rv;
}

#define ASSERT(expr) if (!(expr)) \
 { printf("FAILED assertion in " __FILE__ ": %d: " #expr "\n", \
    __LINE__); return -1; }
int check_list()
{
  const char *e1 = "one", *e2 = "two", *e3 = "three";
  GWEN_LIST *list;
  GWEN_LIST_ITERATOR *iter;

  list = GWEN_List_new();
  ASSERT(GWEN_List_GetSize(list) == 0);
  GWEN_List_PushBack(list, (void *) e2);
  ASSERT(GWEN_List_GetSize(list) == 1);
  GWEN_List_PushBack(list, (void *) e3);
  ASSERT(GWEN_List_GetSize(list) == 2);
  GWEN_List_PushFront(list, (void *) e1);
  ASSERT(GWEN_List_GetSize(list) == 3);
  ASSERT(GWEN_List_GetFront(list) == e1);
  ASSERT(GWEN_List_GetBack(list) == e3);

  GWEN_List_Remove(list, e2);
  ASSERT(GWEN_List_GetSize(list) == 2);
  ASSERT(GWEN_List_GetFront(list) == e1);
  ASSERT(GWEN_List_GetBack(list) == e3);

  GWEN_List_PopBack(list);
  ASSERT(GWEN_List_GetSize(list) == 1);
  ASSERT(GWEN_List_GetFront(list) == e1);
  ASSERT(GWEN_List_GetBack(list) == e1);

  GWEN_List_PushBack(list, (void *) e2);
  ASSERT(GWEN_List_GetSize(list) == 2);
  ASSERT(GWEN_List_GetFront(list) == e1);
  ASSERT(GWEN_List_GetBack(list) == e2);

  iter = GWEN_List_First(list);
  ASSERT(GWEN_ListIterator_Data(iter) == e1);
  ASSERT(GWEN_ListIterator_Next(iter) == e2);
  ASSERT(GWEN_ListIterator_Data(iter) == e2);

  ASSERT(GWEN_ListIterator_Previous(iter) == e1);
  GWEN_List_Erase(list, iter);
  ASSERT(GWEN_List_GetSize(list) == 1);
  ASSERT(GWEN_List_GetFront(list) == e2);
  ASSERT(GWEN_List_GetBack(list) == e2);

  GWEN_List_Clear(list);
  ASSERT(GWEN_List_GetSize(list) == 0);

  GWEN_List_free(list);
  GWEN_ListIterator_free(iter);
  printf("check_list: All tests passed.\n");
  return 0;
}

int check_constlist()
{
  const char *e1 = "one", *e2 = "two", *e3 = "three";
  GWEN_CONSTLIST *list;
  GWEN_CONSTLIST_ITERATOR *iter;

  list = GWEN_ConstList_new();
  ASSERT(GWEN_ConstList_GetSize(list) == 0);
  GWEN_ConstList_PushBack(list, e2);
  ASSERT(GWEN_ConstList_GetSize(list) == 1);
  GWEN_ConstList_PushBack(list, e3);
  ASSERT(GWEN_ConstList_GetSize(list) == 2);
  GWEN_ConstList_PushFront(list, e1);
  ASSERT(GWEN_ConstList_GetSize(list) == 3);
  ASSERT(GWEN_ConstList_GetFront(list) == e1);
  ASSERT(GWEN_ConstList_GetBack(list) == e3);

  GWEN_ConstList_PopBack(list);
  ASSERT(GWEN_ConstList_GetSize(list) == 2);
  ASSERT(GWEN_ConstList_GetFront(list) == e1);
  ASSERT(GWEN_ConstList_GetBack(list) == e2);

  GWEN_ConstList_PushBack(list, e3);
  ASSERT(GWEN_ConstList_GetSize(list) == 3);
  ASSERT(GWEN_ConstList_GetFront(list) == e1);
  ASSERT(GWEN_ConstList_GetBack(list) == e3);

  iter = GWEN_ConstList_First(list);
  ASSERT(GWEN_ConstListIterator_Data(iter) == e1);
  ASSERT(GWEN_ConstListIterator_Next(iter) == e2);
  ASSERT(GWEN_ConstListIterator_Data(iter) == e2);

  ASSERT(GWEN_ConstListIterator_Previous(iter) == e1);

  GWEN_ConstList_Clear(list);
  ASSERT(GWEN_ConstList_GetSize(list) == 0);

  GWEN_ConstList_free(list);
  GWEN_ConstListIterator_free(iter);
  printf("check_constlist: All tests passed.\n");
  return 0;
}

void *printfunc(const char *s, void *u)
{
  const char *pathname = u;
  printf("Path %s contains: %s\n", pathname, s);
  return 0;
}
int print_paths()
{
  const char *paths[] = { GWEN_PM_SYSCONFDIR
                          , GWEN_PM_LOCALEDIR
                          , GWEN_PM_PLUGINDIR
                          , GWEN_PM_DATADIR
                          , 0
                        };
  const char **p = paths;
  for (; *p != 0; ++p) {
    const char *pathname = *p;
    GWEN_STRINGLIST *sl =
      GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, pathname);
    printf("Path %s has %d elements.\n", pathname, GWEN_StringList_Count(sl));
    GWEN_StringList_ForEach(sl, printfunc, (void *)pathname);
    GWEN_StringList_free(sl);
  }
  return 0;
}



int check2()
{
  const char *testString="01234567890123456789";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  const char *p1, *p2;
  int i;
  int len;

  fprintf(stderr, "Check 2 ...");

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf1, testString);
  rv=GWEN_Padd_PaddWithIso9796_2(buf1, 256);
  if (rv) {
    fprintf(stderr, "FAILED: Could not padd.\n");
    return 2;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendBuffer(buf2, buf1);
  rv=GWEN_Padd_UnpaddWithIso9796_2(buf2);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unpadd.\n");
    return 2;
  }

  p1=testString;
  len=strlen(testString);
  p2=GWEN_Buffer_GetStart(buf2);
  if (GWEN_Buffer_GetUsedBytes(buf2)!=len) {
    fprintf(stderr, "Data differs in size\n");
    return 3;
  }
  rv=0;
  for (i=0; i<len; i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n%s\n", testString);
      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }

  fprintf(stderr, "PASSED.\n");

  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);

  return 0;
}


int test_date()
{
  GWEN_DATE *dt1;
  GWEN_DATE *dt2;
  time_t tt;

  dt1=GWEN_Date_CurrentDate();
  assert(dt1);
  tt=GWEN_Date_toLocalTime(dt1);

  dt2=GWEN_Date_fromLocalTime(tt);
  if (GWEN_Date_Compare(dt1, dt2)!=0) {
    fprintf(stderr, "Error: Date doesn't match: dt1: %s dt2: %s\n",
            GWEN_Date_GetString(dt1), GWEN_Date_GetString(dt2));
    return 3;
  }
  else {
    fprintf(stderr, "Date is okay (%s)\n", GWEN_Date_GetString(dt2));
  }

  return 0;
}



int test_json(const char *fname, const char *sSearch)
{
  int rv;
  GWEN_BUFFER *buf;
  GWEN_JSON_ELEM *je;
  GWEN_JSON_ELEM *jeSearch;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Helper_ReadFile(fname, buf);
  if (rv<0) {
    fprintf(stderr, "Error reading JSON file (%d).\n", rv);
    return 3;
  }

  je=GWEN_JsonElement_fromString(GWEN_Buffer_GetStart(buf));
  if (je==NULL) {
    fprintf(stderr, "Error parsing JSON file.\n");
    return 3;
  }
  fprintf(stdout, "File successfully read.\n");

  GWEN_Buffer_Reset(buf);
  GWEN_JsonElement_DumpToBuffer(je, 2, buf);
  fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(buf));

  jeSearch=GWEN_JsonElement_GetElementByPath(je, sSearch, 0);
  if (jeSearch) {
    fprintf(stdout, "Found element %s:\n", sSearch);
    GWEN_Buffer_Reset(buf);
    GWEN_JsonElement_DumpToBuffer(jeSearch, 2, buf);
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(buf));
  }

  GWEN_Buffer_free(buf);
  return 0;
}



static int check_parser_bounds(void)
{
  GWEN_XMLNODE *defs;
  GWEN_MSGENGINE *e;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *gr;
  GWEN_MSGENGINE_TRUSTEDDATA *td;
  GWEN_MSGENGINE_TRUSTEDDATA *tdNext;
  GWEN_BUFFER *b64;
  GWEN_BUFFER *fixed;
  char big64a[65];
  char small4a[5];
  char small4b[5];
  char fixedRaw[2];
  unsigned char *httpResp;
  uint8_t readTmp[64];
  GWEN_SYNCIO *memIo;
  GWEN_SYNCIO *bufIo;
  GWEN_SYNCIO *httpIo;
  const char *xmlDefs=
    "<GROUPs>"
    "  <GROUPdef id=\"SegHead\" delimiter=\":\" hide=\"1\">"
    "    <ELEM name=\"code\" type=\"AN\" maxsize=\"6\" />"
    "    <ELEM name=\"seq\" type=\"num\" maxsize=\"15\" />"
    "    <ELEM name=\"version\" type=\"num\" maxsize=\"3\" />"
    "  </GROUPdef>"
    "</GROUPs>";
  int rv;

  defs=GWEN_XMLNode_fromString(xmlDefs, (int)strlen(xmlDefs), GWEN_XML_FLAGS_DEFAULT);
  ASSERT(defs!=NULL);
  e=GWEN_MsgEngine_new();
  ASSERT(e!=NULL);
  GWEN_MsgEngine_SetDefinitions(e, defs, 1);

  mbuf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Buffer_AppendString(mbuf, "HXXXX:2:1'");
  GWEN_Buffer_Rewind(mbuf);
  gr=GWEN_DB_Group_new("msg");
  rv=GWEN_MsgEngine_ReadMessage(e, "SEG", mbuf, gr, GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO);
  ASSERT(rv==1);
  ASSERT(GWEN_MsgEngine_TakeTrustInfo(e)==NULL);
  GWEN_DB_Group_free(gr);
  GWEN_Buffer_free(mbuf);

  mbuf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Buffer_AppendString(mbuf, "HXXXX:2:1+'");
  GWEN_Buffer_Rewind(mbuf);
  gr=GWEN_DB_Group_new("msg");
  rv=GWEN_MsgEngine_ReadMessage(e, "SEG", mbuf, gr, GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO);
  ASSERT(rv==1);
  ASSERT(GWEN_MsgEngine_TakeTrustInfo(e)==NULL);
  GWEN_DB_Group_free(gr);
  GWEN_Buffer_free(mbuf);
  GWEN_MsgEngine_free(e);

  e=GWEN_MsgEngine_new();
  ASSERT(e!=NULL);
  memset(big64a, 'A', 64);
  big64a[64]=0;
  memcpy(small4a, "1234", 5);
  memcpy(small4b, "1234", 5);
  rv=GWEN_MsgEngine_AddTrustInfo(e, big64a, 64, "head", GWEN_MsgEngineTrustLevelHigh, 0);
  ASSERT(rv==0);
  rv=GWEN_MsgEngine_AddTrustInfo(e, small4a, 4, "pin1", GWEN_MsgEngineTrustLevelHigh, 70);
  ASSERT(rv==0);
  rv=GWEN_MsgEngine_AddTrustInfo(e, small4b, 4, "pin2", GWEN_MsgEngineTrustLevelHigh, 80);
  ASSERT(rv==0);
  td=GWEN_MsgEngine_TakeTrustInfo(e);
  ASSERT(td!=NULL);
  rv=GWEN_MsgEngine_TrustedData_CreateReplacements(td);
  ASSERT(rv==0);
  tdNext=GWEN_MsgEngine_TrustedData_GetNext(td);
  ASSERT(tdNext!=NULL);
  ASSERT(GWEN_MsgEngine_TrustedData_GetNext(tdNext)!=NULL);
  ASSERT(strcmp(GWEN_MsgEngine_TrustedData_GetReplacement(tdNext),
                GWEN_MsgEngine_TrustedData_GetReplacement(GWEN_MsgEngine_TrustedData_GetNext(tdNext)))==0);
  while (td) {
    GWEN_MSGENGINE_TRUSTEDDATA *next;

    next=GWEN_MsgEngine_TrustedData_GetNext(td);
    GWEN_MsgEngine_TrustedData_free(td);
    td=next;
  }
  GWEN_MsgEngine_free(e);

  b64=GWEN_Buffer_new(0, 32, 0, 1);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"TQ==", 0, b64)==0);
  ASSERT(GWEN_Buffer_GetUsedBytes(b64)==1 && strcmp(GWEN_Buffer_GetStart(b64), "M")==0);
  GWEN_Buffer_Reset(b64);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"TQ==", 4, b64)==0);
  ASSERT(GWEN_Buffer_GetUsedBytes(b64)==1 && strcmp(GWEN_Buffer_GetStart(b64), "M")==0);
  GWEN_Buffer_Reset(b64);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"TWE=", 4, b64)==0);
  ASSERT(GWEN_Buffer_GetUsedBytes(b64)==2 && strcmp(GWEN_Buffer_GetStart(b64), "Ma")==0);
  GWEN_Buffer_Reset(b64);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"TWFu", 4, b64)==0);
  ASSERT(GWEN_Buffer_GetUsedBytes(b64)==3 && strcmp(GWEN_Buffer_GetStart(b64), "Man")==0);
  GWEN_Buffer_Reset(b64);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"====", 0, b64)<0);
  GWEN_Buffer_Reset(b64);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"A===", 0, b64)<0);
  GWEN_Buffer_Reset(b64);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"AB=C", 0, b64)<0);
  GWEN_Buffer_free(b64);

  fixed=GWEN_Buffer_new(fixedRaw, (uint32_t)sizeof(fixedRaw), 0, 0);
  ASSERT(fixed!=NULL);
  GWEN_Buffer_SubMode(fixed, GWEN_BUFFER_MODE_DYNAMIC | GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  ASSERT(GWEN_Base64_Decode((const unsigned char *)"TWFu", 0, fixed)<0);
  GWEN_Buffer_free(fixed);

  httpResp=(unsigned char *)malloc(270000);
  ASSERT(httpResp!=NULL);
  memset(httpResp, 'X', 70000);
  memcpy(httpResp, "HTTP/1.1 200 ", 13);
  httpResp[70000]='\n';
  memIo=GWEN_SyncIo_Memory_fromBuffer(httpResp, 70001);
  bufIo=GWEN_SyncIo_Buffered_new(memIo);
  httpIo=GWEN_SyncIo_Http_new(bufIo);
  ASSERT(GWEN_SyncIo_Connect(httpIo)==0);
  rv=GWEN_SyncIo_Read(httpIo, readTmp, (uint32_t)sizeof(readTmp));
  ASSERT(rv==GWEN_ERROR_BUFFER_OVERFLOW);
  GWEN_SyncIo_free(httpIo);

  memcpy(httpResp, "HTTP/1.1 200 OK\r\nX-Big: ", 24);
  memset(httpResp+24, 'Y', 265000);
  memcpy(httpResp+24+265000, "\r\n\r\n", 4);
  memIo=GWEN_SyncIo_Memory_fromBuffer(httpResp, 24+265000+4);
  bufIo=GWEN_SyncIo_Buffered_new(memIo);
  httpIo=GWEN_SyncIo_Http_new(bufIo);
  ASSERT(GWEN_SyncIo_Connect(httpIo)==0);
  rv=GWEN_SyncIo_Read(httpIo, readTmp, (uint32_t)sizeof(readTmp));
  ASSERT(rv==GWEN_ERROR_BUFFER_OVERFLOW);
  GWEN_SyncIo_free(httpIo);
  free(httpResp);

  return 0;
}

int main(int argc, char **argv)
{
  int rv;
  const char *cmd;

  if (argc>1)
    cmd=argv[1];
  else
    cmd="check";

  if (strcasecmp(cmd, "check")==0) {
    rv=check1() ||
       check2() ||
       test_gui(0) ||
       check_directory() ||
       check_list() ||
       check_constlist()
       || check_parser_bounds()
       || print_paths()
       ;
  }
  else if (strcasecmp(cmd, "gui")==0) {
    rv=test_gui(1);
  }
  else if (strcasecmp(cmd, "date")==0) {
    rv=test_date();
  }
  else if (strcasecmp(cmd, "json")==0) {
    if (argc<4) {
      fprintf(stderr, "args: FILENAME SEARCHPATH.\n");
      return 1;
    }
    rv=test_json(argv[2], argv[3]);
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", cmd);
    return 1;
  }
  return rv;
}


