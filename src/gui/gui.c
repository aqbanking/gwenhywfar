/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: bio_buffer.c 1102 2006-12-30 19:39:37Z martin $
 begin       : Fri Feb 07 2003
 copyright   : (C) 2003 by Martin Preuss
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


#include "gui_p.h"
#include "i18n_l.h"

#include <gwenhywfar/debug.h>



static GWEN_GUI *gwenhywfar_gui=NULL;


GWEN_INHERIT_FUNCTIONS(GWEN_GUI)



GWEN_GUI *GWEN_Gui_new() {
  GWEN_GUI *gui;

  GWEN_NEW_OBJECT(GWEN_GUI, gui);
  GWEN_INHERIT_INIT(GWEN_GUI, gui);
  gui->refCount=1;

  gui->checkCertFn=GWEN_Gui_CheckCertBuiltIn;

  return gui;
}



void GWEN_Gui_free(GWEN_GUI *gui) {
  if (gui) {
    assert(gui->refCount);
    if ((--gui->refCount)==0) {
      GWEN_INHERIT_FINI(GWEN_GUI, gui);
      GWEN_FREE_OBJECT(gui);
    }
  }
}



void GWEN_Gui_Attach(GWEN_GUI *gui) {
  assert(gui);
  assert(gui->refCount);
  gui->refCount++;
}



void GWEN_Gui_SetGui(GWEN_GUI *gui) {
  if (gui)
    GWEN_Gui_Attach(gui);
  if (gwenhywfar_gui)
    GWEN_Gui_free(gwenhywfar_gui);
  gwenhywfar_gui=gui;
}



GWEN_GUI *GWEN_Gui_GetGui() {
  return gwenhywfar_gui;
}



GWEN_GUI_MESSAGEBOX_FN GWEN_Gui_SetMessageBoxFn(GWEN_GUI *gui,
						GWEN_GUI_MESSAGEBOX_FN f){
  GWEN_GUI_MESSAGEBOX_FN of;

  assert(gui);
  of=gui->messageBoxFn;
  gui->messageBoxFn=f;
  return of;
}



GWEN_GUI_INPUTBOX_FN GWEN_Gui_SetInputBoxFn(GWEN_GUI *gui,
					    GWEN_GUI_INPUTBOX_FN f){
  GWEN_GUI_INPUTBOX_FN of;

  assert(gui);
  of=gui->inputBoxFn;
  gui->inputBoxFn=f;
  return of;
}



GWEN_GUI_SHOWBOX_FN GWEN_Gui_SetShowBoxFn(GWEN_GUI *gui,
					  GWEN_GUI_SHOWBOX_FN f){
  GWEN_GUI_SHOWBOX_FN of;

  assert(gui);
  of=gui->showBoxFn;
  gui->showBoxFn=f;
  return of;
}



GWEN_GUI_HIDEBOX_FN GWEN_Gui_SetHideBoxFn(GWEN_GUI *gui,
					  GWEN_GUI_HIDEBOX_FN f){
  GWEN_GUI_HIDEBOX_FN of;

  assert(gui);
  of=gui->hideBoxFn;
  gui->hideBoxFn=f;
  return of;
}



GWEN_GUI_PROGRESS_START_FN
GWEN_Gui_SetProgressStartFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_START_FN f){
  GWEN_GUI_PROGRESS_START_FN of;

  assert(gui);
  of=gui->progressStartFn;
  gui->progressStartFn=f;
  return of;
}



GWEN_GUI_PROGRESS_ADVANCE_FN
GWEN_Gui_SetProgressAdvanceFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_ADVANCE_FN f){
  GWEN_GUI_PROGRESS_ADVANCE_FN of;

  assert(gui);
  of=gui->progressAdvanceFn;
  gui->progressAdvanceFn=f;
  return of;
}



GWEN_GUI_PROGRESS_LOG_FN
GWEN_Gui_SetProgressLogFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_LOG_FN f){
  GWEN_GUI_PROGRESS_LOG_FN of;

  assert(gui);
  of=gui->progressLogFn;
  gui->progressLogFn=f;
  return of;
}



