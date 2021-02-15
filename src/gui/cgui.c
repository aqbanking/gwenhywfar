/***************************************************************************
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002-2017 by Martin Preuss
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

#define DISABLE_DEBUGLOG


#include "cgui_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/text.h>


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_ICONV_H
# include <iconv.h>
#endif
#ifndef ICONV_CONST
# define ICONV_CONST
#endif



GWEN_INHERIT(GWEN_GUI, GWEN_GUI_CGUI)




GWEN_GUI *GWEN_Gui_CGui_new(void)
{
  GWEN_GUI *gui;
  GWEN_GUI_CGUI *cgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(GWEN_GUI_CGUI, cgui);
  cgui->progressList=GWEN_Gui_CProgress_List_new();
  GWEN_INHERIT_SETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui, cgui,
                       GWEN_Gui_CGui_FreeData);

  GWEN_Gui_SetMessageBoxFn(gui, GWEN_Gui_CGui_MessageBox);
  GWEN_Gui_SetInputBoxFn(gui, GWEN_Gui_CGui_InputBox);
  GWEN_Gui_SetShowBoxFn(gui, GWEN_Gui_CGui_ShowBox);
  GWEN_Gui_SetHideBoxFn(gui, GWEN_Gui_CGui_HideBox);
  GWEN_Gui_SetProgressStartFn(gui, GWEN_Gui_CGui_ProgressStart);
  GWEN_Gui_SetProgressAdvanceFn(gui, GWEN_Gui_CGui_ProgressAdvance);
  GWEN_Gui_SetProgressSetTotalFn(gui, GWEN_Gui_CGui_ProgressSetTotal);
  GWEN_Gui_SetProgressLogFn(gui, GWEN_Gui_CGui_ProgressLog);
  GWEN_Gui_SetProgressEndFn(gui, GWEN_Gui_CGui_ProgressEnd);

  return gui;
}



void GWENHYWFAR_CB GWEN_Gui_CGui_FreeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_GUI_CGUI *cgui;

  cgui=(GWEN_GUI_CGUI *)p;
  GWEN_Gui_CProgress_List_free(cgui->progressList);
  GWEN_FREE_OBJECT(cgui);
}



char GWEN_Gui_CGui__readCharFromStdin(int waitFor)
{
  int chr;
#ifdef HAVE_TERMIOS_H
  struct termios OldAttr, NewAttr;
  int AttrChanged = 0;
#endif
#if HAVE_DECL_SIGPROCMASK
  sigset_t snew, sold;
#endif

  // disable canonical mode to receive a single character
#if HAVE_DECL_SIGPROCMASK
  sigemptyset(&snew);
  sigaddset(&snew, SIGINT);
  sigaddset(&snew, SIGSTOP);
  sigprocmask(SIG_BLOCK, &snew, &sold);
#endif
#ifdef HAVE_TERMIOS_H
  if (0 == tcgetattr(fileno(stdin), &OldAttr)) {
    NewAttr = OldAttr;
    NewAttr.c_lflag &= ~ICANON;
    NewAttr.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSAFLUSH, &NewAttr);
    AttrChanged = !0;
  }
#endif

  for (;;) {
    chr=getchar();
    if (waitFor) {
      if (chr==-1 ||
          chr==GWEN_GUI_CGUI_CHAR_ABORT ||
          chr==GWEN_GUI_CGUI_CHAR_ENTER ||
          chr==waitFor)
        break;
    }
    else
      break;
  }

#ifdef HAVE_TERMIOS_H
  /* re-enable canonical mode (if previously disabled) */
  if (AttrChanged)
    tcsetattr(fileno(stdin), TCSADRAIN, &OldAttr);
#endif

#if HAVE_DECL_SIGPROCMASK
  sigprocmask(SIG_BLOCK, &sold, 0);
#endif

  return chr;
}



int GWEN_Gui_CGui__input(GWEN_UNUSED GWEN_GUI *gui,
                         uint32_t flags,
                         char *buffer,
                         int minLen,
                         int maxLen,
                         uint32_t guiid)
{
#ifdef HAVE_TERMIOS_H
  struct termios OldInAttr, NewInAttr;
  struct termios OldOutAttr, NewOutAttr;
  int AttrInChanged = 0;
  int AttrOutChanged = 0;
#endif
  int chr;
  unsigned int pos;
  char *pOutbuf;
  int rv;
#if HAVE_DECL_SIGPROCMASK
  sigset_t snew, sold;
#endif
#ifdef HAVE_ICONV
#define INBUFSIZE 6
  char inbuf[INBUFSIZE];
  iconv_t ic;
  size_t inLeft;
  size_t outLeft;
  size_t done;
  ICONV_CONST char *pInbuf;
  const char *wantedCharSet;
  char *nextchr;

  wantedCharSet=GWEN_Gui_GetCharSet(gui);
  if (!wantedCharSet)
    wantedCharSet="UTF-8";
  ic=iconv_open("UTF-8", wantedCharSet);
  if (ic==(iconv_t)-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Cannot convert from \"%s\" to \"UTF-8\", %s",
              wantedCharSet, strerror(errno));
    return GWEN_ERROR_GENERIC;
  }

  pInbuf=inbuf;
  outLeft=maxLen;
