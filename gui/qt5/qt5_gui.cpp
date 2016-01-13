/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
                  (C) 2016 by Christian David
    email       : martin@libchipcard.de
                  christian-david@web.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qt5_gui.hpp"
#include "qt5_gui_dialog.hpp"

#include <gwenhywfar/debug.h>

#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>

#include <assert.h>


QT5_Gui::QT5_Gui()
  :CppGui()
  ,_parentWidget(NULL) {

  GWEN_Gui_AddFlags(_gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(_gui);
  GWEN_Gui_SetName(_gui, "qt5-gui");
}



QT5_Gui::~QT5_Gui() {
}



void QT5_Gui::pushParentWidget(QWidget *w) {
  if (_parentWidget)
    _pushedParents.push_back(_parentWidget);
  _parentWidget=w;
}



void QT5_Gui::popParentWidget() {
  if (!_pushedParents.empty()) {
    _parentWidget=_pushedParents.back();
    _pushedParents.pop_back();
  }
  else
    _parentWidget=NULL;
}



QString QT5_Gui::extractHtml(const char *text) {
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



int QT5_Gui::execDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  QT5_GuiDialog qt5Dlg(this, dlg);
  QWidget *owner = qApp->activeWindow();

  /* setup widget tree for the dialog */
  if (!(qt5Dlg.setup(owner))) {
    return GWEN_ERROR_GENERIC;
  }

  return qt5Dlg.execute();
}



int QT5_Gui::openDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  QT5_GuiDialog *qtDlg = new QT5_GuiDialog(this, dlg);
  QWidget *owner = qApp->activeWindow();

  /* setup widget tree for the dialog */
  if (!(qtDlg->setup(owner))) {
    delete qtDlg;
    return GWEN_ERROR_GENERIC;
  }

  return qtDlg->openDialog();
}



int QT5_Gui::closeDialog(GWEN_DIALOG *dlg) {
  QT5_GuiDialog *qtDlg = QT5_GuiDialog::getDialog(dlg);
  assert(qtDlg);

  int rv = qtDlg->closeDialog();
  delete qtDlg;
  return rv;
}



int QT5_Gui::runDialog(GWEN_DIALOG *dlg, int untilEnd) {
  QT5_GuiDialog *qtDlg = QT5_GuiDialog::getDialog(dlg);
  assert(qtDlg);

  return qtDlg->runDialog((untilEnd==0)?false:true);
}



int QT5_Gui::getFileName(const char *caption,
                         GWEN_GUI_FILENAME_TYPE fnt,
                         uint32_t flags,
                         const char *patterns,
                         GWEN_BUFFER *pathBuffer,
                         uint32_t guiid) {
  QString sCaption;
  QString sPatterns;
  QString sPath;
  QString str;
  QWidget *owner = qApp->activeWindow();

  if (caption)
    sCaption=QString::fromUtf8(caption);

  if (patterns) {
    const char *s1;
    const char *s2;

    s1=patterns;
    qDebug("Patterns example: '%s'", patterns);
    //! @todo Create pattern correctly
    while(s1 && *s1) {
      s2=strchr(s1, '\t');
      if (s2) {
        str=QString::fromUtf8(s1, s2-s1);
        /* skip tab */
        s2++;
      }
      else {
        str=QString::fromUtf8(s1);
        s2=NULL;
      }
      str.replace(',', ' ');
      str.replace(';', ' ');

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









