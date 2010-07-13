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

  static GWENHYWFAR_CB int SetIntProperty(GWEN_DIALOG *dlg,
					  GWEN_WIDGET *w,
					  GWEN_DIALOG_PROPERTY prop,
					  int index,
					  int value,
					  int doSignal);

  static GWENHYWFAR_CB int GetIntProperty(GWEN_DIALOG *dlg,
					  GWEN_WIDGET *w,
					  GWEN_DIALOG_PROPERTY prop,
					  int index,
					  int defaultValue);

  static GWENHYWFAR_CB int SetCharProperty(GWEN_DIALOG *dlg,
					   GWEN_WIDGET *w,
					   GWEN_DIALOG_PROPERTY prop,
					   int index,
					   const char *value,
					   int doSignal);

  static GWENHYWFAR_CB const char *GetCharProperty(GWEN_DIALOG *dlg,
						   GWEN_WIDGET *w,
						   GWEN_DIALOG_PROPERTY prop,
						   int index,
						   const char *defaultValue);

  static GWENHYWFAR_CB void freeData(void *bp, void *p);

};




#endif /* CPPDIALOG_P_HPP */


