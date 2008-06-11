/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: error.h 1104 2007-01-03 09:21:32Z martin $
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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
#else
# define ICONV_CONST
#endif


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



GWEN_INHERIT(GWEN_GUI, GWEN_GUI_CGUI)




GWEN_GUI *GWEN_Gui_CGui_new() {
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
  GWEN_Gui_SetProgressLogFn(gui, GWEN_Gui_CGui_ProgressLog);
  GWEN_Gui_SetProgressEndFn(gui, GWEN_Gui_CGui_ProgressEnd);
  GWEN_Gui_SetSetPasswordStatusFn(gui, GWEN_Gui_CGui_SetPasswordStatus);
  GWEN_Gui_SetGetPasswordFn(gui, GWEN_Gui_CGui_GetPassword);

  cgui->checkCertFn=GWEN_Gui_SetCheckCertFn(gui, GWEN_Gui_CGui_CheckCert);

  cgui->dbPasswords=GWEN_DB_Group_new("passwords");
  cgui->dbCerts=GWEN_DB_Group_new("certs");
  cgui->badPasswords=GWEN_StringList_new();

  return gui;
}



void GWENHYWFAR_CB GWEN_Gui_CGui_FreeData(void *bp, void *p) {
  GWEN_GUI_CGUI *cgui;

  cgui=(GWEN_GUI_CGUI*)p;
  GWEN_Gui_CProgress_List_free(cgui->progressList);
  free(cgui->charSet);
  GWEN_StringList_free(cgui->badPasswords);
  GWEN_DB_Group_free(cgui->dbCerts);
  GWEN_DB_Group_free(cgui->dbPasswords);
  GWEN_FREE_OBJECT(cgui);
}



const char *GWEN_Gui_CGui_GetCharSet(const GWEN_GUI *gui) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  return cgui->charSet;
}



void GWEN_Gui_CGui_SetCharSet(GWEN_GUI *gui, const char *s) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  free(cgui->charSet);
  if (s)
    cgui->charSet=strdup(s);
  else
    cgui->charSet=NULL;
}



int GWEN_Gui_CGui_GetIsNonInteractive(const GWEN_GUI *gui) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  return cgui->nonInteractive;
}



void GWEN_Gui_CGui_SetIsNonInteractive(GWEN_GUI *gui, int i) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cgui->nonInteractive=i;
}



int GWEN_Gui_CGui__ConvertFromUtf8(GWEN_GUI *gui,
				   const char *text,
				   int len,
				   GWEN_BUFFER *tbuf){
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  assert(len);

  if (cgui->charSet) {
    if (strcasecmp(cgui->charSet, "utf-8")!=0) {
#ifndef HAVE_ICONV
      DBG_INFO(GWEN_LOGDOMAIN,
	       "iconv not available, can not convert to \"%s\"",
	       cgui->charSet);
#else
      iconv_t ic;

      ic=iconv_open(cgui->charSet, "UTF-8");
      if (ic==((iconv_t)-1)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Charset \"%s\" not available",
		  cgui->charSet);
      }
      else {
        char *outbuf;
        char *pOutbuf;
	/* Some systems have iconv in libc, some have it in libiconv
	   (OSF/1 and those with the standalone portable GNU libiconv
	   installed). Check which one is available. The define
	   ICONV_CONST will be "" or "const" accordingly. */
	ICONV_CONST char *pInbuf;
        size_t inLeft;
        size_t outLeft;
        size_t done;
        size_t space;

        /* convert */
        pInbuf=(char*)text;

        outLeft=len*2;
        space=outLeft;
        outbuf=(char*)malloc(outLeft);
        assert(outbuf);

        inLeft=len;
        pInbuf=(char*)text;
        pOutbuf=outbuf;
        done=iconv(ic, &pInbuf, &inLeft, &pOutbuf, &outLeft);
        if (done==(size_t)-1) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Error in conversion: %s (%d)",
                    strerror(errno), errno);
          free(outbuf);
          iconv_close(ic);
	  return GWEN_ERROR_GENERIC;
        }

        GWEN_Buffer_AppendBytes(tbuf, outbuf, space-outLeft);
	free(outbuf);
	DBG_DEBUG(GWEN_LOGDOMAIN, "Conversion done.");
	iconv_close(ic);
	return 0;
      }
