/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cryptmanager_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
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

#include "globals.h"


GWEN_INHERIT(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER)



GWEN_PLUGIN_MANAGER *CON_CryptManager_new() {
  GWEN_PLUGIN_MANAGER *cm;
  CON_CRYPTMANAGER *bcm;

  cm=GWEN_CryptManager_new();
  GWEN_NEW_OBJECT(CON_CRYPTMANAGER, bcm);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm, bcm,
		       CON_CryptManager_FreeData);

  /* set virtual functions */
  GWEN_CryptManager_SetGetPinFn(cm, CON_CryptManager_GetPin);
  GWEN_CryptManager_SetBeginEnterPinFn(cm, CON_CryptManager_BeginEnterPin);
  GWEN_CryptManager_SetEndEnterPinFn(cm, CON_CryptManager_EndEnterPin);
  GWEN_CryptManager_SetInsertTokenFn(cm, CON_CryptManager_InsertToken);
  GWEN_CryptManager_SetInsertCorrectTokenFn(cm,
					    CON_CryptManager_InsertCorrectToken);
  GWEN_CryptManager_SetShowMessageFn(cm, CON_CryptManager_ShowMessage);

  return cm;
}



void CON_CryptManager_FreeData(void *bp, void *p) {
  CON_CRYPTMANAGER *bcm;

  bcm=(CON_CRYPTMANAGER *)p;
  GWEN_FREE_OBJECT(bcm);
}


char CON_CryptManager___readCharFromStdin(int waitFor) {
  int chr;
#ifdef HAVE_TERMIOS_H
  struct termios OldAttr, NewAttr;
  int AttrChanged = 0;
#endif
#ifdef HAVE_SIGNAL_H
  //sigset_t snew, sold;
#endif

  // disable canonical mode to receive a single character
#ifdef HAVE_SIGNAL_H
  //sigemptyset(&snew);
  //sigaddset(&snew, SIGINT);
  //sigaddset(&snew, SIGSTOP);
  //sigprocmask(SIG_BLOCK, &snew, &sold);
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
          chr==CON_CRYPTMANAGER_CHAR_ABORT ||
          chr==CON_CRYPTMANAGER_CHAR_ENTER ||
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

#ifdef HAVE_SIGNAL_H
  //sigprocmask(SIG_BLOCK, &sold, 0);
#endif

  return chr;
}



int CON_CryptManager__input(GWEN_TYPE_UINT32 flags,
                            char *buffer,
                            int minLen,
                            int maxLen){
#ifdef HAVE_TERMIOS_H
  struct termios OldInAttr, NewInAttr;
  struct termios OldOutAttr, NewOutAttr;
  int AttrInChanged = 0;
  int AttrOutChanged = 0;
#endif
  int chr;
  unsigned int pos;
  int rv;
#ifdef HAVE_SIGNAL_H
  //sigset_t snew, sold;
#endif

  /* if possible, disable echo from stdin to stderr during password
   * entry */
#ifdef HAVE_SIGNAL_H
  //sigemptyset(&snew);
  //sigaddset(&snew, SIGINT);
  //sigaddset(&snew, SIGSTOP);
  //sigprocmask(SIG_BLOCK, &snew, &sold);
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
    if (chr==CON_CRYPTMANAGER_CHAR_DELETE) {
      if (pos) {
        pos--;
        fprintf(stderr, "%c %c", 8, 8);
      }
    }
    else if (chr==CON_CRYPTMANAGER_CHAR_ENTER) {
      if (minLen && pos<minLen) {
        /* too few characters */
        fprintf(stderr, "\007");
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
        if (chr==CON_CRYPTMANAGER_CHAR_ABORT) {
          DBG_INFO(0, "User aborted");
          rv=GWEN_ERROR_USER_ABORTED;
          break;
        }
        else {
          if ((flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_NUMERIC) &&
              !isdigit(chr)) {
            /* bad character */
            fprintf(stderr, "\007");
          }
          else {
            if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_SHOW)
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

#ifdef HAVE_SIGNAL_H
  //sigprocmask(SIG_BLOCK, &sold, 0);
#endif
  return rv;
}




int CON_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                            GWEN_CRYPTTOKEN *token,
                            GWEN_CRYPTTOKEN_PINTYPE pt,
                            GWEN_CRYPTTOKEN_PINENCODING pe,
                            GWEN_TYPE_UINT32 flags,
                            unsigned char *pwbuffer,
                            unsigned int minLength,
                            unsigned int maxLength,
                            unsigned int *pinLength) {
  CON_CRYPTMANAGER *bcm;
  int rv;
  const char *name;
  const char *numeric_warning="";
  const char *retry_warning="";
  char buffer[512];
  char lpwbuffer1[64];

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm);
  assert(bcm);

  if (pe!=GWEN_CryptToken_PinEncoding_ASCII) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Unhandled pin encoding %d", pe);
    return GWEN_ERROR_INVALID;
  }

  name=GWEN_CryptToken_GetDescriptiveName(token);
  if (!name || !*name)
    name=GWEN_CryptToken_GetTokenName(token);

  if (maxLength>=sizeof(lpwbuffer1))
    maxLength=sizeof(lpwbuffer1)-1;

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_NUMERIC) {
    numeric_warning = I18N(" You must only enter numbers, not letters.");
  }

  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_RETRY) {
    retry_warning=I18N("(correct) ");
  }

  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_CONFIRM) {
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please enter a new password for \n"
                  "  \"%s\"\n"
                  "The password must be at least %d characters long.%s"),
             name,
             minLength,
	     numeric_warning);
  }
  else {
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please enter the %spassword for \n"
                  "  \"%s\"\n"
                  "%s"),
             retry_warning,
             name,
             numeric_warning);
  }

  fprintf(stderr, "----- PIN INPUT -----\n");
  fprintf(stderr, "%s\n", buffer);

  while(1) {
    /* TRANSLATORS: The field here should be as long as the "Repeat:" field
     * to have the PIN entries begin at the same column for both the first and
     * the second pin entry. */
    fprintf(stderr, I18N("Input : "));
    rv=CON_CryptManager__input(flags, lpwbuffer1, minLength, maxLength);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return rv;
    }

    if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_CONFIRM) {
      char lpwbuffer2[64];

      memset(lpwbuffer2, 0, sizeof(lpwbuffer2));
      /* TRANSLATORS: The field here should be as long as the "Input :" field
       * to have the PIN entries begin at the same column for both the first
       * and the second pin entry. */
      fprintf(stderr, I18N("Repeat: "));
      rv=CON_CryptManager__input(flags, lpwbuffer2, minLength, maxLength);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
      if (strcmp(lpwbuffer1, lpwbuffer2)!=0) {
        fprintf(stderr,
                "Passwords are not equal, please try again\n");
      }
      else {
        memset(lpwbuffer2, 0, sizeof(lpwbuffer2));
        break;
      }
    }
    else
      break;
  } /* while */

  strcpy(pwbuffer, lpwbuffer1);
  memset(lpwbuffer1, 0, sizeof(lpwbuffer1));
  *pinLength=strlen(pwbuffer);
  return 0;
}




