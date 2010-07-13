/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef CPPWIDGET_P_HPP
#define CPPWIDGET_P_HPP


#include "cppwidget.hpp"


class CppWidgetLinker {
  friend class CppWidget;

  static GWENHYWFAR_CB int SetIntProperty(GWEN_WIDGET *w,
					  GWEN_DIALOG_PROPERTY prop,
					  int index,
					  int value,
					  int doSignal);

  static GWENHYWFAR_CB int GetIntProperty(GWEN_WIDGET *w,
					  GWEN_DIALOG_PROPERTY prop,
					  int index,
					  int defaultValue);

  static GWENHYWFAR_CB int SetCharProperty(GWEN_WIDGET *w,
					   GWEN_DIALOG_PROPERTY prop,
					   int index,
					   const char *value,
					   int doSignal);

  static GWENHYWFAR_CB const char *GetCharProperty(GWEN_WIDGET *w,
						   GWEN_DIALOG_PROPERTY prop,
						   int index,
						   const char *defaultValue);

  static GWENHYWFAR_CB int AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild);


  static GWENHYWFAR_CB void freeData(void *bp, void *p);

};




#endif /* CPPDIALOG_P_HPP */


