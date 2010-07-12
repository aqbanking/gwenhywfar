
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef OS_WIN32
# include <windows.h>
# define sleep(x) Sleep(x)
#endif


#undef BUILDING_FOX16_GUI


#include <gwen-gui-fox16/fox16_gui.hpp>
#include "fox16_htmlctx.hpp"
#include "fox16_htmllabel.hpp"
#include "fox16_htmltext.hpp"

#include <gwenhywfar/debug.h>



int test1(int argc, char **argv) {
  FXApp a("libtest","Martin Preuss");
  int rv;
  const char testString[]=
#if 0
    "This is <b>a</b> test. "
    "And this, is the second line: followed by something else."
    "<table>"
    "  <tr>"
    "    <th>header1</th>"
    "    <th>header2</th>"
    "  </tr>"
    "  <tr>"
    "    <td>data1</td>"
    "    <td>data2</td>"
    "  </tr>"
    "</table>"
    "And   this is   after the table.<br>"
    "and this <font color=\"red\">one</font> is red.";
#else
    "This is before right"
    "<right>RIGHT</right>"
    "and this after right.";
#endif

  a.init(argc,argv);
  a.create();

  FOX16_HtmlCtx ctx(0);
  ctx.setText(testString);
  rv=ctx.layout(300, 1024);
  fprintf(stderr, "Result of layout: %d\n", rv);
  ctx.dump();

  return 0;
}




int test2(int argc, char **argv) {
  FXApp application("libtest","Martin Preuss");
  FOX16_Gui *gui;
  int rv;
  uint32_t id1;
  uint32_t id2;
  uint64_t i1;
  uint64_t i2;

  application.init(argc,argv);

  application.create();

  gui=new FOX16_Gui(&application);
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

    snprintf(numbuf, sizeof(numbuf)-1, "Step %d", (int)i1);
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
	fprintf(stderr, "Aborted by user\n");
	break;
      }
    }
    GWEN_Gui_ProgressEnd(id2);

    rv=GWEN_Gui_ProgressAdvance(id1, i1);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user\n");
      break;
    }
  }

  GWEN_Gui_ProgressEnd(id1);

  return 0;
}



int test3(int argc, char **argv) {
  FXApp a("libtest","Martin Preuss");
  FOX16_HtmlLabel *label;
  FOX16_Gui *gui;
  FXDialogBox *dbox;
  FXVerticalFrame *vf;
  const char testString[]=
#if 1
    "<h1>Title</h1>"
    "<h2>Subtitle</h2>"
    "This is <b>a</b> test. "
    "And this, is the second line: followed by something else."
    "<right>This should be right aligned</right>"
    "<table>"
    "  <tr>"
    "    <th>header1</th>"
    "    <th>header2</th>"
    "  </tr>"
    "  <tr>"
    "    <td>data1</td>"
    "    <td>this is longer data (data2)</td>"
    "  </tr>"
    "</table>"
    "And   this is   after the table.<br>"
    "and this <font color=\"red\">one</font> is red.";
#else
    "This is before right"
    "<right>Right</right>"
    "and this after right.";
#endif
  a.init(argc,argv);
  a.create();

  gui=new FOX16_Gui(&a);
  GWEN_Gui_SetGui(gui->getCInterface());

  dbox=new FXDialogBox(&a, "Test", DECOR_ALL);
  vf=new FXVerticalFrame(dbox, LAYOUT_FILL_X | LAYOUT_FILL_Y,
                         0, 0, 0, 0, 1, 1, 1, 1);
  label=new FOX16_HtmlLabel(vf, FXString(testString), LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXSpring(vf, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  new FXLabel(vf, "Normal Testlabel");

  dbox->create();
  dbox->resize(300, 200);
  dbox->show();
  a.runModalFor(dbox);

  return 0;
}



int test4(int argc, char **argv) {
  FXApp application("libtest","Martin Preuss");
  FOX16_Gui *gui;
  int rv;
  uint32_t id1;
  uint32_t id2;
  uint64_t i1;
  uint64_t i2;

  application.init(argc,argv);

  application.create();

  gui=new FOX16_Gui(&application);
  GWEN_Gui_SetGui(gui->getCInterface());

  id1=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_SHOW_ABORT |
			     GWEN_GUI_PROGRESS_KEEP_OPEN,
			     "Progress-Title",
                             "<html>"
                             "<p><b>Test</b> f\xc3\xbcr Umlaute.</p>"
                             "</html>",
                             10,
                             0);
  for (i1=1; i1<=10; i1++) {
    char numbuf[128];

    snprintf(numbuf, sizeof(numbuf)-1, "Step %d", (int)i1);
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
	fprintf(stderr, "Aborted by user\n");
	break;
      }
    }
    GWEN_Gui_ProgressEnd(id2);

    rv=GWEN_Gui_ProgressAdvance(id1, i1);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user\n");
      break;
    }
  }

  GWEN_Gui_ProgressEnd(id1);

  return 0;
}



