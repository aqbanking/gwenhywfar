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

  static int GWENHYWFAR_CB SetIntProperty(GWEN_WIDGET *w,
                                          GWEN_DIALOG_PROPERTY prop,
                                          int index,
                                          int value,
                                          int doSignal);

  static int GWENHYWFAR_CB GetIntProperty(GWEN_WIDGET *w,
                                          GWEN_DIALOG_PROPERTY prop,
                                          int index,
                                          int defaultValue);

  static int GWENHYWFAR_CB SetCharProperty(GWEN_WIDGET *w,
      GWEN_DIALOG_PROPERTY prop,
      int index,
      const char *value,
      int doSignal);

  static const char *GWENHYWFAR_CB GetCharProperty(GWEN_WIDGET *w,
      GWEN_DIALOG_PROPERTY prop,
      int index,
      const char *defaultValue);

  static int GWENHYWFAR_CB AddChildGuiWidget(GWEN_WIDGET *w, GWEN_WIDGET *wChild);


  static void GWENHYWFAR_CB freeData(void *bp, void *p);

};




#endif /* CPPDIALOG_P_HPP */


