

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/list.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gwendate.h>
#include <errno.h>
#include "gwenhywfar.h"



int check1() {
  const char *testString="01234567890123456789";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  const char *p1, *p2;
  int i;
  int len;

  fprintf(stderr, "Check 1 ...");

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode((const unsigned char*)testString,
                        strlen(testString),
                        buf1, 0);
  if (rv) {
    fprintf(stderr, "FAILED: Could not encode.\n");
    return 2;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Decode((const unsigned char*)GWEN_Buffer_GetStart(buf1), 0,
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

  return 0;
}



int test_gui(int test_with_interaction) {
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
  return 0;
}



#ifndef MAX_PATH
# define MAX_PATH 200
#endif
int check_directory() {
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
int check_list() {
  const char *e1 = "one", *e2 = "two", *e3 = "three";
  GWEN_LIST *list;
  GWEN_LIST_ITERATOR *iter;

  list = GWEN_List_new();
  ASSERT(GWEN_List_GetSize(list) == 0);
  GWEN_List_PushBack(list, (void*) e2);
  ASSERT(GWEN_List_GetSize(list) == 1);
  GWEN_List_PushBack(list, (void*) e3);
  ASSERT(GWEN_List_GetSize(list) == 2);
  GWEN_List_PushFront(list, (void*) e1);
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

  GWEN_List_PushBack(list, (void*) e2);
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

int check_constlist() {
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

void *printfunc(const char *s, void *u) {
  const char *pathname = u;
  printf("Path %s contains: %s\n", pathname, s);
  return 0;
}
int print_paths() {
  const char *paths[] = { GWEN_PM_SYSCONFDIR
                          , GWEN_PM_LOCALEDIR
                          , GWEN_PM_PLUGINDIR
                          , GWEN_PM_DATADIR
                          , 0
                        };
  const char **p = paths;
  for ( ; *p != 0; ++p) {
    const char *pathname = *p;
    GWEN_STRINGLIST *sl =
      GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, pathname);
    printf("Path %s has %d elements.\n", pathname, GWEN_StringList_Count(sl));
    GWEN_StringList_ForEach(sl, printfunc, (void*)pathname);
  }
  return 0;
}



int check2() {
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

  return 0;
}


int test_date() {
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



int main(int argc, char **argv) {
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
       || print_paths()
       ;
  }
  else if (strcasecmp(cmd, "gui")==0) {
    rv=test_gui(1);
  }
  else if (strcasecmp(cmd, "date")==0) {
    rv=test_date();
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", cmd);
    return 1;
  }
  return rv;
}


