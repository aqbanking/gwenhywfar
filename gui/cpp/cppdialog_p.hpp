/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef CPPDIALOG_P_HPP
#define CPPDIALOG_P_HPP


#include "cppdialog.hpp"


class CppDialogLinker {
  friend class CppDialog;

  static int GWENHYWFAR_CB SetIntProperty(GWEN_DIALOG *dlg,
                                          GWEN_WIDGET *w,
                                          GWEN_DIALOG_PROPERTY prop,
                                          int index,
                                          int value,
                                          int doSignal);

  static int GWENHYWFAR_CB GetIntProperty(GWEN_DIALOG *dlg,
                                          GWEN_WIDGET *w,
                                          GWEN_DIALOG_PROPERTY prop,
                                          int index,
                                          int defaultValue);

  static int GWENHYWFAR_CB SetCharProperty(GWEN_DIALOG *dlg,
      GWEN_WIDGET *w,
      GWEN_DIALOG_PROPERTY prop,
      int index,
      const char *value,
      int doSignal);

  static const char *GWENHYWFAR_CB GetCharProperty(GWEN_DIALOG *dlg,
      GWEN_WIDGET *w,
      GWEN_DIALOG_PROPERTY prop,
      int index,
      const char *defaultValue);

  static void GWENHYWFAR_CB freeData(void *bp, void *p);

};




#endif /* CPPDIALOG_P_HPP */


