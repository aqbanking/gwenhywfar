/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2021 by Martin Preuss
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

/* included from gui.c */



void GWEN_Gui_UseDialogs(GWEN_GUI *gui)
{
  assert(gui);
  DBG_INFO(GWEN_LOGDOMAIN, "Using own callbacks in gui %p", gui);
  gui->progressStartFn=GWEN_Gui_DialogBased_ProgressStart;
  gui->progressAdvanceFn=GWEN_Gui_DialogBased_ProgressAdvance;
  gui->progressSetTotalFn=GWEN_Gui_DialogBased_ProgressSetTotal;
  gui->progressLogFn=GWEN_Gui_DialogBased_ProgressLog;
  gui->progressEndFn=GWEN_Gui_DialogBased_ProgressEnd;
  gui->inputBoxFn=GWEN_Gui_DialogBased_InputBox;
  gui->messageBoxFn=GWEN_Gui_DialogBased_MessageBox;
  gui->showBoxFn=GWEN_Gui_DialogBased_ShowBox;
  gui->hideBoxFn=GWEN_Gui_DialogBased_HideBox;
}



int GWEN_Gui_ShowProgress(GWEN_PROGRESS_DATA *pd)
{
  GWEN_PROGRESS_DATA *highest=NULL;
  GWEN_PROGRESS_DATA *t;
  GWEN_DIALOG *dlg=NULL;

  assert(gwenhywfar_gui);

  t=pd;
  while (t) {
    highest=t;
    t=GWEN_ProgressData_Tree_GetParent(t);
  }

  /* highest must always be visible */
  if (GWEN_ProgressData_GetShown(highest)==0)
    GWEN_ProgressData_SetShown(highest, 1);

  dlg=GWEN_ProgressData_GetDialog(highest);
  if (dlg==NULL) {
    int rv;

    /* need to create dialog for it */
    dlg=GWEN_DlgProgress_new();
    if (dlg==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unable to create progress dialog, maybe data not installed?");
      return GWEN_ERROR_INTERNAL;
    }
    if (GWEN_ProgressData_GetFlags(pd) & GWEN_GUI_PROGRESS_KEEP_OPEN)
      GWEN_DlgProgress_SetStayOpen(dlg, 1);

    if (GWEN_ProgressData_GetFlags(pd) & GWEN_GUI_PROGRESS_SHOW_LOG)
      GWEN_DlgProgress_SetShowLog(dlg, 1);

    rv=GWEN_Gui_OpenDialog(dlg, 0);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unable to openDialog: %d", rv);
      GWEN_Dialog_free(dlg);
      return rv;
    }

    DBG_INFO(GWEN_LOGDOMAIN, "Setting new firstprogress: %08x",
             GWEN_ProgressData_GetId(pd));
    GWEN_DlgProgress_SetFirstProgress(dlg, highest);
    GWEN_ProgressData_SetDialog(highest, dlg);
  }

  if (pd!=highest) {
    DBG_INFO(GWEN_LOGDOMAIN, "Setting new second progress: %08x",
             GWEN_ProgressData_GetId(pd));
    GWEN_DlgProgress_SetSecondProgress(dlg, pd);
    GWEN_ProgressData_SetDialog(pd, dlg);
    GWEN_ProgressData_SetShown(pd, 1);
  }

  GWEN_Gui_RunDialog(dlg, 0);

  return 0;
}




void GWEN_Gui_DialogBased_CheckShow(GWEN_UNUSED GWEN_GUI *gui, GWEN_PROGRESS_DATA *pd)
{
  if (GWEN_ProgressData_GetShown(pd)==0) {
    if (GWEN_ProgressData_GetFlags(pd) & GWEN_GUI_PROGRESS_DELAY) {
      double dt;
      time_t t1;

      t1=time(0);
      dt=difftime(t1, GWEN_ProgressData_GetStartTime(pd));
      if ((int)dt>=GWEN_GUI_DELAY_SECS) {
        DBG_INFO(GWEN_LOGDOMAIN, "Progress %08x open for %d secs, showing",
                 GWEN_ProgressData_GetId(pd), (int) dt);
        GWEN_ProgressData_SetShown(pd, 1);
      }
    }
    else
      GWEN_ProgressData_SetShown(pd, 1);
  }

  if (GWEN_ProgressData_GetShown(pd)==1) {
    if (GWEN_ProgressData_GetDialog(pd)==NULL) {
      GWEN_Gui_ShowProgress(pd);
    }
  }
}



