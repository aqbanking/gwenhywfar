/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "filedialog_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/event.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/ui/window.h>
#include <gwenhywfar/ui/scrollwidget.h>
#include <gwenhywfar/ui/tablewidget.h>
#include <gwenhywfar/ui/textwidget.h>
#include <gwenhywfar/ui/dropdownbox.h>
#include <gwenhywfar/ui/editbox.h>
#include <gwenhywfar/ui/button.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <ctype.h>


GWEN_INHERIT(GWEN_WIDGET, GWEN_FILEDIALOG)



GWEN_WIDGET *GWEN_FileDialog_new(GWEN_TYPE_UINT32 flags,
                                 const char *name,
                                 const char *title,
                                 const char *dir,
                                 const char *fileName,
                                 const GWEN_STRINGLIST *patterns){
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *mwp;
  GWEN_WIDGET *wScroller;
  GWEN_FILEDIALOG *win;
  const int ww=70;
  const int wh=21;
  int x, y;

  mw=GWEN_Window_new(0,
                     (flags & ~GWEN_WIDGET_FLAGS_WINDOWFLAGS)|
                     GWEN_WIDGET_FLAGS_MODAL |
                     GWEN_WIDGET_FLAGS_BORDER,
                     name,
                     title,
                     (GWEN_UI_GetCols()-ww)/2,
                     (GWEN_UI_GetLines()-wh)/2,
                     ww,
                     wh);
  GWEN_Widget_SetTypeName(mw, "FileDialog");
  GWEN_NEW_OBJECT(GWEN_FILEDIALOG, win);
  GWEN_INHERIT_SETDATA(GWEN_WIDGET, GWEN_FILEDIALOG, mw, win,
                       GWEN_FileDialog_freeData);

  win->previousHandler=GWEN_Widget_GetEventHandler(mw);
  assert(win->previousHandler);
  GWEN_Widget_SetEventHandler(mw, GWEN_FileDialog_EventHandler);
  GWEN_Widget_SetRunFn(mw, GWEN_FileDialog_Run);
  win->flags=flags;

  if (dir) {
    win->currentDir=strdup(dir);
    win->startDir=strdup(dir);
  }
  else {
    win->currentDir=strdup(".");
    win->startDir=strdup(".");
  }

  wScroller=GWEN_ScrollWidget_new(GWEN_Window_GetViewPort(mw),
                                  GWEN_WIDGET_FLAGS_DEFAULT |
                                  GWEN_SCROLLWIN_FLAGS_VSLIDER |
                                  GWEN_SCROLLWIN_FLAGS_PASSIVE_SLIDERS |
                                  GWEN_WIDGET_FLAGS_BORDER,
                                  "Scroller",
                                  0, 0, 0, wh-7);

  w=GWEN_TableWidget_new(GWEN_ScrollWidget_GetViewPort(wScroller),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_TABLEWIDGET_FLAGS_COLBORDER |
                         GWEN_TABLEWIDGET_FLAGS_FIXED |
                         GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT |
                         ((flags & GWEN_FILEDIALOG_FLAGS_MULTI)
                          ?GWEN_TABLEWIDGET_FLAGS_SELECTION:0) |
                         GWEN_TABLEWIDGET_FLAGS_LINEMODE,
                         "FileTable",
                         0, 0, 0, 0);
  //GWEN_Widget_Subscribe(w, GWEN_EventType_Chosen, mw);
  win->wTable=w;
  GWEN_TextWidget_SetVirtualSize(w, 200, 200);
  GWEN_TableWidget_AddColumn(w, 30); /* file name */
  GWEN_TableWidget_AddColumn(w, 8); /* size or "<DIR>" */
  GWEN_TableWidget_AddColumn(w, 16); /* date */


  x=0;
  y=GWEN_Widget_GetHeight(wScroller)+1;

  mwp=GWEN_Window_GetViewPort(mw);

  /* patterns */
  w=GWEN_TextWidget_new(mwp,
                        GWEN_WIDGET_FLAGS_DEFAULT &
                        ~GWEN_WIDGET_FLAGS_FOCUSABLE,
                        "Label1",
                        "<gwen>Patterns</gwen>",
                        x, y, 15, 1);
  x+=GWEN_Widget_GetWidth(w)+1;

  GWEN_TextWidget_SetVirtualSize(win->wTable,
                                 200, 100);

  w=GWEN_DropDownBox_new(mwp,
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT|
                         GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_EDITBOX_FLAGS_EDIT,
                         "DropDown-Patterns",
                         x, y,
                         GWEN_Widget_GetWidth(mwp)-2-x, 1,
                         patterns);
  GWEN_Widget_Subscribe(w, GWEN_EventType_Changed, mw);
  
  y+=GWEN_Widget_GetHeight(w);
  x=0;
  win->wPatterns=w;

  /* selection */
  w=GWEN_TextWidget_new(mwp,
                        GWEN_WIDGET_FLAGS_DEFAULT &
                        ~GWEN_WIDGET_FLAGS_FOCUSABLE,
                        "Label2",
                        "<gwen>Selection</gwen>",
                        x, y, 15, 1);
  x+=GWEN_Widget_GetWidth(w)+1;

  w=GWEN_EditBox_new(GWEN_Window_GetViewPort(mw),
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_HIGHLIGHT|
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_EDITBOX_FLAGS_EDIT,
                     "EditBox-Selection",
                     fileName,
                     x, y,
                     GWEN_Widget_GetWidth(mwp)-2-x,
                     1, 256);
  y+=GWEN_Widget_GetHeight(w);
  x=0;
  win->wSelected=w;

  y++;
  w=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WIDGET_FLAGS_HCENTER |
                    GWEN_WIDGET_FLAGS_HIGHLIGHT,
                    "Button-Ok",
                    "Ok",
                    1,
                    x, y,
                    7, 1);
  x+=GWEN_Widget_GetWidth(w)+1;

  w=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WIDGET_FLAGS_HCENTER |
                    GWEN_WIDGET_FLAGS_HIGHLIGHT,
                    "Button-Abort",
                    "Abort",
                    1,
                    x, y,
                    7, 1);
  x+=GWEN_Widget_GetWidth(w)+1;


  GWEN_FileDialog_Update(mw);

  return mw;
}