#endif

  /* if possible, disable echo from stdin to stdout during password
   * entry */
#if HAVE_DECL_SIGPROCMASK
  sigemptyset(&snew);
  sigaddset(&snew, SIGINT);
  sigaddset(&snew, SIGSTOP);
  sigprocmask(SIG_BLOCK, &snew, &sold);
#endif

#ifdef HAVE_TERMIOS_H
  if (0 == tcgetattr(fileno(stdin), &OldInAttr)) {
    NewInAttr = OldInAttr;
    NewInAttr.c_lflag &= ~ECHO;
    NewInAttr.c_lflag &= ~ICANON;
    tcsetattr(fileno(stdin), TCSAFLUSH, &NewInAttr);
    AttrInChanged = !0;
  }
  if (0 == tcgetattr(fileno(stdout), &OldOutAttr)) {
    NewOutAttr = OldOutAttr;
    NewOutAttr.c_lflag &= ~ICANON;
    tcsetattr(fileno(stdout), TCSAFLUSH, &NewOutAttr);
    AttrOutChanged = !0;
  }
#endif

  pos=0;
  pOutbuf=buffer;
  for (;;) {
#ifdef HAVE_ICONV
    nextchr=inbuf;
    pInbuf=inbuf;
    inLeft=0;
    outLeft=maxLen-pos;
    do {
      chr=getchar();
      if (chr==EOF)
        break;
      *nextchr++=chr;
      inLeft++;
      done=iconv(ic, &pInbuf, &inLeft, &pOutbuf, &outLeft);
    }
    while (done==(size_t)-1 && errno==EINVAL &&
           nextchr-inbuf<INBUFSIZE);

    if (chr==EOF) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected EOF while reading from stdin");
      rv=GWEN_ERROR_GENERIC;
      break;
    }
    else if (done==(size_t)-1) {
      if (errno==E2BIG || errno==EILSEQ) {
        GWEN_Gui_StdPrintf(gui, stdout, "\007");
        continue;
      }
      DBG_ERROR(GWEN_LOGDOMAIN, "Unrecoverable error in conversion: %s",
                strerror(errno));
      rv=GWEN_ERROR_GENERIC;
      break;
    }
#else /* HAVE_ICONV */
    chr=getchar();
    if (chr!=EOF)
      *pOutbuf++=chr;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected EOF while reading from stdin");
      rv=GWEN_ERROR_GENERIC;
      break;
    }
#endif /* HAVE_ICONV */

    if (chr==GWEN_GUI_CGUI_CHAR_DELETE) {
      if (pos) {
        /* Look for the start of the previous UTF-8 character */
        do
          pos--;
        while ((buffer[pos]&0xC0)==0x80 && pos);
        GWEN_Gui_StdPrintf(gui, stdout, "%c %c", 8, 8);
      }
      pOutbuf=buffer+pos;
    }
    else if (chr==GWEN_GUI_CGUI_CHAR_ENTER) {
      if (minLen && pos<minLen) {
        if (pos==0 && (flags & GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT)) {
          rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO |
                                 GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
                                 GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                                 I18N("Empty Input"),
                                 I18N("Your input was empty.\n"
                                      "Do you want to use the default?"),
                                 I18N("Yes"),
                                 I18N("No"),
                                 I18N("Abort"), guiid);
          if (rv==1) {
            rv=GWEN_ERROR_DEFAULT_VALUE;
            break;
          }
          else {
            rv=GWEN_ERROR_USER_ABORTED;
            break;
          }
        }
        else {
          /* too few characters */
          GWEN_Gui_StdPrintf(gui, stdout, "\007");
          pOutbuf=buffer+pos;
        }
      }
      else {
        GWEN_Gui_StdPrintf(gui, stdout, "\n");
        buffer[pos]=0;
        rv=0;
        break;
      }
    }
    else if (chr==GWEN_GUI_CGUI_CHAR_ABORT) {
      DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
      rv=GWEN_ERROR_USER_ABORTED;
      break;
    }
    else if (pOutbuf-buffer<maxLen) {
      if ((flags & GWEN_GUI_INPUT_FLAGS_NUMERIC) &&
          !isdigit(chr)) {
        /* bad character */
        GWEN_Gui_StdPrintf(gui, stdout, "\007");
        pOutbuf=buffer+pos;
      }
      else {
        if (flags & GWEN_GUI_INPUT_FLAGS_SHOW) {
          *pOutbuf=0;
          GWEN_Gui_StdPrintf(gui, stdout, "%s", buffer+pos);
        }
        else
#ifndef HAVE_ICONV
          /* Do not print stars for continuation bytes */
          if ((chr&0xC0)!=0x80)
#endif
            GWEN_Gui_StdPrintf(gui, stdout, "*");
        pos=pOutbuf-buffer;
      }
    }
    else {
      /* buffer full */
      GWEN_Gui_StdPrintf(gui, stdout, "\007");
      pOutbuf=buffer+pos;
    }
  } /* for */

