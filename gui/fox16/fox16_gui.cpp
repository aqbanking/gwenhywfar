/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "fox16_gui.hpp"
#include "fox16_gui_dialog_l.hpp"
#include "fox16_gui_updater_l.hpp"

#include <gwenhywfar/debug.h>



FOX16_Gui::WinScope::WinScope(uint32_t parentId, FXWindow *w)
  : m_parentId(parentId)
  , m_id(0)
  , m_window(w)
  , m_type(WIN_SCOPE_TYPE_WINDOW) {
  FOX16_Gui *gui=FOX16_Gui::getFgGui();
  assert(gui);

  if (m_parentId==0)
    m_parentId=gui->getIdOfLastScope();
  m_id=gui->getNextId();
  gui->addWinScope(this);
}



FOX16_Gui::WinScope::WinScope(FXWindow *w)
  : m_parentId()
  , m_id(ID_MAINWINDOW)
  , m_window(w)
  , m_type(WIN_SCOPE_TYPE_WINDOW) {
  FOX16_Gui *gui=FOX16_Gui::getFgGui();
  assert(gui);

  gui->addWinScope(this);
}



FOX16_Gui::WinScope::WinScope(FOX16_Gui::WinScope::WIN_SCOPE_TYPE t, uint32_t parentId, FXWindow *w)
  : m_parentId(parentId)
  , m_id(0)
  , m_window(w)
  , m_type(t) {
  FOX16_Gui *gui=FOX16_Gui::getFgGui();
  assert(gui);

  if (m_parentId==0)
    m_parentId=gui->getIdOfLastScope();
  m_id=gui->getNextId();
  gui->addWinScope(this);
}



FOX16_Gui::WinScope::~WinScope() {
  FOX16_Gui *gui=FOX16_Gui::getFgGui();
  assert(gui);
  gui->delWinScope(this);
}