void GWEN_FileDialog_freeData(void *bp, void *p) {
  GWEN_FILEDIALOG *win;

  win=(GWEN_FILEDIALOG*)p;
  free(win->currentDir);
  free(win->startDir);
  GWEN_FREE_OBJECT(win);
}



GWEN_UI_RESULT GWEN_FileDialog_EventHandler(GWEN_WIDGET *w, GWEN_EVENT *e) {
  GWEN_FILEDIALOG *win;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_FILEDIALOG, w);
  assert(win);
  assert(e);

  switch(GWEN_Event_GetType(e)) {
  case GWEN_EventType_Draw:
    break;

  case GWEN_EventType_Changed:
    if (GWEN_Event_DueToSubscription(e)) {
      GWEN_WIDGET *wSender;

      wSender=GWEN_Event_GetSender(e);
      if (wSender) {
        if (wSender==win->wPatterns ||
            GWEN_Widget_IsChildOf(wSender, win->wPatterns)) {
          GWEN_FileDialog_Files2Table(w, win->files);
          GWEN_Widget_Redraw(win->wTable);
        }
      }
      return GWEN_UIResult_Handled;
    }
    break;

  case GWEN_EventType_ContentChg:
    break;

  case GWEN_EventType_Highlight:
    break;

  case GWEN_EventType_Refresh:
    break;

  case GWEN_EventType_Update:
    break;

  case GWEN_EventType_Chosen: {
    const char *text;
    GWEN_WIDGET *wSender;

    wSender=GWEN_Event_GetSender(e);
    if (wSender) {
      if (wSender==win->wTable ||
          GWEN_Widget_IsChildOf(wSender, win->wTable)) {
        text=GWEN_EventChosen_GetText(e);
        if (text) {
          int x;
          int y;
          GWEN_BUFFER *buf;
          struct stat st;

          if (strcasecmp(text, ".")==0)
            break;

          x=GWEN_EventChosen_GetX(e);
          y=GWEN_EventChosen_GetY(e);
          buf=GWEN_Buffer_new(0, 256, 0, 1);

          if (strcasecmp(text, "..")==0) {
            char *p;

            p=strrchr(win->currentDir, '/');
            if (p) {
              /* shorten name by last element */
              *p=0;
              p=win->currentDir;
            }
            else {
              p=win->startDir;
            }
            GWEN_Buffer_AppendString(buf, p);
          }
          else {
            GWEN_Buffer_AppendString(buf, win->currentDir);
            GWEN_Buffer_AppendByte(buf, '/');
            GWEN_Buffer_AppendString(buf, text);
          }
          DBG_NOTICE(0, "Chosen: %s (%d, %d)", text, x, y);
          DBG_NOTICE(0, "Checking dir %s",
                     GWEN_Buffer_GetStart(buf));

          if (!stat(GWEN_Buffer_GetStart(buf), &st)) {
            if (S_ISDIR(st.st_mode)) {
              GWEN_DB_NODE *db;

              db=GWEN_DB_Group_new("files");
              if (!GWEN_FileDialog_ScanDir(w, GWEN_Buffer_GetStart(buf), db)) {
                GWEN_FileDialog_Files2Table(w, db);
                GWEN_DB_Group_free(win->files);
                win->files=db;
                free(win->currentDir);
                win->currentDir=strdup(GWEN_Buffer_GetStart(buf));
                GWEN_Widget_Redraw(win->wTable);
              }
              else {
                GWEN_DB_Group_free(db);
              }
            } /* if dir */
            else {
              /* filr */
              if (!(win->flags & GWEN_FILEDIALOG_FLAGS_WANTDIR) &&
                  !(win->flags & GWEN_FILEDIALOG_FLAGS_MULTI)) {
                /* user wanted a single file, we got one */
                GWEN_Widget_SetText(win->wSelected, text,
                                    GWEN_EventSetTextMode_Replace);
                GWEN_Buffer_free(buf);
                return GWEN_UIResult_Finished;
              }
            }
          } /* if stat succeeded */
          GWEN_Buffer_free(buf);
        }
      }
    }
    break;
  }

  default:
    break;
  } /* switch */

  return win->previousHandler(w, e);
}



