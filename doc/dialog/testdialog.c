/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "testdialog_p.h"


#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dialog.h>
#include <gwenhywfar/debug.h>


#include <unistd.h>



GWEN_DIALOG *TestDialog_new()
{
  int rv;
  GWEN_DIALOG *dlg;

  /* create dialog */
  dlg=GWEN_Dialog_new("testdialog");

  /* set signal handler */
  GWEN_Dialog_SetSignalHandler(dlg, TestDialog_SignalHandler);

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, "testdialog.dlg");
  if (rv<0) {
    fprintf(stderr, "Error reading dialog file (%d)\n", rv);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  return dlg;
}



static int GWENHYWFAR_CB TestDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                  GWEN_DIALOG_EVENTTYPE t,
                                                  const char *sender)
{
  fprintf(stderr,
          "Received event %d from widget [%s]\n", t, sender);

  switch (t) {
  case GWEN_DialogEvent_TypeInit:
    /* This is the init event. It is issued immediately before showing the
     * dialog on screen. Only after this signal has been received the properties
     * of a dialog are accessible.
     */
    fprintf(stderr, "Init\n");
    /* add some entries to the ComboBox "combo1" */
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
    /* select first entry (entry 0) of the ComboBo "combo1" */
    GWEN_Dialog_SetIntProperty(dlg, "combo1",
                               GWEN_DialogProperty_Value,
                               0,
                               0,
                               0);

    /* same for ComboBox "combo2" */
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

    /* set the title of the ListBox "listbox1". Please note that multiple
     * columns can be specified here (separated by TAB, i.e. '\t')
     */
    GWEN_Dialog_SetCharProperty(dlg, "listbox1",
                                GWEN_DialogProperty_Title,
                                0,
                                "Column1\tColumn2",
                                0);
    /* add some entries to the ListBox "listbox1". As with the title
     * multiple columns can be specified (separated by TAB).
     */
    GWEN_Dialog_SetCharProperty(dlg, "listbox1",
                                GWEN_DialogProperty_AddValue,
                                0,
                                "Zeile 1 Spalte 1\tZeile 1 Spalte 2",
                                0);

    GWEN_Dialog_SetCharProperty(dlg, "listbox1",
                                GWEN_DialogProperty_AddValue,
                                0,
                                "Zeile 2 Spalte 1\tZeile 2 Spalte 2",
                                0);

    /* for every column of a ListBox the width can be specified. This is done
     * by modification of the property "ColumnWidth". In this example we set
     * the width of column 0 to 100 pixel and that of column 1 to 200 pixel.
     */
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

    /* Set the width and height of the dialog itself to 640x480 pixel.
     * As you can see specifying the dialog itself is done by providing
     * an empty widget name.
     */
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
    /* this is the fini signal. It is emitted immediately after hiding
     * the dialog from screen. After this signal handler returns the
     * properties of this dialog are no longer accessible.
     */
    fprintf(stderr, "Fini\n");
    break;

  case GWEN_DialogEvent_TypeValueChanged:
    /* this signal is emitted by some of the widgets when its value
     * has changed. For LineEdit widgets this event occurrs when something
     * is entered into the text field, for a ListBox this is emitted when
     * another entry from the ListBox is selected.
     */
    fprintf(stderr, "ValueChanged\n");
    if (strcasecmp(sender, "listbox1")==0) {
      fprintf(stderr, "Selected list entry %d\n",
              GWEN_Dialog_GetIntProperty(dlg, "listbox1", GWEN_DialogProperty_Value, 0, -1));
    }
    break;

  /* this signal is emitted by some widgets upon activation, e.g. a Button
   * widget sends this signal when it is clicked.
   */
  case GWEN_DialogEvent_TypeActivated:
    fprintf(stderr, "Activated\n");
    if (strcasecmp(sender, "okButton")==0)
      return GWEN_DialogEvent_ResultAccept;
    else if (strcasecmp(sender, "abortButton")==0)
      return GWEN_DialogEvent_ResultReject;
    break;

  case GWEN_DialogEvent_TypeClose:
    /* this signal is emitted by the dialog when closing of the dialog is
     * requested (e.g. by clicking the little X in the title bar).
     * If the dialog doesn't want to be closed you can return
     * GWEN_DialogEvent_ResultReject here.
     */
    fprintf(stderr, "Close\n");
    return GWEN_DialogEvent_ResultNotHandled;

  default:
    fprintf(stderr, "Unhandled signal, ignored\n");
    return GWEN_DialogEvent_ResultNotHandled;
  }
  return GWEN_DialogEvent_ResultHandled;
}