int test5(int argc, char **argv) {
  FXApp a("libtest","Martin Preuss");
  FOX16_HtmlText *label;
  FOX16_Gui *gui;
  FXDialogBox *dbox;
  FXVerticalFrame *vf;
  const char testString[]=
#if 1
    "<h1>Test5</h1>"
    "<h2>Subtitle</h2>"
    "This is <b>a</b> test. "
    "And this, is the second line: followed by something else."
    "<right>This should be right aligned</right>"
    "<table>"
    "  <tr>"
    "    <th>header1</th>"
    "    <th>header2</th>"
    "  </tr>"
    "  <tr>"
    "    <td>data1</td>"
    "    <td>this is longer data (data2)</td>"
    "  </tr>"
    "</table>"
    "And   this is   after the table.<br>"
    "and this <font color=\"red\">one</font> is red.";
#else
    "This is before right"
    "<right>Right</right>"
    "and this after right.";
#endif
  a.init(argc,argv);
  a.create();

  gui=new FOX16_Gui(&a);
  GWEN_Gui_SetGui(gui->getCInterface());

  dbox=new FXDialogBox(&a, "Test", DECOR_ALL);
  vf=new FXVerticalFrame(dbox, LAYOUT_FILL_X | LAYOUT_FILL_Y,
                         0, 0, 0, 0, 1, 1, 1, 1);
  label=new FOX16_HtmlText(vf, FXString(testString),
			   LAYOUT_FILL_X|LAYOUT_FILL_Y |
			   HSCROLLING_OFF | VSCROLLER_ALWAYS);
  new FXSpring(vf, LAYOUT_FILL_X | LAYOUT_FILL_Y);
  new FXLabel(vf, "Normal Testlabel");

  dbox->create();
  dbox->resize(300, 200);
  dbox->show();
  a.runModalFor(dbox);

  return 0;
}



int test6(int argc, char **argv) {
  FXApp application("libtest","Martin Preuss");
  FOX16_Gui *gui;
  int rv;
  uint32_t id1;
  uint32_t id2;
  uint64_t i1;
  uint64_t i2;

  application.init(argc,argv);

  application.create();

  gui=new FOX16_Gui(&application);
  GWEN_Gui_SetGui(gui->getCInterface());

  id1=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_SHOW_ABORT |
			     GWEN_GUI_PROGRESS_KEEP_OPEN,
			     "Progress-Title",
                             "<html>"
                             "<p><b>Test</b> f\xc3\xbcr Umlaute.</p>"
                             "</html>",
                             10,
                             0);
  for (i1=1; i1<=20; i1++) {
    char numbuf[128];

    snprintf(numbuf, sizeof(numbuf)-1, "Step %d", (int)i1);
    GWEN_Gui_ProgressLog(id1, GWEN_LoggerLevel_Notice, numbuf);
    id2=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_SHOW_LOG |
                               GWEN_GUI_PROGRESS_DELAY |
			       GWEN_GUI_PROGRESS_SHOW_ABORT,
			       "2nd progress",
			       "Starting 2nd progress...",
			       5,
			       id1);
    for (i2=1; i2<=5; i2++) {
      sleep(1);
      fprintf(stderr, "Advancing %d/%d\n", (int)i1, (int)i2);
      GWEN_Gui_ProgressLog(id2, GWEN_LoggerLevel_Notice, "Advancing...");
      rv=GWEN_Gui_ProgressAdvance(id2, i2);
      if (rv==GWEN_ERROR_USER_ABORTED) {
	fprintf(stderr, "Aborted by user\n");
	break;
      }
    }
    GWEN_Gui_ProgressEnd(id2);

    rv=GWEN_Gui_ProgressAdvance(id1, i1);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      fprintf(stderr, "Aborted by user\n");
      break;
    }
  }

  GWEN_Gui_ProgressEnd(id1);

  return 0;
}



