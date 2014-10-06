
#undef BUILDING_QT4_GUI

#include "qt4_gui.hpp"
#include "../testdialogs/dlg_test.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dialog.h>
#include <gwenhywfar/debug.h>

#include <qapplication.h>

#include <unistd.h>



int test1(int argc, char **argv) {
  QApplication a(argc, argv);
  QT4_Gui *gui;
  int rv;
  GWEN_DIALOG *dlg;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(0, rv);
    return 2;
  }

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);

  /* create GUI */
  gui=new QT4_Gui();
  GWEN_Gui_SetGui(gui->getCInterface());

  dlg=Dlg_Test1_new();
  if (dlg==NULL) {
    fprintf(stderr, "Could not create dialog.\n");
    return 2;
  }

  rv=GWEN_Gui_ExecDialog(dlg, 0);
  fprintf(stderr, "Result: %d\n", rv);

  return 0;
}



int test2(int argc, char **argv) {
  QApplication a(argc, argv);
  QT4_Gui *gui;
  QString lf;
  int rv;
  uint32_t pid;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(0, rv);
    return 2;
  }

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);

  /* create GUI */
  gui=new QT4_Gui();
  GWEN_Gui_SetGui(gui->getCInterface());


#if 0
  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS | GWEN_GUI_PROGRESS_KEEP_OPEN,
                             "Progress-Title",
                             "This is an example progress with 2 steps"
                             "<html>This is an <strong>example</strong> progress with 2 steps</html>",
                             2,
                             0);
#else
  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_PROGRESS | GWEN_GUI_PROGRESS_KEEP_OPEN,
                             "Progress-Title",
                             "This is an <b>example</b> progress with 2 steps",
                             2,
                             0);
#endif

  GWEN_Gui_ProgressAdvance(pid, 1);
  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO,
                         "MessageBox-Title",
                         "This message box should appear in the context of the open progress dialog",
                         "Button1",
                         "Button2",
                         "Button3",
                         pid);
  GWEN_Gui_ProgressAdvance(pid, 2);
  GWEN_Gui_ProgressEnd(pid);

  return 0;
}



int test3(int argc, char **argv) {
  int rv;
  uint32_t id1;
  uint32_t id2;
  uint64_t i1;
  uint64_t i2;
  QApplication a(argc, argv);
  QT4_Gui *gui;

  gui=new QT4_Gui();
  GWEN_Gui_SetGui(gui->getCInterface());

  id1=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_SHOW_ABORT |
                             GWEN_GUI_PROGRESS_KEEP_OPEN,
                             "Progress-Title",
                             "<html>"
                             "<p><b>This</b> is an example <i>text</i>..</p>"
                             "<p>As you can see <font color=red>colors</font> can "
                             "be used.</p>"
                             "</html>",
                             10,
                             0);
  for (i1=1; i1<=10; i1++) {
    char numbuf[128];

    snprintf(numbuf, sizeof(numbuf)-1, "Step %d\n", (int)i1);
    GWEN_Gui_ProgressLog(id1, GWEN_LoggerLevel_Notice, numbuf);
    id2=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
                               GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_SHOW_ABORT,
                               "2nd progress",
                               "Starting 2nd progress...",
                               10,
                               id1);
    for (i2=1; i2<=10; i2++) {
      sleep(1);
      fprintf(stderr, "Advancing %d/%d\n", (int)i1, (int)i2);
      rv=GWEN_Gui_ProgressAdvance(id2, i2);
      if (rv==GWEN_ERROR_USER_ABORTED) {
        fprintf(stderr, "Aborted by user (2)\n");
        break;
      }
    }
    GWEN_Gui_ProgressEnd(id2);

    rv=GWEN_Gui_ProgressAdvance(id1, i1);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user (1)\n");
      break;
    }
  }

  GWEN_Gui_ProgressEnd(id1);

  return 0;
}



int main(int argc, char **argv) {
  return test1(argc, argv);
  //return test2(argc, argv);
  //return test3(argc, argv);
}