int CON_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *cm,
				  GWEN_CRYPTTOKEN *token,
				  GWEN_CRYPTTOKEN_PINTYPE pt) {
  CON_CRYPTMANAGER *bcm;
  char buffer[512];

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm);
  assert(bcm);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  snprintf(buffer, sizeof(buffer)-1,
           I18N("Please enter your PIN into the card reader."));
  fprintf(stderr, "----- SECURE PIN INPUT -----\n");
  fprintf(stderr, "%s\n", buffer);

  return 0;
}



int CON_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token,
                                GWEN_CRYPTTOKEN_PINTYPE pt,
				int ok) {
  CON_CRYPTMANAGER *bcm;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm);
  assert(bcm);

  return 0;
}



int CON_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *cm,
				GWEN_CRYPTTOKEN *token) {
  CON_CRYPTMANAGER *bcm;
  char buffer[512];
  char c;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm);
  assert(bcm);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  if (GWEN_CryptToken_GetDeviceType(token)==GWEN_CryptToken_Device_File)
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please insert the security disc\nfor %s"),
             GWEN_CryptToken_GetDescriptiveName(token));
  else
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please insert the chip card\nfor %s"),
             GWEN_CryptToken_GetDescriptiveName(token));

  fprintf(stderr, "----- INSERT MEDIUM -----\n");
  fprintf(stderr, "%s\n", buffer);

  /* Wait for input */
  c=CON_CryptManager___readCharFromStdin(0);
  if (c==CON_CRYPTMANAGER_CHAR_ABORT)
    return GWEN_ERROR_USER_ABORTED;

  return 0;
}



int CON_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *cm,
				       GWEN_CRYPTTOKEN *token) {
  CON_CRYPTMANAGER *bcm;
  char buffer[512];
  char c;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm);
  assert(bcm);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  if (GWEN_CryptToken_GetDeviceType(token)==GWEN_CryptToken_Device_File)
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please insert the correct security disc\nfor %s"),
             GWEN_CryptToken_GetDescriptiveName(token));
  else
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Please insert the correct chip card\nfor %s"),
             GWEN_CryptToken_GetDescriptiveName(token));

  fprintf(stderr, "----- INSERT CORRECT MEDIUM -----\n");
  fprintf(stderr, "%s\n", buffer);

  /* Wait for input */
  c=CON_CryptManager___readCharFromStdin(0);
  if (c==CON_CRYPTMANAGER_CHAR_ABORT)
    return GWEN_ERROR_USER_ABORTED;

  return 0;
}



int CON_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *cm,
				GWEN_CRYPTTOKEN *token,
				const char *title,
				const char *msg) {
  CON_CRYPTMANAGER *bcm;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, CON_CRYPTMANAGER, cm);
  assert(bcm);

  return 0;
}