int GWEN_FileDialog_Update(GWEN_WIDGET *w) {
  GWEN_FILEDIALOG *win;
  GWEN_DB_NODE *db;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_FILEDIALOG, w);
  assert(win);

  GWEN_DB_Group_free(win->files);
  win->files=0;
  db=GWEN_DB_Group_new("files");

  if (GWEN_FileDialog_ScanDir(w, win->currentDir, db)) {
    DBG_NOTICE(0, "Error scanning folder \"%s\"", win->currentDir);
  }
  else {
    if (!GWEN_FileDialog_Files2Table(w, db)) {
      win->files=db;
    }
  }
  GWEN_Widget_Redraw(win->wTable);

  return 0;
}



int GWEN_FileDialog_ScanDir(GWEN_WIDGET *w, const char *s, GWEN_DB_NODE *db){
  GWEN_DIRECTORYDATA *d;

  d=GWEN_Directory_new();
  if (!GWEN_Directory_Open(d, s)) {
    char buffer[256];

    while(!GWEN_Directory_Read(d, buffer, sizeof(buffer))) {
      if (strcasecmp(buffer, ".")) {
        GWEN_BUFFER *fbuf;
        struct stat st;
        GWEN_DB_NODE *dbEntry;

        fbuf=GWEN_Buffer_new(0, 256, 0, 1);
        GWEN_Buffer_AppendString(fbuf, s);
        GWEN_Buffer_AppendByte(fbuf, '/');
        GWEN_Buffer_AppendString(fbuf, buffer);
        if (!stat(GWEN_Buffer_GetStart(fbuf), &st)) {
          struct tm *lt;

          if (S_ISDIR(st.st_mode)) {
            dbEntry=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP,
                                     "dir");
            DBG_NOTICE(0, "Dir: %s", buffer);
          }
          else {
            char numbuf[32];
            const char *suffixes=" KMGT";
            const char *p;
            size_t rest;

            DBG_NOTICE(0, "File: %s", buffer);
            dbEntry=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP,
                                     "file");
            p=suffixes;
            rest=st.st_size;
            if (rest/1024) {
              p++;         /* KB */
              rest/=1024;
              if (rest/1024) {
                p++;       /* MB */
                rest/=1024;
                if (rest/1024) {
                  p++;       /* GB */
                  rest/=1024;
                  if (rest/1024) {
                    p++;       /* TB */
                    rest/=1024;
                  }
                }
              }
            }

            if (isspace(*p))
              snprintf(numbuf, sizeof(numbuf), " %7d",
                       rest);
            else
              snprintf(numbuf, sizeof(numbuf), "%7d%c",
                       rest, *p);
            GWEN_DB_SetCharValue(dbEntry,
                                 GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "size",
                                 numbuf);
          }
          GWEN_DB_SetCharValue(dbEntry,
                               GWEN_DB_FLAGS_OVERWRITE_VARS,
                               "name", buffer);
          lt=localtime(&st.st_mtime);
          if (lt) {
            char numbuf[64];

            snprintf(numbuf, sizeof(numbuf),
                     "%04d/%02d/%02d %02d:%02d",
                     lt->tm_year+1900,
                     lt->tm_mon+1,
                     lt->tm_mday,
                     lt->tm_hour,
                     lt->tm_min);
            DBG_NOTICE(0, "Date: %s", numbuf);
            GWEN_DB_SetCharValue(dbEntry,
                                 GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "date", numbuf);
          }
        } /* if stat succeeded */
      }
    } /* if entry read */

    GWEN_Directory_Close(d);
  }
  else
    return -1;
  return 0;
}