GWEN_GUI_PROGRESS_END_FN
GWEN_Gui_SetProgressEndFn(GWEN_GUI *gui, GWEN_GUI_PROGRESS_END_FN f){
  GWEN_GUI_PROGRESS_END_FN of;

  assert(gui);
  of=gui->progressEndFn;
  gui->progressEndFn=f;
  return of;
}



GWEN_GUI_PRINT_FN GWEN_Gui_SetPrintFn(GWEN_GUI *gui,
				      GWEN_GUI_PRINT_FN f){
  GWEN_GUI_PRINT_FN of;

  assert(gui);
  of=gui->printFn;
  gui->printFn=f;
  return of;
}



GWEN_GUI_GETPASSWORD_FN GWEN_Gui_SetGetPasswordFn(GWEN_GUI *gui,
						  GWEN_GUI_GETPASSWORD_FN f) {
  GWEN_GUI_GETPASSWORD_FN of;

  assert(gui);
  of=gui->getPasswordFn;
  gui->getPasswordFn=f;
  return of;
}



GWEN_GUI_SETPASSWORDSTATUS_FN
GWEN_Gui_SetSetPasswordStatusFn(GWEN_GUI *gui,
				GWEN_GUI_SETPASSWORDSTATUS_FN f) {
  GWEN_GUI_SETPASSWORDSTATUS_FN of;

  assert(gui);
  of=gui->setPasswordStatusFn;
  gui->setPasswordStatusFn=f;
  return of;
}



GWEN_GUI_LOG_HOOK_FN GWEN_Gui_SetLogHookFn(GWEN_GUI *gui,
					   GWEN_GUI_LOG_HOOK_FN f) {
  GWEN_GUI_LOG_HOOK_FN of;

  assert(gui);
  of=gui->logHookFn;
  gui->logHookFn=f;

  return of;
}



GWEN_GUI_WAITFORSOCKETS_FN GWEN_Gui_SetWaitForSocketsFn(GWEN_GUI *gui,
							GWEN_GUI_WAITFORSOCKETS_FN f) {
  GWEN_GUI_WAITFORSOCKETS_FN of;

  assert(gui);
  of=gui->waitForSocketsFn;
  gui->waitForSocketsFn=f;

  return of;
}



GWEN_GUI_CHECKCERT_FN GWEN_Gui_SetCheckCertFn(GWEN_GUI *gui, GWEN_GUI_CHECKCERT_FN f) {
  GWEN_GUI_CHECKCERT_FN of;

  assert(gui);
  of=gui->checkCertFn;
  gui->checkCertFn=f;

  return of;
}



GWEN_GUI_EXEC_DIALOG_FN GWEN_Gui_SetExecDialogFn(GWEN_GUI *gui, GWEN_GUI_EXEC_DIALOG_FN f) {
  GWEN_GUI_EXEC_DIALOG_FN of;

  assert(gui);
  of=gui->execDialogFn;
  gui->execDialogFn=f;

  return of;
}



GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN
GWEN_Gui_SetKeyDataFromTextOpenSslFn(GWEN_GUI *gui,
				     GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN f) {
  GWEN_GUI_KEYDATAFROMTEXT_OPENSSL_FN of;

  assert(gui);
  of=gui->keyDataFromTextOpenSslFn;
  gui->keyDataFromTextOpenSslFn=f;

  return of;

}



uint32_t GWEN_Gui_GetFlags(const GWEN_GUI *gui) {
  assert(gui);
  return gui->flags;
}



void GWEN_Gui_SetFlags(GWEN_GUI *gui, uint32_t fl) {
  assert(gui);
  gui->flags=fl;
}



void GWEN_Gui_AddFlags(GWEN_GUI *gui, uint32_t fl) {
  assert(gui);
  gui->flags|=fl;
}



void GWEN_Gui_SubFlags(GWEN_GUI *gui, uint32_t fl) {
  assert(gui);
  gui->flags&=~fl;
}










