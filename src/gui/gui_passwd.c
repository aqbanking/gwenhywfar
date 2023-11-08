/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2023 by Martin Preuss
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


static int _tryReadStoredPasswd(GWEN_GUI *gui,
				const char *pwName,
				uint32_t flags,
				const char *token,
				char *buffer,
				int minLen,
				int maxLen);
static int _tryReadCachedPasswd(GWEN_GUI *gui, const char *pwName, char *buffer, int minLen, int maxLen);
static int _tryReadPasswdViaInputBox(GWEN_GUI *gui,
				     uint32_t flags,
				     const char *token,
				     const char *title,
				     const char *text,
				     char *buffer,
				     int minLen,
				     int maxLen,
				     uint32_t guiid);
static void _tryStorePasswdInCacheAndStorage(GWEN_GUI *gui,
					     const char *pwName,
					     const char *token,
					     const char *pwBuffer,
					     int userWantsToStorePasswd);



static int GWEN_Gui__HashPair(const char *token, const char *pin, GWEN_BUFFER *buf)
{
  GWEN_MDIGEST *md;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Md5_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t *)token, strlen(token));
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t *)pin, strlen(pin));
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  GWEN_Text_ToHexBuffer((const char *)GWEN_MDigest_GetDigestPtr(md),
                        GWEN_MDigest_GetDigestSize(md),
                        buf,
                        0, 0, 0);
  GWEN_MDigest_free(md);
  return 0;
}




static int GWENHYWFAR_CB GWEN_Gui_Internal_GetPassword(GWEN_GUI *gui,
                                                       uint32_t flags,
                                                       const char *token,
                                                       const char *title,
                                                       const char *text,
                                                       char *pwBuffer,
                                                       int minLen,
                                                       int maxLen,
                                                       GWEN_UNUSED GWEN_GUI_PASSWORD_METHOD methodId,
                                                       GWEN_UNUSED GWEN_DB_NODE *methodParams,
                                                       uint32_t guiid)
{
  if ((flags & GWEN_GUI_INPUT_FLAGS_TAN) || (flags & GWEN_GUI_INPUT_FLAGS_DIRECT) || (gui->dbPasswords==NULL)) {
    return GWEN_Gui_InputBox(flags, title, text, pwBuffer, minLen, maxLen, guiid);
  }
  else {
    GWEN_BUFFER *bufPasswdName;
    int rv;

    bufPasswdName=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_EscapeToBufferTolerant(token, bufPasswdName);

    rv=_tryReadStoredPasswd(gui, GWEN_Buffer_GetStart(bufPasswdName), flags, token, pwBuffer, minLen, maxLen);
    if (rv!=0) {
      GWEN_Buffer_free(bufPasswdName);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      else if (rv>0) {
	/* got password */
	return 0;
      }
    }

    if (gui->flags & GWEN_GUI_FLAGS_NONINTERACTIVE) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Password for [%s] missing in noninteractive mode, aborting", GWEN_Buffer_GetStart(bufPasswdName));
      GWEN_Buffer_free(bufPasswdName);
      return GWEN_ERROR_USER_ABORTED;
    }

    rv=_tryReadPasswdViaInputBox(gui, flags, token, title, text, pwBuffer, minLen, maxLen, guiid);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(bufPasswdName);
      return rv;
    }

    _tryStorePasswdInCacheAndStorage(gui, GWEN_Buffer_GetStart(bufPasswdName), token, pwBuffer, (rv==1)?1:0);

    GWEN_Buffer_free(bufPasswdName);
    return 0;
  }
}



int _tryReadStoredPasswd(GWEN_GUI *gui, const char *pwName, uint32_t flags, const char *token, char *buffer, int minLen, int maxLen)
{
  int rv;

  /* look into password cache */
  if (!(flags & GWEN_GUI_INPUT_FLAGS_CONFIRM)) {
    rv=_tryReadCachedPasswd(gui, pwName, buffer, minLen, maxLen);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else if (rv>0) {
      /* got password */
      return 1;
    }

    /* look into password storage */
    if (gui->passwdStore) {
      rv=GWEN_PasswordStore_GetPassword(gui->passwdStore, token, buffer, minLen, maxLen);
      if (rv<0) {
	if (rv==GWEN_ERROR_NOT_FOUND || rv==GWEN_ERROR_NO_DATA) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Password not found in PasswordStore");
	}
	else {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
      }
      else {
	/* got password */
	return 1;
      }
    }
  }
  return 0;
}



int _tryReadCachedPasswd(GWEN_GUI *gui, const char *pwName, char *buffer, int minLen, int maxLen)
{
  const char *s;

  s=GWEN_DB_GetCharValue(gui->dbPasswords, pwName, 0, NULL);
  if (s) {
    int i;

    i=strlen(s);
    if (i>=minLen && i < maxLen) {
      memmove(buffer, s, i+1);
      return 1;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Stored password [%s] is not within size limits (%d), rejecting.", pwName, i);
    }
  }
  return 0;
}



