/***************************************************************************
 begin       : Thu Jan 09 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "testgui_p.h"

#include <gwenhywfar/fslock.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/debug.h>




static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int GWENHYWFAR_CB _logHook(GWEN_GUI *gui, const char *logDomain, GWEN_LOGGER_LEVEL priority, const char *s);

static void _addLogLineToList(GWEN_GUI *gui, const char *s);
static void _addLogLineToFile(GWEN_GUI *gui, const char *s);
static void _writeLinesToFile(GWEN_GUI *gui);




GWEN_INHERIT(GWEN_GUI, GWEN_GUI_TEST)




void TestGui_Extend(GWEN_GUI *gui)
{
  GWEN_GUI_TEST *xgui;

  GWEN_NEW_OBJECT(GWEN_GUI_TEST, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, GWEN_GUI_TEST, gui, xgui, _freeData);

  xgui->logStrings=GWEN_StringList_new();
  xgui->previousLogHook=GWEN_Gui_SetLogHookFn(gui, _logHook);
  xgui->logHookIsActive=1;
}



void TestGui_Unextend(GWEN_GUI *gui)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  /* reset log hook */
  GWEN_Gui_SetLogHookFn(gui, xgui->previousLogHook);

  GWEN_INHERIT_UNLINK(GWEN_GUI, GWEN_GUI_TEST, gui);

  free(xgui->logFile);
  GWEN_StringList_free(xgui->logStrings);
  GWEN_FREE_OBJECT(xgui);
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_GUI_TEST *xgui;

  xgui=(GWEN_GUI_TEST *)p;

  free(xgui->logFile);
  GWEN_StringList_free(xgui->logStrings);
  GWEN_FREE_OBJECT(xgui);
}



void Test_Gui_FlushLogs(GWEN_GUI *gui)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  _writeLinesToFile(gui);
  GWEN_StringList_Clear(xgui->logStrings);
}




int _logHook(GWEN_GUI *gui, const char *logDomain, GWEN_LOGGER_LEVEL priority, const char *s)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  if (xgui->logHookIsActive) {
    GWEN_BUFFER *tbuf;
    int rv;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Logger_CreateLogMessage(logDomain, priority, s, tbuf);
    if (rv<0) {
      fprintf(stderr, "Unable to generate log message, turning off logging.\n");
      xgui->logHookIsActive=0;
      GWEN_Buffer_free(tbuf);
      return 0;
    }

    TestGui_AddLogLine(gui, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
    return 1;
  }
  return 0;
}



void TestGui_AddLogLine(GWEN_GUI *gui, const char *s)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  if (xgui->logLastX)
    _addLogLineToList(gui, s);
  else
    _addLogLineToFile(gui, s);
}



void TestGui_SetLogFile(GWEN_GUI *gui, const char *fname)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  if (xgui->logFile)
    free(xgui->logFile);
  if (fname)
    xgui->logFile=strdup(fname);
  else
    xgui->logFile=NULL;
}



void TestGui_SetLogLastX(GWEN_GUI *gui, int i)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  xgui->logLastX=i;
}



void _addLogLineToList(GWEN_GUI *gui, const char *s)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  GWEN_StringList_AppendString(xgui->logStrings, s, 0, 0);
  if (xgui->logLastX>0 && GWEN_StringList_Count(xgui->logStrings)>xgui->logLastX)
    GWEN_StringList_RemoveFirstString(xgui->logStrings);
}



void _addLogLineToFile(GWEN_GUI *gui, const char *s)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  if (xgui->logFile) {
    GWEN_FSLOCK *lck;
    GWEN_FSLOCK_RESULT res;
    FILE *f;

    if (GWEN_Directory_GetPath(xgui->logFile, GWEN_PATH_FLAGS_VARIABLE)) {
      xgui->logHookIsActive=0;
      fprintf(stderr, "Unable to get logfile, turning off logging.\n");
      return;
    }

    lck=GWEN_FSLock_new(xgui->logFile, GWEN_FSLock_TypeFile);
    res=GWEN_FSLock_Lock(lck, 30, 0);
    if (res!=GWEN_FSLock_ResultOk) {
      xgui->logHookIsActive=0;
      fprintf(stderr, "Unable to lock logfile, turning off logging.\n");
      GWEN_FSLock_free(lck);
      return;
    }

    f=fopen(xgui->logFile, "a");
    if (f==NULL) {
      fprintf(stderr, "Unable to open logfile, turning off logging.\n");
      xgui->logHookIsActive=0;
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
      return;
    }

    /* write log message */
    fprintf(f, "%s", s);

    /* close log file */
    if (fclose(f)) {
      fprintf(stderr, "Unable to close logfile, turning off logging.\n");
      xgui->logHookIsActive=0;
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
      return;
    }

    /* unlock log file */
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
  }
  else {
    fprintf(stderr, "%s\n", s);
  }
}



void _writeLinesToFile(GWEN_GUI *gui)
{
  GWEN_GUI_TEST *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_TEST, gui);
  assert(xgui);

  if (GWEN_StringList_Count(xgui->logStrings)) {
    if (xgui->logFile) {
      GWEN_FSLOCK *lck;
      GWEN_FSLOCK_RESULT res;
      FILE *f;
      GWEN_STRINGLISTENTRY *se;
  
      if (GWEN_Directory_GetPath(xgui->logFile, GWEN_PATH_FLAGS_VARIABLE)) {
        xgui->logHookIsActive=0;
        fprintf(stderr, "Unable to get logfile, turning off logging.\n");
        return;
      }
  
      lck=GWEN_FSLock_new(xgui->logFile, GWEN_FSLock_TypeFile);
      res=GWEN_FSLock_Lock(lck, 30, 0);
      if (res!=GWEN_FSLock_ResultOk) {
        xgui->logHookIsActive=0;
        fprintf(stderr, "Unable to lock logfile, turning off logging.\n");
        GWEN_FSLock_free(lck);
        return;
      }
  
      f=fopen(xgui->logFile, "a");
      if (f==NULL) {
        fprintf(stderr, "Unable to open logfile, turning off logging.\n");
        xgui->logHookIsActive=0;
        GWEN_FSLock_Unlock(lck);
        GWEN_FSLock_free(lck);
        return;
      }
  
      /* write log message */
      se=GWEN_StringList_FirstEntry(xgui->logStrings);
      while(se) {
        const char *s;
  
        s=GWEN_StringListEntry_Data(se);
        if (s && *s)
          fprintf(f, "%s", s);
        se=GWEN_StringListEntry_Next(se);
      }
  
      /* close log file */
      if (fclose(f)) {
        fprintf(stderr, "Unable to close logfile, turning off logging.\n");
        xgui->logHookIsActive=0;
        GWEN_FSLock_Unlock(lck);
        GWEN_FSLock_free(lck);
        return;
      }
  
      /* unlock log file */
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    else {
      GWEN_STRINGLISTENTRY *se;
  
      /* write log message */
      se=GWEN_StringList_FirstEntry(xgui->logStrings);
      while(se) {
        const char *s;
    
        s=GWEN_StringListEntry_Data(se);
        if (s && *s)
          fprintf(stderr, "%s", s);
        se=GWEN_StringListEntry_Next(se);
      }
    }


  }
}