int test7(int argc, char **argv) {
  FXApp application("libtest","Martin Preuss");
  FOX16_Gui *gui;
  char buffer[65];

  application.init(argc,argv);

  application.create();

  gui=new FOX16_Gui(&application);
  GWEN_Gui_SetGui(gui->getCInterface());

  GWEN_Gui_InputBox(GWEN_GUI_INPUT_FLAGS_SHOW,
		    "This is the Title",
		    "<html>This is the text.</html>This is ASCII",
		    buffer,
		    1,
		    sizeof(buffer)-1,
		    0);


  return 0;
}



static int GWENHYWFAR_CB _gwenGuiSignalHandler(GWEN_DIALOG *dlg,
					       GWEN_DIALOG_EVENTTYPE t,
					       const char *sender) {
  fprintf(stderr,
	  "Received event %d from widget [%s]\n", t, sender);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    fprintf(stderr, "Init\n");
    GWEN_Dialog_SetCharProperty(dlg, "combo1",
				GWEN_DialogProperty_AddValue,
				0,
				"Erster Text in Combo1",
				0);
    GWEN_Dialog_SetCharProperty(dlg, "combo1",
				GWEN_DialogProperty_AddValue,
				0,
				"Zweiter Text in Combo1",
				0);
    GWEN_Dialog_SetIntProperty(dlg, "combo1",
			       GWEN_DialogProperty_Value,
			       0,
			       0,
			       0);


    GWEN_Dialog_SetCharProperty(dlg, "combo2",
				GWEN_DialogProperty_AddValue,
				0,
				"Erster Text in Combo2",
				0);
    GWEN_Dialog_SetCharProperty(dlg, "combo2",
				GWEN_DialogProperty_AddValue,
				0,
				"Zweiter Text in Combo2",
				0);
    GWEN_Dialog_SetIntProperty(dlg, "combo2",
			       GWEN_DialogProperty_Value,
			       0,
			       0,
			       0);

    GWEN_Dialog_SetCharProperty(dlg, "listbox1",
				GWEN_DialogProperty_Title,
				0,
				"Column1\tColumn2\tColumn3",
				0);

    GWEN_Dialog_SetCharProperty(dlg, "listbox1",
				GWEN_DialogProperty_AddValue,
				0,
				"Zeile 1 Spalte 1\tZeile 1 Spalte 2\tZeile 1 Spalte 3",
				0);
    GWEN_Dialog_SetCharProperty(dlg, "listbox1",
				GWEN_DialogProperty_AddValue,
				0,
				"Zeile 2 Spalte 1\tZeile 2 Spalte 2\tZeile 2 Spalte 3",
				0);
    GWEN_Dialog_SetIntProperty(dlg, "listbox1",
			       GWEN_DialogProperty_ColumnWidth,
			       0,
                               100,
			       0);
    GWEN_Dialog_SetIntProperty(dlg, "listbox1",
			       GWEN_DialogProperty_ColumnWidth,
			       1,
			       200,
			       0);

    GWEN_Dialog_SetIntProperty(dlg, "",
			       GWEN_DialogProperty_Width,
			       0,
			       640,
			       0);
   GWEN_Dialog_SetIntProperty(dlg, "",
			       GWEN_DialogProperty_Height,
			       0,
			       480,
			       0);

    break;

  case GWEN_DialogEvent_TypeFini:
    fprintf(stderr, "Fini\n");
    break;
  case GWEN_DialogEvent_TypeValueChanged:
    fprintf(stderr, "ValueChanged\n");
    if (strcasecmp(sender, "listbox1")==0) {
      fprintf(stderr, "Selected list entry %d\n",
	      GWEN_Dialog_GetIntProperty(dlg, "listbox1", GWEN_DialogProperty_Value, 0, -1));
    }
    break;

  case GWEN_DialogEvent_TypeActivated:
    fprintf(stderr, "Activated\n");
    if (strcasecmp(sender, "listbox1")==0) {
      fprintf(stderr, "Selected list entry %d\n",
	      GWEN_Dialog_GetIntProperty(dlg, "listbox1", GWEN_DialogProperty_Value, 0, -1));
    }
    else if (strcasecmp(sender, "okButton")==0)
      return GWEN_DialogEvent_ResultAccept;
    else if (strcasecmp(sender, "abortButton")==0)
      return GWEN_DialogEvent_ResultReject;
    break;
  case GWEN_DialogEvent_TypeEnabled:
    fprintf(stderr, "Enabled\n");
    break;
  case GWEN_DialogEvent_TypeDisabled:
    fprintf(stderr, "Disabled\n");
    break;
  case GWEN_DialogEvent_TypeClose:
    fprintf(stderr, "Close\n");
    return GWEN_DialogEvent_ResultNotHandled;
  case GWEN_DialogEvent_TypeLast:
    fprintf(stderr, "Last, ignored\n");
    return GWEN_DialogEvent_ResultNotHandled;
  }
  return GWEN_DialogEvent_ResultHandled;
}