#ifdef HAVE_TERMIOS_H
  /* re-enable echo (if previously disabled) */
  if (AttrOutChanged)
    tcsetattr(fileno(stdout), TCSADRAIN, &OldOutAttr);
  if (AttrInChanged)
    tcsetattr(fileno(stdin), TCSADRAIN, &OldInAttr);
#endif

#if HAVE_DECL_SIGPROCMASK
  sigprocmask(SIG_BLOCK, &sold, 0);
#endif
#ifdef HAVE_ICONV
  iconv_close(ic);
#endif
  return rv;
}



int GWENHYWFAR_CB GWEN_Gui_CGui_MessageBox(GWEN_GUI *gui,
                             uint32_t flags,
                             const char *title,
                             const char *text,
                             const char *b1,
                             const char *b2,
                             const char *b3,
                             GWEN_UNUSED uint32_t guiid)
{
  GWEN_BUFFER *tbuf;
  int c;

  assert(gui);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Gui_GetRawText(gui, text, tbuf);

  if (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_NONINTERACTIVE) {
    if (GWEN_GUI_MSG_FLAGS_SEVERITY_IS_DANGEROUS(flags)) {
      GWEN_Gui_StdPrintf(gui, stdout,
                         "Got the following dangerous message:\n%s\n",
                         GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Auto-answering the following message with %d:\n%s",
               GWEN_GUI_MSG_FLAGS_CONFIRM_BUTTON(flags),
               GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      return GWEN_GUI_MSG_FLAGS_CONFIRM_BUTTON(flags);
    }
  }

  GWEN_Gui_StdPrintf(gui, stdout, "===== %s =====\n", title);
  GWEN_Gui_StdPrintf(gui, stdout, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  if (b1) {
    GWEN_Gui_StdPrintf(gui, stdout, "(1) %s", b1);
    if (b2) {
      GWEN_Gui_StdPrintf(gui, stdout, "  (2) %s", b2);
      if (b3) {
        GWEN_Gui_StdPrintf(gui, stdout, "  (3) %s", b3);
      }
    }
    GWEN_Gui_StdPrintf(gui, stdout, "\n");
  }
  GWEN_Gui_StdPrintf(gui, stdout, "Please enter your choice: ");
  for (;;) {
    c=GWEN_Gui_CGui__readCharFromStdin(0);
    if (c==EOF) {
      GWEN_Gui_StdPrintf(gui, stdout, "Aborted.\n");
      return GWEN_ERROR_USER_ABORTED;
    }
    if (!b1 && c==13)
      return 0;
    if (c=='1' && b1) {
      GWEN_Gui_StdPrintf(gui, stdout, "1\n");
      return 1;
    }
    else if (c=='2' && b2) {
      GWEN_Gui_StdPrintf(gui, stdout, "2\n");
      return 2;
    }
    else if (c=='3' && b3) {
      GWEN_Gui_StdPrintf(gui, stdout, "3\n");
      return 3;
    }
    else {
      GWEN_Gui_StdPrintf(gui, stdout, "%c", 7);
    }
  } /* for */

}



int GWENHYWFAR_CB GWEN_Gui_CGui_InputBox(GWEN_GUI *gui,
                           uint32_t flags,
                           const char *title,
                           const char *text,
                           char *buffer,
                           int minLen,
                           int maxLen,
                           uint32_t guiid)
{
  int rv;
  GWEN_BUFFER *tbuf;

  assert(gui);

  if ((GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_NONINTERACTIVE) &&
      (flags & GWEN_GUI_INPUT_FLAGS_TAN)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No TAN input in non-interactive mode");
    return GWEN_ERROR_USER_ABORTED;
  }


  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Gui_GetRawText(gui, text, tbuf);

  GWEN_Gui_StdPrintf(gui, stdout, "===== %s =====\n", title);
  GWEN_Gui_StdPrintf(gui, stdout, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  if (flags & GWEN_GUI_INPUT_FLAGS_CONFIRM) {
    char *lbuffer=0;

    lbuffer=(char *)malloc(maxLen);
    if (!lbuffer) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Not enough memory for %d bytes", maxLen);
      return GWEN_ERROR_INVALID;
    }
    for (;;) {
      GWEN_Gui_StdPrintf(gui, stdout, "Input: ");
      rv=GWEN_Gui_CGui__input(gui, flags, lbuffer, minLen, maxLen, guiid);
      if (rv) {
        free(lbuffer);
        return rv;
      }

      GWEN_Gui_StdPrintf(gui, stdout, "Again: ");
      rv=GWEN_Gui_CGui__input(gui, flags, buffer, minLen, maxLen, guiid);
      if (rv) {
        free(lbuffer);
        return rv;
      }
      if (strcmp(lbuffer, buffer)!=0) {
        GWEN_Gui_StdPrintf(gui, stdout,
                           "ERROR: Entries do not match, please try (again or abort)\n");
      }
      else {
        rv=0;
        break;
      }

    } /* for */
    free(lbuffer);
  }
  else {
    GWEN_Gui_StdPrintf(gui, stdout, "Input: ");
    rv=GWEN_Gui_CGui__input(gui, flags, buffer, minLen, maxLen, guiid);
  }

  if ((rv==0) && (GWEN_Gui_GetFlags(gui) & GWEN_GUI_FLAGS_PERMPASSWORDS)) {
    /* if the user allows it (by setting flag GWEN_GUI_FLAGS_PERMPASSWORDS)
     * return 1, meaning the input may be stored in a permanent password store */
    return 1;
  }

  return rv;
}



uint32_t GWENHYWFAR_CB GWEN_Gui_CGui_ShowBox(GWEN_GUI *gui,
                               GWEN_UNUSED uint32_t flags,
                               const char *title,
                               const char *text,
                               GWEN_UNUSED uint32_t guiid)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_BUFFER *tbuf;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Gui_GetRawText(gui, text, tbuf);

  GWEN_Gui_StdPrintf(gui, stdout, "===== %s =====\n", title);
  GWEN_Gui_StdPrintf(gui, stdout, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  return ++(cgui->nextBoxId);
}



void GWENHYWFAR_CB GWEN_Gui_CGui_HideBox(GWEN_GUI *gui, GWEN_UNUSED uint32_t id)
{
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  /* nothing to do right now */
}



uint32_t GWENHYWFAR_CB GWEN_Gui_CGui_ProgressStart(GWEN_GUI *gui,
                                     uint32_t progressFlags,
                                     const char *title,
                                     const char *text,
                                     uint64_t total,
                                     GWEN_UNUSED uint32_t guiid)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CProgress_new(gui,
                            ++(cgui->nextProgressId),
                            progressFlags,
                            title,
                            text,
                            total);
  GWEN_Gui_CProgress_List_Insert(cp, cgui->progressList);
  return GWEN_Gui_CProgress_GetId(cp);
}



GWEN_GUI_CPROGRESS *GWEN_Gui_CGui__findProgress(GWEN_GUI *gui, uint32_t id)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CProgress_List_First(cgui->progressList);
  if (id==0)
    return cp;
  while (cp) {
    if (GWEN_Gui_CProgress_GetId(cp)==id)
      break;
    cp=GWEN_Gui_CProgress_List_Next(cp);
  } /* while */

  return cp;
}