#endif
    }
  }

  GWEN_Buffer_AppendBytes(tbuf, text, len);
  return 0;
}



void GWEN_Gui_CGui_GetRawText(GWEN_GUI *gui,
			      const char *text,
			      GWEN_BUFFER *tbuf) {
  const char *p;
  int rv;

  assert(text);
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
            break;
          }
        }
      }
    }
    p++;
  } /* while */

  if (p)
    rv=GWEN_Gui_CGui__ConvertFromUtf8(gui, text, (p-text), tbuf);
  else
    rv=GWEN_Gui_CGui__ConvertFromUtf8(gui, text, strlen(text), tbuf);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error converting text");
    GWEN_Buffer_Reset(tbuf);
    if (p)
      GWEN_Buffer_AppendBytes(tbuf, text, (p-text));
    else
      GWEN_Buffer_AppendString(tbuf, text);
  }
}



char GWEN_Gui_CGui__readCharFromStdin(int waitFor) {
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
  if (0 == tcgetattr (fileno (stdin), &OldAttr)){
    NewAttr = OldAttr;
    NewAttr.c_lflag &= ~ICANON;
    NewAttr.c_lflag &= ~ECHO;
    tcsetattr (fileno (stdin), TCSAFLUSH, &NewAttr);
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
    tcsetattr (fileno (stdin), TCSADRAIN, &OldAttr);
#endif

#if HAVE_DECL_SIGPROCMASK
  sigprocmask(SIG_BLOCK, &sold, 0);
#endif

  return chr;
}



int GWEN_Gui_CGui__input(GWEN_GUI *gui,
			 uint32_t flags,
			 char *buffer,
			 int minLen,
			 int maxLen,
			 uint32_t guiid){
#ifdef HAVE_TERMIOS_H
  struct termios OldInAttr, NewInAttr;
  struct termios OldOutAttr, NewOutAttr;
  int AttrInChanged = 0;
  int AttrOutChanged = 0;
#endif
  int chr;
  unsigned int pos;
  int rv;
#if HAVE_DECL_SIGPROCMASK
  sigset_t snew, sold;
#endif

  /* if possible, disable echo from stdin to stderr during password
   * entry */
#if HAVE_DECL_SIGPROCMASK
  sigemptyset(&snew);
  sigaddset(&snew, SIGINT);
  sigaddset(&snew, SIGSTOP);
  sigprocmask(SIG_BLOCK, &snew, &sold);
#endif

#ifdef HAVE_TERMIOS_H
  if (0 == tcgetattr (fileno (stdin), &OldInAttr)){
    NewInAttr = OldInAttr;
    NewInAttr.c_lflag &= ~ECHO;
    NewInAttr.c_lflag &= ~ICANON;
    tcsetattr (fileno (stdin), TCSAFLUSH, &NewInAttr);
    AttrInChanged = !0;
  }
  if (0 == tcgetattr (fileno (stderr), &OldOutAttr)){
    NewOutAttr = OldOutAttr;
    NewOutAttr.c_lflag &= ~ICANON;
    tcsetattr (fileno (stderr), TCSAFLUSH, &NewOutAttr);
    AttrOutChanged = !0;
  }
#endif

  pos=0;
  rv=0;
  for (;;) {
    chr=getchar();
    if (chr==GWEN_GUI_CGUI_CHAR_DELETE) {
      if (pos) {
        pos--;
        fprintf(stderr, "%c %c", 8, 8);
      }
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
	  fprintf(stderr, "\007");
	}
      }
      else {
        fprintf(stderr, "\n");
        buffer[pos]=0;
        rv=0;
        break;
      }
    }
    else {
      if (pos<maxLen) {
        if (chr==GWEN_GUI_CGUI_CHAR_ABORT) {
          DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
          rv=GWEN_ERROR_USER_ABORTED;
          break;
        }
        else {
          if ((flags & GWEN_GUI_INPUT_FLAGS_NUMERIC) &&
              !isdigit(chr)) {
            /* bad character */
            fprintf(stderr, "\007");
          }
          else {
            if (flags & GWEN_GUI_INPUT_FLAGS_SHOW)
              fprintf(stderr, "%c", chr);
            else
              fprintf(stderr, "*");
            buffer[pos++]=chr;
            buffer[pos]=0;
          }
        }
      }
      else {
        /* buffer full */
        fprintf(stderr, "\007");
      }
    }
  } /* for */