uint32_t GWEN_Gui_DialogBased_ProgressStart(GWEN_GUI *gui,
                                         uint32_t progressFlags,
                                         const char *title,
                                         const char *text,
                                         uint64_t total,
                                         uint32_t guiid)
{
  GWEN_PROGRESS_DATA *pdParent=NULL;
  GWEN_PROGRESS_DATA *pd;
  uint32_t id;

  id=++(gui->nextProgressId);

  DBG_DEBUG(GWEN_LOGDOMAIN, "ProgressStart: flags=%08x, title=[%s], total=%08x, guiid=%08x",
            progressFlags, title?title:"(none)", (uint32_t) total, guiid);

  if (guiid==0) {
    guiid=gui->lastProgressId;
  }

  if (guiid) {
    pdParent=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, guiid);
    if (pdParent==NULL) {
      DBG_WARN(GWEN_LOGDOMAIN, "Parent progress by id %08x not found", guiid);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Title: [%s], Text: [%s]",
                title?title:"no title",
                text?text:"no text");
    }
  }

  pd=GWEN_ProgressData_new(gui, id, progressFlags, title, text, total);
  assert(pd);
  GWEN_ProgressData_SetPreviousId(pd, gui->lastProgressId);
  if (pdParent)
    GWEN_ProgressData_Tree_AddChild(pdParent, pd);
  else
    GWEN_ProgressData_Tree_Add(gui->progressDataTree, pd);

  GWEN_Gui_DialogBased_CheckShow(gui, pd);

  gui->lastProgressId=id;

  return id;
}



int GWEN_Gui_DialogBased_ProgressEnd(GWEN_GUI *gui, uint32_t pid)
{
  GWEN_PROGRESS_DATA *pd;
  /*uint32_t parentPid=0;*/

  DBG_DEBUG(GWEN_LOGDOMAIN, "ProgressEnd: guiid=%08x", pid);

  if (pid==0) {
    pid=gui->lastProgressId;
    if (pid==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
      return GWEN_ERROR_INVALID;
    }
  }

  pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
  if (pd==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DIALOG *dlg;
    GWEN_PROGRESS_DATA *previousPd;

    /* set previous progress id */
    gui->lastProgressId=GWEN_ProgressData_GetPreviousId(pd);

    /* find next highest active progress */
    previousPd=GWEN_ProgressData_Tree_GetParent(pd);
    /*if (previousPd)
      parentPid=GWEN_ProgressData_GetId(previousPd);*/
    while (previousPd) {
      if (GWEN_ProgressData_GetShown(previousPd))
        break;
      previousPd=GWEN_ProgressData_Tree_GetParent(previousPd);
    }

    dlg=GWEN_ProgressData_GetDialog(pd);
    if (dlg) {
      GWEN_PROGRESS_DATA *primary;
      GWEN_PROGRESS_DATA *secondary;

      primary=GWEN_DlgProgress_GetFirstProgress(dlg);
      secondary=GWEN_DlgProgress_GetSecondProgress(dlg);

      /* force update of progress bar */
      GWEN_DlgProgress_Advanced(dlg, pd);
      GWEN_Gui_RunDialog(dlg, 0);

      if (primary==pd) {
        int rv;

        DBG_DEBUG(GWEN_LOGDOMAIN, "Progress %08x is primary, closing dialog",
                  GWEN_ProgressData_GetId(pd));

        if (secondary) {
          DBG_WARN(GWEN_LOGDOMAIN, "There is still a secondary progress!");
          GWEN_DlgProgress_SetSecondProgress(dlg, NULL);
          GWEN_ProgressData_SetDialog(secondary, NULL);
        }

        /* this is the primary progress, with this closed we can also
         * close the dialog */
        DBG_INFO(GWEN_LOGDOMAIN, "Closing progress dialog");
        GWEN_DlgProgress_AddLogText(dlg, GWEN_LoggerLevel_Info, I18N("Operation finished, you can now close this window."));

        // run dialog until end, close then
        GWEN_DlgProgress_SetAllowClose(dlg, 1);
        if (GWEN_DlgProgress_GetStayOpen(dlg)) {
          rv=GWEN_Gui_RunDialog(dlg, 1);
          if (rv<0) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Unable to runDialog: %d", rv);
            /*GWEN_Dialog_free(dlg);
            return rv;*/
          }
        }

        rv=GWEN_Gui_CloseDialog(dlg);
        if (rv<0) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Unable to closeDialog: %d", rv);
          GWEN_Dialog_free(dlg);
          return rv;
        }
        GWEN_Dialog_free(dlg);
      }
      else if (secondary==pd) {
        /* t is maybe the next higher progress, it will become the second progress */
        if (previousPd && previousPd!=GWEN_DlgProgress_GetFirstProgress(dlg)) {
          DBG_DEBUG(GWEN_LOGDOMAIN, "Progress %08x becomes new second progress",
                    GWEN_ProgressData_GetId(previousPd));
          GWEN_DlgProgress_SetSecondProgress(dlg, pd);
          GWEN_ProgressData_SetDialog(pd, dlg);
        }
        else {
          DBG_INFO(GWEN_LOGDOMAIN, "No next secondary progress");
          GWEN_DlgProgress_SetSecondProgress(dlg, NULL);
        }
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Progress %08x is neither primary nor secondary, SNH",
                  GWEN_ProgressData_GetId(pd));
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Progress %08x has no dialog", GWEN_ProgressData_GetId(pd));
    }

    GWEN_ProgressData_SetDialog(pd, NULL);
    GWEN_ProgressData_Tree_Del(pd);
    GWEN_ProgressData_free(pd);
  }

  return 0;
}