int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressAdvance(GWEN_GUI *gui,
                                  uint32_t id,
                                  uint64_t progress)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CGui__findProgress(gui, id);
  if (!cp) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Progress object %u not found", id);
    return 0;
  }
  else {
    return GWEN_Gui_CProgress_Advance(cp, progress);
  }
}



int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressSetTotal(GWEN_GUI *gui, uint32_t id, uint64_t total)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CGui__findProgress(gui, id);
  if (!cp) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Progress object %u not found", id);
  }
  else
    GWEN_Gui_CProgress_SetTotal(cp, total);
  return 0;
}



int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressLog(GWEN_GUI *gui,
                              uint32_t id,
                              GWEN_LOGGER_LEVEL level,
                              const char *text)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CGui__findProgress(gui, id);
  if (!cp) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Progress object %u not found", id);
    return 0;
  }
  else {
    return GWEN_Gui_CProgress_Log(cp, level, text);
  }
}



int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressEnd(GWEN_GUI *gui, uint32_t id)
{
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CGui__findProgress(gui, id);
  if (!cp) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Progress object %u not found", id);
    return 0;
  }
  else {
    int rv;

    rv=GWEN_Gui_CProgress_End(cp);
    GWEN_Gui_CProgress_List_Del(cp);
    GWEN_Gui_CProgress_free(cp);
    return rv;
  }
}



