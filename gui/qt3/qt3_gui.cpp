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

#include "qt3_gui.hpp"
#include "qt3_gui_dialog.hpp"
#include "qt3_printdialog.hpp"

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qapplication.h>
#include <qfiledialog.h>

#include <assert.h>




QT3_Gui::QT3_Gui()
:CppGui()
,_parentWidget(NULL)
{

  GWEN_Gui_AddFlags(_gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(_gui);
  GWEN_Gui_SetName(_gui, "qt3-gui");
}



QT3_Gui::~QT3_Gui() {
}



void QT3_Gui::pushParentWidget(QWidget *w) {
  if (_parentWidget)
    _pushedParents.push_back(_parentWidget);
  _parentWidget=w;
}



void QT3_Gui::popParentWidget() {
  if (!_pushedParents.empty()) {
    _parentWidget=_pushedParents.back();
    _pushedParents.pop_back();
  }
  else
    _parentWidget=NULL;
}



QString QT3_Gui::extractHtml(const char *text) {
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



std::string QT3_Gui::qstringToUtf8String(const QString &qs) {
  if (qs.isEmpty())
    return "";
  else {
    QCString utfData=qs.utf8();
    return utfData.data();
  }
}



int QT3_Gui::execDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  QT3_GuiDialog qt3Dlg(this, dlg);
  QWidget *owner=qApp->activeWindow();

  /* setup widget tree for the dialog */
  if (!(qt3Dlg.setup(owner))) {
    return GWEN_ERROR_GENERIC;
  }

  return qt3Dlg.execute();
}



int QT3_Gui::openDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  QT3_GuiDialog *qt3Dlg;
  QWidget *owner=qApp->activeWindow();

  qt3Dlg=new QT3_GuiDialog(this, dlg);

  /* setup widget tree for the dialog */
  if (!(qt3Dlg->setup(owner))) {
    delete qt3Dlg;
    return GWEN_ERROR_GENERIC;
  }

  return qt3Dlg->openDialog();
}



int QT3_Gui::closeDialog(GWEN_DIALOG *dlg) {
  QT3_GuiDialog *qt3Dlg;
  int rv;

  qt3Dlg=QT3_GuiDialog::getDialog(dlg);
  assert(qt3Dlg);

  rv=qt3Dlg->closeDialog();
  delete qt3Dlg;
  return rv;
}



int QT3_Gui::runDialog(GWEN_DIALOG *dlg, int untilEnd) {
  QT3_GuiDialog *qt3Dlg;

  qt3Dlg=QT3_GuiDialog::getDialog(dlg);
  assert(qt3Dlg);

  return qt3Dlg->runDialog((untilEnd==0)?false:true);
}



int QT3_Gui::getFileName(const char *caption,
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
	/* skip tab */
        s2++;
      }
      else {
	str=QString::fromUtf8(s1);
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
    str=QFileDialog::getOpenFileName(sPath, sPatterns,
				     owner,
                                     NULL, /* name */
				     sCaption);
    break;

  case GWEN_Gui_FileNameType_SaveFileName:
    str=QFileDialog::getSaveFileName(sPath, sPatterns,
				     owner,
				     NULL, /* name */
				     sCaption);
    break;

  case GWEN_Gui_FileNameType_OpenDirectory:
    str=QFileDialog::getExistingDirectory(sPath,
					  owner,
					  NULL,  /* name */
					  sCaption,
					  TRUE,  /* dirOnly */
					  TRUE); /* resolveSymlinks */
    break;
  }

  if (str.isEmpty()) {
    DBG_ERROR(0, "Empty filename returned.");
    return GWEN_ERROR_ABORTED;
  }
  else {
    GWEN_Buffer_Reset(pathBuffer);
    GWEN_Buffer_AppendString(pathBuffer, str.utf8());
    return 0;
  }
}



int QT3_Gui::print(const char *docTitle,
		   const char *docType,
		   const char *descr,
		   const char *text,
		   uint32_t guiid) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  int rv;

  buf1=GWEN_Buffer_new(0, strlen(descr)+32, 0, 1);

  if (!extractHTML(descr, buf1)) {
    descr=GWEN_Buffer_GetStart(buf1);
  }
  buf2=GWEN_Buffer_new(0, strlen(text)+32, 0, 1);
  if (!extractHTML(text, buf2)) {
    text=GWEN_Buffer_GetStart(buf2);
  }

  QT3_PrintDialog pdlg(docTitle, docType, descr, text, getParentWidget(),
		       "printdialog", true);

  if (pdlg.exec()==QDialog::Accepted)
    rv=0;
  else
    rv=GWEN_ERROR_USER_ABORTED;

  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);
  return rv;
}







