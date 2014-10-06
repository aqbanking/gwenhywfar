/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dialog.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/pathmanager.h>


#ifdef OS_WIN32
# include <windows.h>
# define sleep(x) Sleep(x)
#endif

#include <unistd.h>





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

    GWEN_Dialog_SetCharProperty(dlg, "labelWithHtmlImage",
                                GWEN_DialogProperty_Title,
                                0,
                                "<html>This label contains text which should also contain an image:"
                                "<img src=\"chipcard.png\">"
                                "<p>And here the text should continue, followed by another image:"
                                "<img src=\"disk.png\"></p>"
                                "And again, this should be on the following line.</html>"
                                "This label would contain an image if it were able to use "
                                "HTML.",
                                0);

    GWEN_Dialog_SetCharProperty(dlg, "textBrowser1",
                                GWEN_DialogProperty_Value,
                                0,
                                "<html>"
                                "<p>This is <b>bold</b> text, while this one is <i>italic</i>.</p>"
                                "This is a list:"
                                "<ul>"
                                "<li>first item</li>"
                                "<li>second item</li>"
                                "<li>third item</li>"
                                "<li>fourth item</li>"
                                "</ul>"
                                "<p>This paragraph should follow the list.</p>"
                                "</html>"
                                "This is BOLD text, while this one is would be i t a l i c.\n"
                                "This is a list:\n"
                                "- first item\n"
                                "- second item\n"
                                "- third item\n"
                                "- fourth item\n"
                                "This paragraph should follow the list.",
                                0);

    GWEN_Dialog_SetIntProperty(dlg, "progressBar1",
                               GWEN_DialogProperty_MinValue,
                               0,
                               0,
                               0);
    GWEN_Dialog_SetIntProperty(dlg, "progressBar1",
                               GWEN_DialogProperty_MaxValue,
                               0,
                               10,
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
    else if (strcasecmp(sender, "editPass1")==0) {
      const char *s;

      s=GWEN_Dialog_GetCharProperty(dlg, "editPass1", GWEN_DialogProperty_Value, 0, NULL);
      if (!(s && *s))
        s="<empty>";
      GWEN_Dialog_SetCharProperty(dlg, "editPass2", GWEN_DialogProperty_Value, 0, s, 0);
    }
    break;

  case GWEN_DialogEvent_TypeActivated:
    fprintf(stderr, "Activated\n");
    if (strcasecmp(sender, "listbox1")==0) {
      int idx;

      idx=GWEN_Dialog_GetIntProperty(dlg, "listbox1", GWEN_DialogProperty_Value, 0, -1);
      fprintf(stderr, "Selected list entry %d\n", idx);
      if (idx>=0) {
        const char *s;

        s=GWEN_Dialog_GetCharProperty(dlg, "listbox1", GWEN_DialogProperty_Value, idx, NULL);
        if (s && *s) {
          fprintf(stderr, "Text of selected list entry %d: [%s]\n", idx, s);
        }
      }
    }
    else if (strcasecmp(sender, "combo1")==0) {
      int idx;

      idx=GWEN_Dialog_GetIntProperty(dlg, "combo1", GWEN_DialogProperty_Value, 0, -1);
      fprintf(stderr, "Selected list entry %d\n", idx);
      if (idx>=0) {
        const char *s;

        s=GWEN_Dialog_GetCharProperty(dlg, "combo1", GWEN_DialogProperty_Value, idx, NULL);
        if (s && *s) {
          fprintf(stderr, "Text of selected list entry %d: [%s]\n", idx, s);
        }
      }

    }
    else if (strcasecmp(sender, "okButton")==0)
      return GWEN_DialogEvent_ResultAccept;
    else if (strcasecmp(sender, "abortButton")==0)
      return GWEN_DialogEvent_ResultReject;
    else if (strcasecmp(sender, "progressMinus")==0) {
      int v;

      v=GWEN_Dialog_GetIntProperty(dlg, "progressBar1", GWEN_DialogProperty_Value, 0, -1);
      if (v>0)
        GWEN_Dialog_SetIntProperty(dlg, "progressBar1", GWEN_DialogProperty_Value, 0, v-1, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
    else if (strcasecmp(sender, "progressPlus")==0) {
      int v;

      v=GWEN_Dialog_GetIntProperty(dlg, "progressBar1", GWEN_DialogProperty_Value, 0, -1);
      if (v<10)
        GWEN_Dialog_SetIntProperty(dlg, "progressBar1", GWEN_DialogProperty_Value, 0, v+1, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
    else if (strcasecmp(sender, "prevPageButton")==0) {
      int v;

      v=GWEN_Dialog_GetIntProperty(dlg, "stack1", GWEN_DialogProperty_Value, 0, -1);
      if (v>0)
        GWEN_Dialog_SetIntProperty(dlg, "stack1", GWEN_DialogProperty_Value, 0, v-1, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
    else if (strcasecmp(sender, "nextPageButton")==0) {
      int v;

      v=GWEN_Dialog_GetIntProperty(dlg, "stack1", GWEN_DialogProperty_Value, 0, -1);
      if (v<3)
        GWEN_Dialog_SetIntProperty(dlg, "stack1", GWEN_DialogProperty_Value, 0, v+1, 0);
      return GWEN_DialogEvent_ResultHandled;
    }
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



GWEN_DIALOG *Dlg_Test1_new() {
  GWEN_DIALOG *dlg;
  int rv;
  const char *s;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=getenv("DIALOG_DIR");
  if (s && *s)
    GWEN_Buffer_AppendString(tbuf, s);
  else
    GWEN_Buffer_AppendString(tbuf, MEDIAPATH);
  dlg=GWEN_Dialog_new("dlg_test");
  GWEN_Dialog_SetSignalHandler(dlg, _gwenGuiSignalHandler);
  GWEN_Dialog_AddMediaPath(dlg, GWEN_Buffer_GetStart(tbuf));

  /* read dialog from dialog description file */
  GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S "dlg_test.dlg");
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d).", rv);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* done */
  return dlg;
}





