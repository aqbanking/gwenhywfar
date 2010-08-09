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
    GWEN_Dialog_SetCharProperty(dlg, "button1",
				GWEN_DialogProperty_Title,
				0,
				"Click this button to close",
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
    if (strcasecmp(sender, "button1")==0)
      return GWEN_DialogEvent_ResultAccept;
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



GWEN_DIALOG *Dlg_Test2_new() {
  GWEN_DIALOG *dlg;
  int rv;

  dlg=GWEN_Dialog_new("dlg_test2");
  GWEN_Dialog_SetSignalHandler(dlg, _gwenGuiSignalHandler);
  GWEN_Dialog_AddMediaPath(dlg, MEDIAPATH);

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, MEDIAPATH "/dlg_test2.dlg");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d).", rv);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* done */
  return dlg;
}