int GWEN_Gui_DialogBased_ProgressAdvance(GWEN_GUI *gui, uint32_t pid, uint64_t progress)
{
  GWEN_PROGRESS_DATA *pd;
  int aborted=0;

  if (pid==0) {
    pid=gui->lastProgressId;
    if (pid==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
      return GWEN_ERROR_INVALID;
    }
  }

  pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
  if (pd==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DIALOG *dlg;

    if (progress==GWEN_GUI_PROGRESS_ONE)
      progress=GWEN_ProgressData_GetCurrent(pd)+1;
    else if (progress==GWEN_GUI_PROGRESS_NONE)
      progress=GWEN_ProgressData_GetCurrent(pd);
    GWEN_ProgressData_SetCurrent(pd, progress);
    GWEN_Gui_DialogBased_CheckShow(gui, pd);

    dlg=GWEN_ProgressData_GetDialog(pd);
    if (dlg) {
      time_t t0;
      time_t t1;

      t0=GWEN_ProgressData_GetCheckTime(pd);
      t1=time(0);
      if (t0!=t1) {
        GWEN_DlgProgress_Advanced(dlg, pd);
        GWEN_Gui_RunDialog(dlg, 0);
        GWEN_ProgressData_SetCheckTime(pd, t1);
      }
    }
    aborted=GWEN_ProgressData_GetAborted(pd);
  }

  if (aborted)
    return GWEN_ERROR_USER_ABORTED;
  return 0;
}



int GWEN_Gui_DialogBased_ProgressSetTotal(GWEN_GUI *gui, uint32_t pid, uint64_t total)
{
  GWEN_PROGRESS_DATA *pd;
  int aborted=0;

  if (pid==0) {
    pid=gui->lastProgressId;
    if (pid==0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
      return GWEN_ERROR_INVALID;
    }
  }

  pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
  if (pd==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DIALOG *dlg;

    GWEN_ProgressData_SetTotal(pd, total);
    GWEN_Gui_DialogBased_CheckShow(gui, pd);

    dlg=GWEN_ProgressData_GetDialog(pd);
    if (dlg) {
      time_t t0;
      time_t t1;

      t0=GWEN_ProgressData_GetCheckTime(pd);
      t1=time(0);
      if (t0!=t1) {
        GWEN_DlgProgress_TotalChanged(dlg, pd);
        GWEN_Gui_RunDialog(dlg, 0);
        GWEN_ProgressData_SetCheckTime(pd, t1);
      }
    }
    aborted=GWEN_ProgressData_GetAborted(pd);
  }

  if (aborted)
    return GWEN_ERROR_USER_ABORTED;
  return 0;
}