int test8(int argc, char **argv) {
  FXApp application("libtest","Martin Preuss");
  FOX16_Gui *gui;
  int rv;
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nDialog;
  GWEN_DIALOG *dlg;

  application.init(argc,argv);

  application.create();

  gui=new FOX16_Gui(&application);
  GWEN_Gui_SetGui(gui->getCInterface());


  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);
  if (GWEN_XML_ReadFile(n, "libtest.dlg",
			GWEN_XML_FLAGS_DEFAULT |
			GWEN_XML_FLAGS_HANDLE_HEADERS)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  nDialog=GWEN_XMLNode_FindFirstTag(n, "dialog", NULL, NULL);
  if (nDialog==NULL) {
    fprintf(stderr, "Dialog not found in XML file\n");
    return 2;
  }

  dlg=GWEN_Dialog_new("testdialog");
  rv=GWEN_Dialog_ReadXml(dlg, nDialog);
  if (rv) {
    fprintf(stderr, "Error reading widgets from XML node: %d\n", rv);
    return 2;
  }
  GWEN_XMLNode_free(n);

  GWEN_Dialog_AddMediaPath(dlg, ".");

  GWEN_Dialog_SetSignalHandler(dlg, _gwenGuiSignalHandler);

  rv=GWEN_Gui_ExecDialog(dlg, 0);
  fprintf(stderr, "Result: %d\n", rv);

  return 0;


  return 0;
}




int main(int argc, char **argv) {
  if (argc>1) {
    if (strcasecmp(argv[1], "1")==0)
      return test1(argc, argv);
    else if (strcasecmp(argv[1], "2")==0)
      return test2(argc, argv);
    else if (strcasecmp(argv[1], "3")==0)
      return test3(argc, argv);
    else if (strcasecmp(argv[1], "4")==0)
      return test4(argc, argv);
    else if (strcasecmp(argv[1], "5")==0)
      return test5(argc, argv);
    else if (strcasecmp(argv[1], "6")==0)
      return test6(argc, argv);
    else if (strcasecmp(argv[1], "7")==0)
      return test7(argc, argv);
    else if (strcasecmp(argv[1], "8")==0)
      return test8(argc, argv);
  }
  else
    return test7(argc, argv);
}