int GWEN_Gui_MessageBox(uint32_t flags,
			const char *title,
			const char *text,
			const char *b1,
			const char *b2,
			const char *b3,
			uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->messageBoxFn)
    return gwenhywfar_gui->messageBoxFn(gwenhywfar_gui,
					flags,
					title,
					text,
					b1, b2, b3, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_InputBox(uint32_t flags,
		      const char *title,
		      const char *text,
		      char *buffer,
		      int minLen,
		      int maxLen,
		      uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->inputBoxFn)
    return gwenhywfar_gui->inputBoxFn(gwenhywfar_gui,
				      flags,
                                      title,
				      text,
				      buffer,
				      minLen, maxLen, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



uint32_t GWEN_Gui_ShowBox(uint32_t flags,
			  const char *title,
			  const char *text,
			  uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->showBoxFn)
    return gwenhywfar_gui->showBoxFn(gwenhywfar_gui,
				     flags,
				     title,
				     text,
				     guiid);
  return 0;
}



void GWEN_Gui_HideBox(uint32_t id) {
  if (gwenhywfar_gui && gwenhywfar_gui->hideBoxFn)
    return gwenhywfar_gui->hideBoxFn(gwenhywfar_gui, id);
}



uint32_t GWEN_Gui_ProgressStart(uint32_t progressFlags,
				const char *title,
				const char *text,
				uint64_t total,
				uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressStartFn)
    return gwenhywfar_gui->progressStartFn(gwenhywfar_gui,
					   progressFlags,
					   title,
					   text,
					   total,
					   guiid);
  return 0;
}



int GWEN_Gui_ProgressAdvance(uint32_t id, uint32_t progress) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressAdvanceFn)
    return gwenhywfar_gui->progressAdvanceFn(gwenhywfar_gui,
					     id,
					     progress);
  return 0;
}



int GWEN_Gui_ProgressLog(uint32_t id,
			 GWEN_LOGGER_LEVEL level,
			 const char *text) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressLogFn)
    return gwenhywfar_gui->progressLogFn(gwenhywfar_gui,
					 id, level, text);
  return 0;
}