FOX16_Gui::FOX16_Gui(FXApp *a)
  :CppGui()
  ,m_app(a)
  ,m_lastId(0)
  ,m_updater()
  ,m_fontList(NULL) {
  m_updater=new FOX16_GuiUpdater();
  GWEN_Gui_AddFlags(_gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(_gui);
  GWEN_Gui_SetName(_gui, "fox16-gui");
  m_fontList=HtmlFont_List_new();
}



FOX16_Gui::~FOX16_Gui() {
  if (!m_scopeList.empty()) {
    DBG_ERROR(GWEN_LOGDOMAIN, "ScopeList is not empty!");
  }

  if (m_updater)
    delete m_updater;
  HtmlFont_List_free(m_fontList);
}



uint32_t FOX16_Gui::getNextId() {
  return ++m_lastId;
}



uint32_t FOX16_Gui::getIdOfLastScope() {
  if (!m_scopeList.empty())
    return m_scopeList.back()->getId();

  return 0;
}



void FOX16_Gui::addWinScope(WinScope *ws) {
  m_scopeList.push_back(ws);
}



void FOX16_Gui::delWinScope(WinScope *ws) {
  m_scopeList.remove(ws);
}



FOX16_Gui::WinScope *FOX16_Gui::findWinScope(uint32_t id) {
  WinScopePtrList::iterator it;

  for (it=m_scopeList.begin();
       it!=m_scopeList.end();
       it++) {
    if ((*it)->getId()==id)
      return (*it);
  }

  return NULL;
}



FXWindow *FOX16_Gui::getGuiWindow(uint32_t id) {
  return m_app->getActiveWindow();
}




FOX16_Gui *FOX16_Gui::getFgGui() {
  CppGui *cppgui;

  cppgui=CppGui::getCppGui();
  if (cppgui)
    return dynamic_cast<FOX16_Gui*>(cppgui);
  else
    return NULL;
}



void FOX16_Gui::dumpScopeList() {
  WinScopePtrList::iterator it;

  for (it=m_scopeList.begin();
       it!=m_scopeList.end();
       it++) {
    const char *s;

    switch((*it)->getType()) {
    case WinScope::WIN_SCOPE_TYPE_WINDOW:
      s="window";
      break;
    default:
      s="unknown";
      break;
    }
    fprintf(stderr, "WinScope: id %08x, parent %08x, type %s\n",
            (*it)->getId(),
            (*it)->getParentId(),
            s);
  }
}




int FOX16_Gui::print(const char *docTitle,
                     const char *docType,
                     const char *descr,
                     const char *text,
                     uint32_t guiid) {
  DBG_ERROR(GWEN_LOGDOMAIN, "Not implemented");
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



FXString FOX16_Gui::getRawText(const char *text) {
  const char *p=0;
  const char *p2=0;

  if (text==NULL)
    return FXString("");

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
    p2=p;
    p2+=6; /* skip "<html>" */
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

  if (p && p2) {
    p2+=7; /* skip "</html>" */

    int startPos=(p-text);
    int endPos=(p2-text);
    FXString result;

    result=FXString(text);
    result.erase(startPos, endPos);
    return result;
  }
  else
    return FXString(text);
}



FXString FOX16_Gui::getHtmlText(const char *text) {
  const char *p=0;
  const char *p2=0;

  if (text==NULL)
    return FXString("");

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
    return FXString(p, p2-p);
  else
    return FXString(text);
}



int FOX16_Gui::execDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  FOX16_GuiDialog foxDlg(this, dlg);
  FXWindow *owner;

  /* get main window of parent dialog (if any) */
  owner=m_app->getActiveWindow();

  /* setup widget tree for the dialog */
  if (!(foxDlg.setup(owner))) {
    return GWEN_ERROR_GENERIC;
  }

  return foxDlg.execute();
}



int FOX16_Gui::openDialog(GWEN_DIALOG *dlg, uint32_t guiid) {
  FOX16_GuiDialog *foxDlg;
  FXWindow *owner;

  /* get main window of parent dialog (if any) */
  owner=m_app->getActiveWindow();

  foxDlg=new FOX16_GuiDialog(this, dlg);

  /* setup widget tree for the dialog */
  if (!(foxDlg->setup(owner))) {
    delete foxDlg;
    return GWEN_ERROR_GENERIC;
  }

  foxDlg->openDialog();
  m_updater->guiUpdate();

  return 0;
}



int FOX16_Gui::closeDialog(GWEN_DIALOG *dlg) {
  FOX16_GuiDialog *foxDlg;

  foxDlg=FOX16_GuiDialog::getDialog(dlg);
  assert(foxDlg);

  foxDlg->closeDialog();
  delete foxDlg;
  m_updater->guiUpdate();

  return 0;
}



int FOX16_Gui::runDialog(GWEN_DIALOG *dlg, int untilEnd) {
  FOX16_GuiDialog *foxDlg;

  foxDlg=FOX16_GuiDialog::getDialog(dlg);
  assert(foxDlg);

  if (untilEnd)
    return foxDlg->cont();
  else {
    m_updater->guiUpdate();
    return 0;
  }
}



int FOX16_Gui::getFileName(const char *caption,
                           GWEN_GUI_FILENAME_TYPE fnt,
                           uint32_t flags,
                           const char *patterns,
                           GWEN_BUFFER *pathBuffer,
                           uint32_t guiid) {
  FXString sCaption;
  FXString sPatterns;
  FXString sPath;
  FXString str;
  FXWindow *owner;

  if (caption)
    sCaption=FXString(caption);

  if (patterns) {
    const char *s1;
    const char *s2;

    s1=patterns;
    while(s1 && *s1) {
      s2=strchr(s1, '\t');
      if (s2) {
        str=FXString(s1, s2-s1);
        /* skip tab */
        s2++;
      }
      else {
        str=FXString(s1);
        s2=NULL;
      }

      if (str.contains('(')) {
        if (!sPatterns.empty())
          sPatterns+='\n';
        sPatterns+=str.before('(');
        str=str.after('(');
        sPatterns+='(';
        sPatterns+=str.substitute(';', ',');
      }
      else {
        if (!sPatterns.empty())
          sPatterns+='\n';
        sPatterns+=str.substitute(';', ',');
      }

      s1=s2;
    }
  }

  if (GWEN_Buffer_GetUsedBytes(pathBuffer))
    sPath=FXString(GWEN_Buffer_GetStart(pathBuffer));

  owner=m_app->getModalWindow();
  if (owner==NULL) {
    owner=m_app->getActiveWindow();
  }
  if (owner==NULL) {
    owner=m_app->getRootWindow();
  }
  if (owner==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not determine owner window");
    return GWEN_ERROR_INTERNAL;
  }

  switch(fnt) {
  case GWEN_Gui_FileNameType_OpenFileName:
    str=FXFileDialog::getOpenFilename(owner, sCaption, sPath, sPatterns, 0);
    break;

  case GWEN_Gui_FileNameType_SaveFileName:
    str=FXFileDialog::getSaveFilename(owner, sCaption, sPath, sPatterns, 0);
    break;

  case GWEN_Gui_FileNameType_OpenDirectory:
    str=FXFileDialog::getOpenDirectory(owner, sCaption, sPath);
    break;
  }

  if (str.empty()) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty filename returned.");
    return GWEN_ERROR_ABORTED;
  }
  else {
    GWEN_Buffer_Reset(pathBuffer);
    GWEN_Buffer_AppendString(pathBuffer, str.text());
    return 0;
  }
}



HTML_FONT *FOX16_Gui::findFont(const char *fontName,
                               int fontSize,
                               uint32_t fontFlags) {
  HTML_FONT *fnt;

  assert(m_fontList);
  fnt=HtmlFont_List_First(m_fontList);
  while(fnt) {
    const char *s;

    s=HtmlFont_GetFontName(fnt);
    if (s && *s &&
        HtmlFont_GetFontSize(fnt)==fontSize &&
        HtmlFont_GetFontFlags(fnt)==fontFlags &&
        strcasecmp(s, fontName)==0)
      break;
    fnt=HtmlFont_List_Next(fnt);
  }

  return fnt;
}



HTML_FONT *FOX16_Gui::getFont(const char *fontName,
                              int fontSize,
                              uint32_t fontFlags) {
  HTML_FONT *fnt;

  fnt=findFont(fontName, fontSize, fontFlags);
  if (fnt)
    return fnt;
  else {
    fnt=HtmlFont_new();
    HtmlFont_SetFontName(fnt, fontName);
    HtmlFont_SetFontSize(fnt, fontSize);
    HtmlFont_SetFontFlags(fnt, fontFlags);
    HtmlFont_List_Add(fnt, m_fontList);
    return fnt;
  }
}