int GWEN_FileDialog_Files2Table(GWEN_WIDGET *w, GWEN_DB_NODE *db){
  GWEN_FILEDIALOG *win;
  int i;
  GWEN_DB_NODE *dbEntry;
  GWEN_BUFFER *pbuf;
  const char *pattern;

  assert(w);
  win=GWEN_INHERIT_GETDATA(GWEN_WIDGET, GWEN_FILEDIALOG, w);
  assert(win);

  i=0;

  GWEN_TableWidget_Clear(win->wTable);
  if (!db) {
    return 0;
  }

  pbuf=GWEN_DropDownBox_GetText(win->wPatterns, 1);
  pattern=0;
  if (pbuf) {
    pattern=GWEN_Buffer_GetStart(pbuf);
    pattern=strchr(pattern, ';');
    if (pattern)
      pattern++;
  }
  if (!pattern)
    pattern="*";

  DBG_NOTICE(0, "ZZZ: Pattern is: %s", pattern);
  /* read directories */
  dbEntry=GWEN_DB_FindFirstGroup(db, "dir");
  while(dbEntry) {
    GWEN_TableWidget_SetText(win->wTable, 0, i,
                             GWEN_DB_GetCharValue(dbEntry, "name",
                                                  0, "(unnamed)"));
    GWEN_TableWidget_SetText(win->wTable, 1, i, " DIR");
    GWEN_TableWidget_SetText(win->wTable, 2, i,
                             GWEN_DB_GetCharValue(dbEntry,
                                                  "date",
                                                  0, "(no date)"));
    i++;
    dbEntry=GWEN_DB_FindNextGroup(dbEntry, "dir");
  } /* while */

  /* read directories */
  dbEntry=GWEN_DB_FindFirstGroup(db, "file");
  while(dbEntry) {
    const char *name;

    name=GWEN_DB_GetCharValue(dbEntry, "name",
                              0, "(unnamed)");
    if (-1!=GWEN_Text_ComparePattern(name, pattern, 0)) {
      GWEN_TableWidget_SetText(win->wTable, 0, i,
                               GWEN_DB_GetCharValue(dbEntry, "name",
                                                    0, "(unnamed)"));
      GWEN_TableWidget_SetText(win->wTable, 1, i,
                               GWEN_DB_GetCharValue(dbEntry, "size",
                                                    0, "(no size)"));
      GWEN_TableWidget_SetText(win->wTable, 2, i,
                               GWEN_DB_GetCharValue(dbEntry,
                                                    "date",
                                                    0, "(no date)"));
      i++;
    }
    dbEntry=GWEN_DB_FindNextGroup(dbEntry, "file");
  } /* while */

  win->entries=i;
  GWEN_TextWidget_SetVirtualSize(win->wTable,
                                 GWEN_TextWidget_GetVirtualWidth(win->wTable),
                                 i);

  GWEN_Buffer_free(pbuf);
  return 0;
}



int GWEN_FileDialog_Run(GWEN_WIDGET *w) {
  int response;

  response=0;
  for (;;) {
    GWEN_EVENT *e;
    GWEN_UI_RESULT res;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      if (GWEN_EventCommand_GetCommandId(e)==1) {
        response=1;
        break;
      }
      else if (GWEN_EventCommand_GetCommandId(e)==2) {
        response=2;
        break;
      }
      else if (GWEN_EventCommand_GetCommandId(e)==3) {
      }
      else
        res=GWEN_UI_DispatchEvent(e);
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
    if (res==GWEN_UIResult_Finished) {
      response=1;
      break;
    }
  } /* for */

  return response;
}