int GWEN_Gui_DialogBased_ProgressLog(GWEN_GUI *gui,
                                  uint32_t pid,
                                  GWEN_LOGGER_LEVEL level,
                                  const char *text)
{
  assert(gui);

  /* only show messages with log level lower or equal threshold */
  if (level<=gui->minProgressLogLevel) {
    GWEN_PROGRESS_DATA *pd;
    int aborted=0;

    if (pid==0) {
      pid=gui->lastProgressId;
      if (pid==0) {
        DBG_INFO(GWEN_LOGDOMAIN, "Last active progress not available");
        return GWEN_ERROR_INVALID;
      }
    }

    pd=GWEN_ProgressData_Tree_FindProgressById(gui->progressDataTree, pid);
    if (pd==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Progress by id %08x not found", pid);
      return GWEN_ERROR_INVALID;
    }
    else {
      GWEN_DIALOG *dlg;

      if (level<=GWEN_LoggerLevel_Notice)
        GWEN_ProgressData_SetShown(pd, 1);
      if (level<=GWEN_LoggerLevel_Warning)
        GWEN_ProgressData_AddFlags(pd, GWEN_GUI_PROGRESS_KEEP_OPEN);
      GWEN_Gui_DialogBased_CheckShow(gui, pd);

      dlg=GWEN_ProgressData_GetDialog(pd);
      if (dlg) {
        if (level<=GWEN_LoggerLevel_Warning) {
          GWEN_DlgProgress_SetStayOpen(dlg, 1);
          GWEN_DlgProgress_SetShowLog(dlg, 1);
        }

        GWEN_DlgProgress_AddLogText(dlg, level, text);
        GWEN_Gui_RunDialog(dlg, 0);
      }
      else
        GWEN_ProgressData_AddLogText(pd, level, text);

      aborted=GWEN_ProgressData_GetAborted(pd);
    }

    if (aborted)
      return GWEN_ERROR_USER_ABORTED;
  }
  return 0;
}



int GWEN_Gui_DialogBased_InputBox(GWEN_UNUSED GWEN_GUI *gui,
                               uint32_t flags,
                               const char *title,
                               const char *text,
                               char *buffer,
                               int minLen,
                               int maxLen,
                               GWEN_UNUSED uint32_t guiid)
{
  GWEN_DIALOG *dlg;
  int rv;

  dlg=GWEN_DlgInput_new(flags, title, text, minLen, maxLen);
  if (dlg==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create dialog");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_Gui_ExecDialog(dlg, 0);
  if (rv==1) {
    rv=GWEN_DlgInput_CopyInput(dlg, buffer, maxLen);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Dialog_free(dlg);
      return rv;
    }
    if (GWEN_DlgInput_GetFlagAllowStore(dlg))
      rv=1;
    else
      rv=0;
    GWEN_Dialog_free(dlg);
    return rv;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "User aborted");
    GWEN_Dialog_free(dlg);
    return GWEN_ERROR_USER_ABORTED;
  }
}



int GWEN_Gui_DialogBased_MessageBox(GWEN_UNUSED GWEN_GUI *gui,
                                 uint32_t flags,
                                 const char *title,
                                 const char *text,
                                 const char *b1,
                                 const char *b2,
                                 const char *b3,
                                 GWEN_UNUSED uint32_t guiid)
{
  GWEN_DIALOG *dlg;
  int rv;

  dlg=GWEN_DlgMessage_new(flags, title, text, b1, b2, b3);
  if (dlg==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create dialog");
    return GWEN_ERROR_INTERNAL;
  }

  GWEN_Gui_ExecDialog(dlg, 0);
  rv=GWEN_DlgMessage_GetResponse(dlg);
  GWEN_Dialog_free(dlg);
  return rv;
}



uint32_t GWEN_Gui_DialogBased_ShowBox(GWEN_GUI *gui,
                                   uint32_t flags,
                                   const char *title,
                                   const char *text,
                                   uint32_t guiid)
{
  GWEN_DIALOG *dlg;
  int rv;
  uint32_t id;

  id=++(gui->nextDialogId);

  dlg=GWEN_DlgShowBox_new(flags, title, text);
  if (dlg==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create dialog");
    return 0;
  }

  GWEN_Dialog_SetGuiId(dlg, id);

  rv=GWEN_Gui_OpenDialog(dlg, guiid);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Dialog_free(dlg);
    return 0;
  }

  GWEN_Dialog_List_Add(dlg, gui->activeDialogs);

  return id;
}



void GWEN_Gui_DialogBased_HideBox(GWEN_GUI *gui, uint32_t id)
{
  GWEN_DIALOG *dlg;

  if (id) {
    dlg=GWEN_Dialog_List_First(gui->activeDialogs);
    while (dlg) {
      if (GWEN_Dialog_GetGuiId(dlg)==id)
        break;
      dlg=GWEN_Dialog_List_Next(dlg);
    }
  }
  else
    dlg=GWEN_Dialog_List_Last(gui->activeDialogs);

  if (dlg) {
    int rv;

    rv=GWEN_Gui_CloseDialog(dlg);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    }
    GWEN_Dialog_List_Del(dlg);
    GWEN_Dialog_free(dlg);
  }
}