#ifdef HAVE_TERMIOS_H
  /* re-enable echo (if previously disabled) */
  if (AttrOutChanged)
    tcsetattr (fileno (stderr), TCSADRAIN, &OldOutAttr);
  if (AttrInChanged)
    tcsetattr (fileno (stdin), TCSADRAIN, &OldInAttr);
#endif

#if HAVE_DECL_SIGPROCMASK
  sigprocmask(SIG_BLOCK, &sold, 0);
#endif
  return rv;
}



int GWEN_Gui_CGui_MessageBox(GWEN_GUI *gui,
			     uint32_t flags,
			     const char *title,
			     const char *text,
			     const char *b1,
			     const char *b2,
			     const char *b3,
			     uint32_t guiid) {
  GWEN_GUI_CGUI *cgui;
  GWEN_BUFFER *tbuf;
  int c;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Gui_CGui_GetRawText(gui, text, tbuf);

  if (cgui->nonInteractive) {
    if (GWEN_GUI_MSG_FLAGS_SEVERITY_IS_DANGEROUS(flags)) {
      fprintf(stderr,
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

  fprintf(stderr, "===== %s =====\n", title);
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  if (b1) {
    fprintf(stderr, "(1) %s", b1);
    if (b2) {
      fprintf(stderr, "  (2) %s", b2);
      if (b3) {
        fprintf(stderr, "  (3) %s", b3);
      }
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "Please enter your choice: ");
  for(;;) {
    c=GWEN_Gui_CGui__readCharFromStdin(0);
    if (c==EOF) {
      fprintf(stderr, "Aborted.\n");
      return GWEN_ERROR_USER_ABORTED;
    }
    if (!b1 && c==13)
      return 0;
    if (c=='1' && b1) {
      fprintf(stderr, "1\n");
      return 1;
    }
    else if (c=='2' && b2) {
      fprintf(stderr, "2\n");
      return 2;
    }
    else if (c=='3' && b3) {
      fprintf(stderr, "3\n");
      return 3;
    }
    else {
      fprintf(stderr, "%c", 7);
    }
  } /* for */

}



int GWEN_Gui_CGui_InputBox(GWEN_GUI *gui,
			   uint32_t flags,
			   const char *title,
			   const char *text,
			   char *buffer,
			   int minLen,
			   int maxLen,
			   uint32_t guiid) {
  int rv;
  GWEN_BUFFER *tbuf;

  assert(gui);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Gui_CGui_GetRawText(gui, text, tbuf);

  fprintf(stderr, "===== %s =====\n", title);
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  if (flags & GWEN_GUI_INPUT_FLAGS_CONFIRM) {
    for (;;) {
      char *lbuffer=0;

      lbuffer=(char*)malloc(maxLen);
      if (!lbuffer) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Not enough memory for %d bytes", maxLen);
        return GWEN_ERROR_INVALID;
      }
      fprintf(stderr, "Input: ");
      rv=GWEN_Gui_CGui__input(gui, flags, lbuffer, minLen, maxLen, guiid);
      if (rv) {
        free(lbuffer);
        return rv;
      }

      fprintf(stderr, "Again: ");
      rv=GWEN_Gui_CGui__input(gui, flags, buffer, minLen, maxLen, guiid);
      if (rv) {
        free(lbuffer);
        return rv;
      }
      if (strcmp(lbuffer, buffer)!=0) {
        fprintf(stderr,
                "ERROR: Entries do not match, please try (again or abort)\n");
      }
      else {
        rv=0;
        break;
      }

    } /* for */
  }
  else {
    fprintf(stderr, "Input: ");
    rv=GWEN_Gui_CGui__input(gui, flags, buffer, minLen, maxLen, guiid);
  }

  return rv;
}



uint32_t GWEN_Gui_CGui_ShowBox(GWEN_GUI *gui,
			       uint32_t flags,
			       const char *title,
			       const char *text,
			       uint32_t guiid) {
  GWEN_GUI_CGUI *cgui;
  GWEN_BUFFER *tbuf;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Gui_CGui_GetRawText(gui, text, tbuf);

  fprintf(stderr, "----- %s -----\n", title);
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  return ++(cgui->nextBoxId);
}



void GWEN_Gui_CGui_HideBox(GWEN_GUI *gui, uint32_t id) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  /* nothing to do right now */
}



uint32_t GWEN_Gui_CGui_ProgressStart(GWEN_GUI *gui,
				     uint32_t progressFlags,
				     const char *title,
				     const char *text,
				     uint64_t total,
				     uint32_t guiid) {
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



GWEN_GUI_CPROGRESS *GWEN_Gui_CGui__findProgress(GWEN_GUI *gui, uint32_t id) {
  GWEN_GUI_CGUI *cgui;
  GWEN_GUI_CPROGRESS *cp;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  cp=GWEN_Gui_CProgress_List_First(cgui->progressList);
  if (id==0)
    return cp;
  while(cp) {
    if (GWEN_Gui_CProgress_GetId(cp)==id)
      break;
    cp=GWEN_Gui_CProgress_List_Next(cp);
  } /* while */

  return cp;
}



int GWEN_Gui_CGui_ProgressAdvance(GWEN_GUI *gui,
				  uint32_t id,
				  uint64_t progress) {
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



int GWEN_Gui_CGui_ProgressLog(GWEN_GUI *gui,
			      uint32_t id,
			      GWEN_LOGGER_LEVEL level,
			      const char *text) {
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



int GWEN_Gui_CGui_ProgressEnd(GWEN_GUI *gui,uint32_t id) {
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



int GWEN_Gui_CGui_Print(GWEN_GUI *gui,
			const char *docTitle,
			const char *docType,
			const char *descr,
			const char *text,
			uint32_t guiid) {
  return GWEN_ERROR_NOT_SUPPORTED;
}



int GWEN_Gui_CGui__HashPair(const char *token,
			    const char *pin,
			    GWEN_BUFFER *buf) {
  GWEN_MDIGEST *md;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Md5_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)token, strlen(token));
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)pin, strlen(pin));
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  GWEN_Text_ToHexBuffer((const char*)GWEN_MDigest_GetDigestPtr(md),
			GWEN_MDigest_GetDigestSize(md),
			buf,
			0, 0, 0);
  GWEN_MDigest_free(md);
  return 0;
}



int GWEN_Gui_CGui_CheckCert(GWEN_GUI *gui,
			    const GWEN_SSLCERTDESCR *cd,
			    GWEN_IO_LAYER *io, uint32_t guiid) {
  GWEN_GUI_CGUI *cgui;
  const char *hash;
  const char *status;
  GWEN_BUFFER *hbuf;
  int i;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  hash=GWEN_SslCertDescr_GetFingerPrint(cd);
  status=GWEN_SslCertDescr_GetStatusText(cd);

  hbuf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Gui_CGui__HashPair(hash, status, hbuf);

  i=GWEN_DB_GetIntValue(cgui->dbCerts, GWEN_Buffer_GetStart(hbuf), 0, 1);
  if (i==0) {
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "Automatically accepting certificate [%s]",
	       hash);
    GWEN_Buffer_free(hbuf);
    return 0;
  }

  if (cgui->nonInteractive) {
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "Automatically rejecting certificate [%s] (noninteractive)",
	       hash);
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_USER_ABORTED;
  }

  if (cgui->checkCertFn) {
    i=cgui->checkCertFn(gui, cd, io, guiid);
    if (i==0) {
      GWEN_DB_SetIntValue(cgui->dbCerts, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  GWEN_Buffer_GetStart(hbuf), i);
    }
    GWEN_Buffer_free(hbuf);

    return i;
  }
  else {
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int GWEN_Gui_CGui_SetPasswordStatus(GWEN_GUI *gui,
				    const char *token,
				    const char *pin,
				    GWEN_GUI_PASSWORD_STATUS status,
				    uint32_t guiid) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  if (token==NULL && pin==NULL && status==GWEN_Gui_PasswordStatus_Remove) {
    if (cgui->persistentPasswords==0)
      GWEN_DB_ClearGroup(cgui->dbPasswords, NULL);
  }
  else {
    GWEN_BUFFER *hbuf;

    hbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Gui_CGui__HashPair(token, pin, hbuf);
    if (status==GWEN_Gui_PasswordStatus_Bad)
      GWEN_StringList_AppendString(cgui->badPasswords,
				   GWEN_Buffer_GetStart(hbuf),
				   0, 1);
    else if (status==GWEN_Gui_PasswordStatus_Ok ||
	     status==GWEN_Gui_PasswordStatus_Remove) {
      if (cgui->persistentPasswords==0)
	GWEN_StringList_RemoveString(cgui->badPasswords,
				     GWEN_Buffer_GetStart(hbuf));
    }
    GWEN_Buffer_free(hbuf);
  }

  return 0;
}



int GWEN_Gui_CGui_GetPassword(GWEN_GUI *gui,
			      uint32_t flags,
			      const char *token,
			      const char *title,
			      const char *text,
			      char *buffer,
			      int minLen,
			      int maxLen,
			      uint32_t guiid) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  if (flags & GWEN_GUI_INPUT_FLAGS_TAN) {
    return GWEN_Gui_InputBox(flags,
			     title,
			     text,
			     buffer,
			     minLen,
			     maxLen,
			     guiid);
  }
  else {
    GWEN_BUFFER *buf;
    int rv;
    const char *s;
  
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_EscapeToBufferTolerant(token, buf);

    if (!(flags & GWEN_GUI_INPUT_FLAGS_CONFIRM)) {
      s=GWEN_DB_GetCharValue(cgui->dbPasswords,
			     GWEN_Buffer_GetStart(buf),
			     0, NULL);
      if (s) {
	int i;

	i=strlen(s);
	if (i>=minLen && i<=maxLen) {
	  memmove(buffer, s, i+1);
	  GWEN_Buffer_free(buf);
	  return 0;
	}
      }
    }

    if (cgui->nonInteractive) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Password for [%s] missing in noninteractive mode, "
		"aborting", token);
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_USER_ABORTED;
    }

    for (;;) {
      rv=GWEN_Gui_InputBox(flags,
			   title,
			   text,
			   buffer,
			   minLen,
			   maxLen,
			   guiid);
      if (rv) {
	GWEN_Buffer_free(buf);
	return rv;
      }
      else {
	GWEN_BUFFER *hbuf;
        int isBad=0;
    
	hbuf=GWEN_Buffer_new(0, 64, 0, 1);
	GWEN_Gui_CGui__HashPair(token, buffer, hbuf);
	isBad=GWEN_StringList_HasString(cgui->badPasswords,
					GWEN_Buffer_GetStart(hbuf));
	if (!isBad) {
	  GWEN_Buffer_free(hbuf);
	  break;
	}
	rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			       GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			       GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
			       I18N("Enforce PIN"),
			       I18N(
				    "You entered the same PIN twice.\n"
				    "The PIN is marked as bad, do you want\n"
				    "to use it anyway?"
				    "<html>"
				    "<p>"
				    "You entered the same PIN twice."
				    "</p>"
				    "<p>"
				    "The PIN is marked as <b>bad</b>, "
				    "do you want to use it anyway?"
				    "</p>"
				    "</html>"),
			       I18N("Use my input"),
			       I18N("Re-enter"),
			       0,
			       guiid);
	if (rv==1) {
	  /* accept this input */
	  GWEN_StringList_RemoveString(cgui->badPasswords,
				       GWEN_Buffer_GetStart(hbuf));
	  GWEN_Buffer_free(hbuf);
	  break;
	}
	GWEN_Buffer_free(hbuf);
      }
    } /* for */
  
    GWEN_DB_SetCharValue(cgui->dbPasswords, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 GWEN_Buffer_GetStart(buf), buffer);
    GWEN_Buffer_free(buf);
    return 0;
  }
}



void GWEN_Gui_CGui_SetPasswordDb(GWEN_GUI *gui,
				 GWEN_DB_NODE *dbPasswords,
				 int persistent) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  GWEN_DB_Group_free(cgui->dbPasswords);
  cgui->dbPasswords=dbPasswords;
  cgui->persistentPasswords=persistent;
}



void GWEN_Gui_CGui_SetCertDb(GWEN_GUI *gui, GWEN_DB_NODE *dbCerts) {
  GWEN_GUI_CGUI *cgui;

  assert(gui);
  cgui=GWEN_INHERIT_GETDATA(GWEN_GUI, GWEN_GUI_CGUI, gui);
  assert(cgui);

  GWEN_DB_Group_free(cgui->dbCerts);
  cgui->dbCerts=dbCerts;
}