int _tryReadPasswdViaInputBox(GWEN_GUI *gui,
			      uint32_t flags,
			      const char *token,
			      const char *title,
			      const char *text,
			      char *buffer,
			      int minLen,
			      int maxLen,
			      uint32_t guiid)
{
  for (;;) {
    int rv;
    int rv2;

    rv=GWEN_Gui_InputBox(flags, title, text, buffer, minLen, maxLen, guiid);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else {
      GWEN_BUFFER *hbuf;
      int isBad=0;

      hbuf=GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_Gui__HashPair(token, buffer, hbuf);
      isBad=GWEN_StringList_HasString(gui->badPasswords, GWEN_Buffer_GetStart(hbuf));
      if (!isBad) {
	/* password not marked as bad, return it */
	GWEN_Buffer_free(hbuf);
	return rv; /* return rv from GWEN_Gui_InputBox */
      }
      rv2=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
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
			      I18N("Yes, use anyway"),
			      I18N("Re-enter"),
			      0,
			      guiid);
      if (rv2==1) {
	/* accept this input */
	GWEN_StringList_RemoveString(gui->badPasswords, GWEN_Buffer_GetStart(hbuf));
	GWEN_Buffer_free(hbuf);
	return rv; /* return rv from GWEN_Gui_InputBox */
      }
      GWEN_Buffer_free(hbuf);
    }
  } /* for */
  /* should not get here */
  return GWEN_ERROR_INTERNAL;
}



void _tryStorePasswdInCacheAndStorage(GWEN_GUI *gui,
				      const char *pwName,
				      const char *token,
				      const char *pwBuffer,
				      int userWantsToStorePasswd)
{
  /* store in temporary cache */
  GWEN_DB_SetCharValue(gui->dbPasswords, GWEN_DB_FLAGS_OVERWRITE_VARS, pwName, pwBuffer);

  /* only store passwd in storage if allowed by the user */
  if (userWantsToStorePasswd && gui->passwdStore) {
    int rv;

    rv=GWEN_PasswordStore_SetPassword(gui->passwdStore, token, pwBuffer);
    if (rv<0) {
      DBG_WARN(GWEN_LOGDOMAIN, "Could not store password (%d)", rv);
    }
  }
}



static int GWENHYWFAR_CB GWEN_Gui_Internal_SetPasswordStatus(GWEN_GUI *gui,
                                                             const char *token,
                                                             const char *pin,
                                                             GWEN_GUI_PASSWORD_STATUS status,
                                                             GWEN_UNUSED uint32_t guiid)
{
  if (token==NULL && pin==NULL && status==GWEN_Gui_PasswordStatus_Remove) {
    /* complete cleaning is requested */
    if (gui->passwdStore)
      GWEN_PasswordStore_ClearStoragePasswd(gui->passwdStore);
    if (gui->persistentPasswords==0)
      GWEN_DB_ClearGroup(gui->dbPasswords, NULL);
  }
  else {
    GWEN_BUFFER *hbuf;

    /* setting ststus of a specific password/pin */
    hbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Gui__HashPair(token, pin, hbuf);
    if (status==GWEN_Gui_PasswordStatus_Bad) {
      GWEN_StringList_AppendString(gui->badPasswords,
				   GWEN_Buffer_GetStart(hbuf),
				   0, 1);
      /* remove from permanent passwd storage */
      if (gui->passwdStore) {
	int rv;

	rv=GWEN_PasswordStore_SetPassword(gui->passwdStore, token, NULL);
	if (rv<0) {
	  DBG_WARN(GWEN_LOGDOMAIN, "Could not remove password from storage (%d)", rv);
	}
      }

      if (gui->dbPasswords) {
	GWEN_BUFFER *buf;

	buf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Text_EscapeToBufferTolerant(token, buf);

	GWEN_DB_DeleteVar(gui->dbPasswords, GWEN_Buffer_GetStart(buf));
      }
    }
    else if (status==GWEN_Gui_PasswordStatus_Ok ||
             status==GWEN_Gui_PasswordStatus_Remove) {
      if (gui->persistentPasswords==0)
        GWEN_StringList_RemoveString(gui->badPasswords, GWEN_Buffer_GetStart(hbuf));
    }
    GWEN_Buffer_free(hbuf);
  }

  return 0;
}



int GWEN_Gui_GetPassword(uint32_t flags,
                         const char *token,
                         const char *title,
                         const char *text,
                         char *buffer,
                         int minLen,
                         int maxLen,
                         GWEN_GUI_PASSWORD_METHOD methodId,
                         GWEN_DB_NODE *methodParams,
                         uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui) {
    if (gui->getPasswordFn)
      return gui->getPasswordFn(gui, flags, token, title, text, buffer, minLen, maxLen, methodId, methodParams, guiid);
    else if (gui->inputBoxFn)
      return gui->inputBoxFn(gui, flags, title, text, buffer, minLen, maxLen, guiid);
  }
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Gui_SetPasswordStatus(const char *token,
                               const char *pin,
                               GWEN_GUI_PASSWORD_STATUS status,
                               uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->setPasswordStatusFn)
    return gui->setPasswordStatusFn(gui, token, pin, status, guiid);
  return GWEN_ERROR_NOT_IMPLEMENTED;
}