int GWEN_Gui_ProgressEnd(uint32_t id) {
  if (gwenhywfar_gui && gwenhywfar_gui->progressEndFn)
    return gwenhywfar_gui->progressEndFn(gwenhywfar_gui, id);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_Print(const char *docTitle,
		   const char *docType,
		   const char *descr,
		   const char *text,
		   uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->printFn)
    return gwenhywfar_gui->printFn(gwenhywfar_gui,
				   docTitle,
				   docType,
				   descr,
				   text,
				   guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_GetPassword(uint32_t flags,
			 const char *token,
			 const char *title,
			 const char *text,
			 char *buffer,
			 int minLen,
			 int maxLen,
			 uint32_t guiid) {
  if (gwenhywfar_gui) {
    if (gwenhywfar_gui->getPasswordFn)
      return gwenhywfar_gui->getPasswordFn(gwenhywfar_gui,
					   flags,
					   token,
					   title,
					   text,
					   buffer,
					   minLen,
					   maxLen,
					   guiid);
    else
      if (gwenhywfar_gui->inputBoxFn)
	return gwenhywfar_gui->inputBoxFn(gwenhywfar_gui,
					flags,
					title,
					text,
					buffer,
					minLen,
					maxLen,
					guiid);
  }
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_SetPasswordStatus(const char *token,
			       const char *pin,
			       GWEN_GUI_PASSWORD_STATUS status,
			       uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->setPasswordStatusFn)
    return gwenhywfar_gui->setPasswordStatusFn(gwenhywfar_gui,
					       token, pin, status, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_LogHook(const char *logDomain,
		     GWEN_LOGGER_LEVEL priority, const char *s) {
  if (gwenhywfar_gui && gwenhywfar_gui->logHookFn) {
    if (priority>=GWEN_LoggerLevel_Debug &&
	logDomain &&
	strcasecmp(logDomain, "gwenhywfar")==0)
      /* don't send possibly sensitive data to the log function because
       * some application tend to store the messages indiscriminately.
       * In some cases sensitive information can be send to this function
       * which we don't want the application to store */
      return 0;
    else
      /* otherwise the log message seems to be uncritical, convey it */
      return gwenhywfar_gui->logHookFn(gwenhywfar_gui, logDomain, priority, s);
  }
  else
    /* handle as usual */
    return 0;
}



int GWEN_Gui_WaitForSockets(GWEN_SOCKET_LIST2 *readSockets,
			    GWEN_SOCKET_LIST2 *writeSockets,
			    uint32_t guiid,
			    int msecs) {
  if (gwenhywfar_gui && gwenhywfar_gui->waitForSocketsFn)
    return gwenhywfar_gui->waitForSocketsFn(gwenhywfar_gui, readSockets, writeSockets, guiid, msecs);
  else {
    GWEN_SOCKETSET *rset;
    GWEN_SOCKETSET *wset;
    GWEN_SOCKET_LIST2_ITERATOR *sit;

    rset=GWEN_SocketSet_new();
    wset=GWEN_SocketSet_new();

    /* fill read socket set */
    sit=GWEN_Socket_List2_First(readSockets);
    if (sit) {
      GWEN_SOCKET *s;

      s=GWEN_Socket_List2Iterator_Data(sit);
      assert(s);

      while(s) {
	GWEN_SocketSet_AddSocket(rset, s);
	s=GWEN_Socket_List2Iterator_Next(sit);
      }
      GWEN_Socket_List2Iterator_free(sit);
    }

    /* fill write socket set */
    sit=GWEN_Socket_List2_First(writeSockets);
    if (sit) {
      GWEN_SOCKET *s;

      s=GWEN_Socket_List2Iterator_Data(sit);
      assert(s);

      while(s) {
	GWEN_SocketSet_AddSocket(wset, s);
	s=GWEN_Socket_List2Iterator_Next(sit);
      }
      GWEN_Socket_List2Iterator_free(sit);
    }

    if (GWEN_SocketSet_GetSocketCount(rset)==0 &&
	GWEN_SocketSet_GetSocketCount(wset)==0) {
      /* no sockets to wait for, sleep for a few ms to keep cpu load down */
      GWEN_SocketSet_free(wset);
      GWEN_SocketSet_free(rset);

      if (msecs) {
	/* only sleep if a timeout was given */
	DBG_DEBUG(GWEN_LOGDOMAIN, "Sleeping (no socket)");
	GWEN_Socket_Select(NULL, NULL, NULL, GWEN_GUI_CPU_TIMEOUT);
      }
      return GWEN_ERROR_TIMEOUT;
    }
    else {
      int rv;

      rv=GWEN_Socket_Select(rset, wset, NULL, msecs);
      GWEN_SocketSet_free(wset);
      GWEN_SocketSet_free(rset);

      return rv;
    }
  }
}



int GWEN_Gui_CheckCert(const GWEN_SSLCERTDESCR *cd, GWEN_IO_LAYER *io, uint32_t guiid) {
  if (gwenhywfar_gui && gwenhywfar_gui->checkCertFn)
    return gwenhywfar_gui->checkCertFn(gwenhywfar_gui, cd, io, guiid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_CheckCertBuiltIn(GWEN_UNUSED GWEN_GUI *gui,
			      const GWEN_SSLCERTDESCR *cd,
			      GWEN_UNUSED GWEN_IO_LAYER *io, uint32_t guiid) {
  int rv;
  int isError;
  const char *hash;
  const char *status;
  const char *ipAddr;
  const char *statusOn;
  const char *statusOff;
  char varName[128];
  char dbuffer1[32];
  char dbuffer2[32];
  char buffer[8192];
  const GWEN_TIME *ti;
  const char *unknown;
  const char *commonName;
  const char *organizationName;
  const char *organizationalUnitName;
  const char *countryName;
  const char *localityName;
  const char *stateOrProvinceName;

  char *msg=I18S(
    "The following certificate has been received:\n"
    "Name        : %s\n"
    "Organisation: %s\n"
    "Department  : %s\n"
    "Country     : %s\n"
    "City        : %s\n"
    "State       : %s\n"
    "Valid after : %s\n"
    "Valid until : %s\n"
    "Hash        : %s\n"
    "Status      : %s\n"
    "Do you wish to accept this certificate?"

    "<html>"
    " <p>"
    "  The following certificate has been received:"
    " </p>"
    " <table>"
    "  <tr><td>Name</td><td>%s</td></tr>"
    "  <tr><td>Organisation</td><td>%s</td></tr>"
    "  <tr><td>Department</td><td>%s</td></tr>"
    "  <tr><td>Country</td><td>%s</td></tr>"
    "  <tr><td>City</td><td>%s</td></tr>"
    "  <tr><td>State</td><td>%s</td></tr>"
    "  <tr><td>Valid after</td><td>%s</td></tr>"
    "  <tr><td>Valid until</td><td>%s</td></tr>"
    "  <tr><td>Hash</td><td>%s</td></tr>"
    "  <tr><td>Status</td><td>%s%s%s</td></tr>"
    " </table>"
    " <p>"
    "  Do you wish to accept this certificate?"
    " </p>"
    "</html>"
    );

  memset(dbuffer1, 0, sizeof(dbuffer1));
  memset(dbuffer2, 0, sizeof(dbuffer2));
  memset(varName, 0, sizeof(varName));

  isError=GWEN_SslCertDescr_GetIsError(cd);

  hash=GWEN_SslCertDescr_GetFingerPrint(cd);
  status=GWEN_SslCertDescr_GetStatusText(cd);
  ipAddr=GWEN_SslCertDescr_GetIpAddress(cd);

  ti=GWEN_SslCertDescr_GetNotBefore(cd);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    /* TRANSLATORS: This string is used as a template string to
       convert a given time into your local translated timeformat. The
       following characters are accepted in the template string: Y -
       digit of the year, M - digit of the month, D - digit of the day
       of month, h - digit of the hour, m - digit of the minute, s-
       digit of the second. All other characters are left unchanged. */
    if (GWEN_Time_toString(ti, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not convert beforeDate to string");
      abort();
    }
    strncpy(dbuffer1, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer1)-1);
    GWEN_Buffer_free(tbuf);
  }

  ti=GWEN_SslCertDescr_GetNotAfter(cd);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(ti, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not convert untilDate to string");
      abort();
    }
    strncpy(dbuffer2, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer2)-1);
    GWEN_Buffer_free(tbuf);
  }

  if (isError) {
    statusOn="<font color=red>";
    statusOff="</font>";
  }
  else {
    statusOn="<font color=green>";
    statusOff="</font>";
  }

  unknown=I18N("unknown");
  commonName=GWEN_SslCertDescr_GetCommonName(cd);
  if (!commonName)
    commonName=unknown;
  organizationName=GWEN_SslCertDescr_GetOrganizationName(cd);
  if (!organizationName)
    organizationName=unknown;
  organizationalUnitName=GWEN_SslCertDescr_GetOrganizationalUnitName(cd);
  if (!organizationalUnitName)
    organizationalUnitName=unknown;
  countryName=GWEN_SslCertDescr_GetCountryName(cd);
  if (!countryName)
    countryName=unknown;
  localityName=GWEN_SslCertDescr_GetLocalityName(cd);
  if (!localityName)
    localityName=unknown;
  stateOrProvinceName=GWEN_SslCertDescr_GetStateOrProvinceName(cd);
  if (!stateOrProvinceName)
    stateOrProvinceName=unknown;
  if (!status)
    status=unknown;

  snprintf(buffer, sizeof(buffer)-1,
	   I18N(msg),
	   commonName,
	   organizationName,
	   organizationalUnitName,
	   countryName,
	   localityName,
	   stateOrProvinceName,
	   dbuffer1, dbuffer2,
	   hash,
	   status,
	   /* the same again for HTML */
	   commonName,
	   organizationName,
	   organizationalUnitName,
	   countryName,
	   localityName,
	   stateOrProvinceName,
	   dbuffer1, dbuffer2,
	   hash,
	   statusOn,
	   status,
	   statusOff
	  );

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
			 GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			 GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
			 I18N("Certificate Received"),
			 buffer,
			 I18N("Yes"), I18N("No"), 0, guiid);
  if (rv==1) {
    return 0;
  }
  else {
    DBG_NOTICE(GWEN_LOGDOMAIN, "User rejected certificate");

    return GWEN_ERROR_SSL_SECURITY;
  }
}



int GWEN_Gui_KeyDataFromText_OpenSSL(const char *text,
				     unsigned char *buffer,
				     unsigned int bufLength) {
  if (gwenhywfar_gui && gwenhywfar_gui->keyDataFromTextOpenSslFn)
    return gwenhywfar_gui->keyDataFromTextOpenSslFn(gwenhywfar_gui,
						    text,
						    buffer,
                                                    bufLength);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_ExecDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg) {
  if (gwenhywfar_gui && gwenhywfar_gui->execDialogFn)
    return gwenhywfar_gui->execDialogFn(gui, dlg);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}













