/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qt4_gui.hpp"
#include "qt4_gui_dialog.hpp"

#include <gwenhywfar/debug.h>

#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>

#include <assert.h>




QT4_Gui::QT4_Gui()
  :CppGui()
  ,_parentWidget(NULL) {

  GWEN_Gui_AddFlags(_gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(_gui);
  GWEN_Gui_SetName(_gui, "qt4-gui");
}



QT4_Gui::~QT4_Gui() {
}



void QT4_Gui::pushParentWidget(QWidget *w) {
  if (_parentWidget)
    _pushedParents.push_back(_parentWidget);
  _parentWidget=w;
}



void QT4_Gui::popParentWidget() {
  if (!_pushedParents.empty()) {
    _parentWidget=_pushedParents.back();
    _pushedParents.pop_back();
  }
  else
    _parentWidget=NULL;
}



QString QT4_Gui::extractHtml(const char *text) {
  const char *p=0;
  const char *p2=0;

  if (text==NULL)
    return QString("");

  /* find begin of HTML area */
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
            t++;
            if (toupper(*t)=='>') {
              break;
            }
          }
        }
      }
    }
    p++;
  } /* while */

  /* find end of HTML area */
  if (p) {
    p+=6; /* skip "<html>" */
    p2=p;
    while ((p2=strchr(p2, '<'))) {
      const char *t;

      t=p2;
      t++;
      if (toupper(*t)=='/') {
        t++;
        if (toupper(*t)=='H') {
          t++;
          if (toupper(*t)=='T') {
            t++;
            if (toupper(*t)=='M') {
              t++;
              if (toupper(*t)=='L') {
                t++;
                if (toupper(*t)=='>') {
                  break;
                }
              }
            }
          }
        }
      }
      p2++;
    } /* while */
  }

  if (p && p2)
    return QString("<qt>")+QString::fromUtf8(p, p2-p)+QString("</qt>");

  return QString::fromUtf8(text);
}



int QT4_Gui::execDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  QT4_GuiDialog qt4Dlg(this, dlg);
  QWidget *owner=qApp->activeWindow();

  /* setup widget tree for the dialog */
  if (!(qt4Dlg.setup(owner))) {
    return GWEN_ERROR_GENERIC;
  }

  return qt4Dlg.execute();
}



int QT4_Gui::openDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  QT4_GuiDialog *qt4Dlg;
  QWidget *owner=qApp->activeWindow();

  qt4Dlg=new QT4_GuiDialog(this, dlg);

  /* setup widget tree for the dialog */
  if (!(qt4Dlg->setup(owner))) {
    delete qt4Dlg;
    return GWEN_ERROR_GENERIC;
  }

  return qt4Dlg->openDialog();
}



int QT4_Gui::closeDialog(GWEN_DIALOG *dlg) {
  QT4_GuiDialog *qt4Dlg;
  int rv;

  qt4Dlg=QT4_GuiDialog::getDialog(dlg);
  assert(qt4Dlg);

  rv=qt4Dlg->closeDialog();
  delete qt4Dlg;
  return rv;
}



int QT4_Gui::runDialog(GWEN_DIALOG *dlg, int untilEnd) {
  QT4_GuiDialog *qt4Dlg;

  qt4Dlg=QT4_GuiDialog::getDialog(dlg);
  assert(qt4Dlg);

  return qt4Dlg->runDialog((untilEnd==0)?false:true);
}



int QT4_Gui::getFileName(const char *caption,
                         GWEN_GUI_FILENAME_TYPE fnt,
                         uint32_t flags,
                         const char *patterns,
                         GWEN_BUFFER *pathBuffer,
                         uint32_t guiid) {
  QString sCaption;
  QString sPatterns;
  QString sPath;
  QString str;
  QWidget *owner=qApp->activeWindow();

  if (caption)
    sCaption=QString::fromUtf8(caption);

  if (patterns) {
    const char *s1;
    const char *s2;

    s1=patterns;
    while(s1 && *s1) {
      s2=strchr(s1, '\t');
      if (s2) {
        str=QString::fromUtf8(s1, s2-s1);
        str.replace(',', ' ');
        str.replace(';', ' ');
        /* skip tab */
        s2++;
      }
      else {
        str=QString::fromUtf8(s1);
        str.replace(',', ' ');
        str.replace(';', ' ');
        s2=NULL;
      }

      if (!str.isEmpty())
        sPatterns+=";;";
      sPatterns+=str;

      s1=s2;
    }
  }

  if (GWEN_Buffer_GetUsedBytes(pathBuffer))
    sPath=QString::fromUtf8(GWEN_Buffer_GetStart(pathBuffer));

  switch(fnt) {
  case GWEN_Gui_FileNameType_OpenFileName:
    str=QFileDialog::getOpenFileName(owner, sCaption, sPath, sPatterns);
    break;

  case GWEN_Gui_FileNameType_SaveFileName:
    str=QFileDialog::getSaveFileName(owner, sCaption, sPath, sPatterns);
    break;

  case GWEN_Gui_FileNameType_OpenDirectory:
    str=QFileDialog::getExistingDirectory(owner, sCaption, sPath);
    break;
  }

  if (str.isEmpty()) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty filename returned.");
    return GWEN_ERROR_ABORTED;
  }
  else {
    GWEN_Buffer_Reset(pathBuffer);
    GWEN_Buffer_AppendString(pathBuffer, str.toUtf8());
    return 0;
  }
}









