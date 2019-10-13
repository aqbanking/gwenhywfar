

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cprogress_p.h"
#include "cgui_l.h"

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(GWEN_GUI_CPROGRESS, GWEN_Gui_CProgress)



GWEN_GUI_CPROGRESS *GWEN_Gui_CProgress_new(GWEN_GUI *gui,
                                           uint32_t id,
                                           uint32_t progressFlags,
                                           const char *title,
                                           const char *text,
                                           uint64_t total)
{
  GWEN_GUI_CPROGRESS *cp;

  GWEN_NEW_OBJECT(GWEN_GUI_CPROGRESS, cp);
  GWEN_LIST_INIT(GWEN_GUI_CPROGRESS, cp);
  cp->gui=gui;
  cp->id=id;
  cp->startTime=time(0);
  cp->flags=progressFlags;
  if (title)
    cp->title=strdup(title);
  if (text)
    cp->text=strdup(text);
  cp->total=total;
  cp->logBuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (!(cp->flags & GWEN_GUI_PROGRESS_DELAY)) {
    GWEN_Gui_StdPrintf(gui, stdout, "===== %s =====\n", title);
    cp->shown=1;
  }

  return cp;
}



void GWEN_Gui_CProgress_free(GWEN_GUI_CPROGRESS *cp)
{
  if (cp) {
    GWEN_LIST_FINI(GWEN_GUI_CPROGRESS, cp);
    GWEN_Buffer_free(cp->logBuf);
    free(cp->text);
    free(cp->title);
    GWEN_FREE_OBJECT(cp);
  }
}



GWEN_GUI *GWEN_Gui_CProgress_GetGui(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->gui;
}



uint32_t GWEN_Gui_CProgress_GetId(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->id;
}



const char *GWEN_Gui_CProgress_GetTitle(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->title;
}



const char *GWEN_Gui_CProgress_GetText(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->text;
}



uint64_t GWEN_Gui_CProgress_GetTotal(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->total;
}



void GWEN_Gui_CProgress_SetTotal(GWEN_GUI_CPROGRESS *cp, uint64_t i)
{
  assert(cp);
  cp->total=i;
}



uint64_t GWEN_Gui_CProgress_GetCurrent(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->current;
}



const char *GWEN_Gui_CProgress_GetLogBuf(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  assert(cp->logBuf);
  return GWEN_Buffer_GetStart(cp->logBuf);
}



int GWEN_Gui_CProgress_GetAborted(const GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);
  return cp->aborted;
}






int GWEN_Gui_CProgress_Advance(GWEN_GUI_CPROGRESS *cp, uint64_t progress)
{
#ifndef OS_WIN32
  int fl;
#endif

  assert(cp);
  if (!cp->shown) {
    time_t t1;

    t1=time(0);
    if (difftime(t1, cp->startTime)>GWEN_GUI_DELAY_SECS) {
      if (!(GWEN_Gui_GetFlags(cp->gui) & GWEN_GUI_FLAGS_NONINTERACTIVE))
        GWEN_Gui_StdPrintf(cp->gui, stdout, "%s: Started.\n", cp->title);
      cp->shown=1;
    }
  }

  if (progress==GWEN_GUI_PROGRESS_ONE)
    progress=cp->current+1;
  if (progress!=GWEN_GUI_PROGRESS_NONE) {
    if (progress!=cp->current) {
      if (cp->shown) {
        if (!(GWEN_Gui_GetFlags(cp->gui) & GWEN_GUI_FLAGS_NONINTERACTIVE)) {
          if (cp->total==GWEN_GUI_PROGRESS_NONE)
            GWEN_Gui_StdPrintf(cp->gui, stdout, "%s: %llu\n", cp->title,
                               (long long unsigned)progress);
          else
            GWEN_Gui_StdPrintf(cp->gui, stdout, "%s: %llu of %llu\n",
                               cp->title,
                               (long long unsigned)progress,
                               (long long unsigned)cp->total);
	}
      }
      cp->current=progress;
    }
  }
  if (cp->aborted)
    return GWEN_ERROR_USER_ABORTED;

#ifndef OS_WIN32
  if (!(GWEN_Gui_GetFlags(cp->gui) & GWEN_GUI_FLAGS_NONINTERACTIVE)) {
    /* check for abort */
    fl=fcntl(fileno(stdin), F_GETFL);
    if (fl!=-1) {
      int chr;

      /* set stdin to nonblocking */
      if (fcntl(fileno(stdin), F_SETFL, fl | O_NONBLOCK)) {
        DBG_INFO(GWEN_LOGDOMAIN, "fcntl(stdin): %s", strerror(errno));
        return 0;
      }
      /* check whether there is a character */
      chr=getchar();
      /* set blocking mode to what we found before modification */
      fcntl(fileno(stdin), F_SETFL, fl);
      if (chr==GWEN_GUI_CPROGRESS_CHAR_ABORT) {
        GWEN_Gui_StdPrintf(cp->gui, stdout, "------> ABORTED BY USER\n");
        cp->aborted=1;
        return GWEN_ERROR_USER_ABORTED;
      }
    }
  }
#endif

  return 0;
}



int GWEN_Gui_CProgress_Log(GWEN_GUI_CPROGRESS *cp,
                           GWEN_UNUSED GWEN_LOGGER_LEVEL level,
                           const char *text)
{
  assert(cp);
  assert(text);

  if (!(GWEN_Gui_GetFlags(cp->gui) & GWEN_GUI_FLAGS_NONINTERACTIVE)) {
    GWEN_BUFFER *tbuf;
    const char *t;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Gui_GetRawText(cp->gui, text, tbuf);
    t=GWEN_Buffer_GetStart(tbuf);
    if (t[GWEN_Buffer_GetUsedBytes(tbuf)-1]!='\n') {
      GWEN_Buffer_AppendByte(tbuf, '\n');
      /* Just in case the buffer has been reallocated */
      t=GWEN_Buffer_GetStart(tbuf);
    }
    GWEN_Gui_StdPrintf(cp->gui, stdout, "%s", t);

    GWEN_Buffer_AppendString(cp->logBuf, t);
    GWEN_Buffer_free(tbuf);
    tbuf=0;
    if (cp->aborted)
      return GWEN_ERROR_USER_ABORTED;
  }
  return 0;
}



int GWEN_Gui_CProgress_End(GWEN_GUI_CPROGRESS *cp)
{
  assert(cp);

  if (cp->shown) {
    if (!(GWEN_Gui_GetFlags(cp->gui) & GWEN_GUI_FLAGS_NONINTERACTIVE))
      GWEN_Gui_StdPrintf(cp->gui, stdout, "%s: Finished.\n", cp->title);
  }
  if (cp->aborted)
    return GWEN_ERROR_USER_ABORTED;

  return 0;
}




