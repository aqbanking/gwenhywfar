/***************************************************************************
    begin       : Sat Dec 16 2012
    copyright   : (C) 2012 by Martin Preuss
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

#include "passwdstore_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gui.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/smalltresor.h>

#include <errno.h>



GWEN_PASSWD_STORE *GWEN_PasswordStore_new(const char *fname) {
  GWEN_PASSWD_STORE *sto;

  GWEN_NEW_OBJECT(GWEN_PASSWD_STORE, sto);
  if (fname)
    sto->fileName=strdup(fname);
  sto->dbPasswords=NULL;

  return sto;
}



void GWEN_PasswordStore_free(GWEN_PASSWD_STORE *sto) {
  if (sto) {
    memset(sto->pw, 0, sizeof(sto->pw));
    if (sto->dbPasswords) {
      GWEN_DB_ModifyBranchFlagsDown(sto->dbPasswords, GWEN_DB_NODE_FLAGS_SAFE, GWEN_DB_NODE_FLAGS_SAFE);
      GWEN_DB_Group_free(sto->dbPasswords);
      sto->dbPasswords=NULL;
    }
    free(sto->fileName);
    GWEN_FREE_OBJECT(sto);
  }
}




static int readFile(const char *fname, GWEN_BUFFER *dbuf) {
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while(!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
	break;
      if (s==(ssize_t)-1) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "fread(%s): %s",
		  fname, strerror(errno));
	fclose(f);
	return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    if (errno==ENOENT) {
      DBG_INFO(GWEN_LOGDOMAIN, "File [%s] does not exist", fname);
      return GWEN_ERROR_NOT_FOUND;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "fopen(%s): %s",
                fname, strerror(errno));
      return GWEN_ERROR_IO;
    }
  }
}



static int writeToFile(FILE *f, const char *p, int len) {
  while(len>0) {
    ssize_t l;
    ssize_t s;

    l=1024;
    if (l>len)
      l=len;
    s=fwrite(p, 1, l, f);
    if (s==(ssize_t)-1 || s==0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
	       "fwrite: %s",
	       strerror(errno));
      return GWEN_ERROR_IO;
    }
    p+=s;
    len-=s;
  }

  return 0;
}



static int writeFile(const char *fname, const char *p, int len) {
  FILE *f;

  f=fopen(fname, "wb");
  if (f) {
    int rv;

    rv=writeToFile(f, p, len);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
      fclose(f);
      return rv;
    }
    if (fclose(f)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s",
	      fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return 0;
}



static int GWEN_PasswordStore_Digest(const uint8_t *t, uint32_t size, GWEN_BUFFER *buf) {
  GWEN_MDIGEST *md;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)t, size);
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  GWEN_Buffer_AppendBytes(buf,
                          (const char*)GWEN_MDigest_GetDigestPtr(md),
                          GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);
  return 0;
}



static int GWEN_PasswordStore_CheckDigest(const uint8_t *t, uint32_t size, const uint8_t *h) {
  GWEN_MDIGEST *md;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)t, size);
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  if (memcmp(h, GWEN_MDigest_GetDigestPtr(md), 20)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad hash");
    GWEN_MDigest_free(md);
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_MDigest_free(md);
  return 0;
}



static void GWEN_PasswordStore_SafeFreeDb(GWEN_PASSWD_STORE *sto) {
  if (sto->dbPasswords) {
    GWEN_DB_ModifyBranchFlagsDown(sto->dbPasswords, GWEN_DB_NODE_FLAGS_SAFE, GWEN_DB_NODE_FLAGS_SAFE);
    GWEN_DB_Group_free(sto->dbPasswords);
    sto->dbPasswords=NULL;
  }
}



static int GWEN_PasswordStore_ReadDecryptFile(GWEN_PASSWD_STORE *sto, GWEN_BUFFER *secbuf) {
  int rv;
  GWEN_BUFFER *sbuf;

  sbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile(sto->fileName, sbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    return rv;
  }

  if (GWEN_Buffer_GetUsedBytes(sbuf)<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Empty file");
    GWEN_Buffer_free(sbuf);
    return GWEN_ERROR_NO_DATA;
  }

  for (;;) {
    GWEN_BUFFER *tbuf;
    uint32_t pos1;
    uint32_t pos2;
    uint32_t len;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, "PASSWORD_STORE_");
    GWEN_Text_UnescapeToBufferTolerant(sto->fileName, tbuf);
  
    if (sto->pw[0]==0) {
      rv=GWEN_Gui_InputBox(GWEN_GUI_INPUT_FLAGS_DIRECT,
                           I18N("Enter Password"),
                           I18N("Please enter the password for the password store\n"
                                "<html>"
                                "Please enter the password for the password store</br>"
                                "</html>"),
                           sto->pw,
                           4,
                           sizeof(sto->pw)-1,
                           0);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(tbuf);
        GWEN_Buffer_free(sbuf);
        return rv;
      }
    }
  
    pos1=GWEN_Buffer_GetPos(secbuf);
  
    rv=GWEN_SmallTresor_Decrypt((const uint8_t*) GWEN_Buffer_GetStart(sbuf),
                                GWEN_Buffer_GetUsedBytes(sbuf),
                                sto->pw,
                                secbuf,
                                GWEN_PASSWDSTORE_PW_ITERATIONS,
                                GWEN_PASSWDSTORE_CRYPT_ITERATIONS);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(sbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    }
    else {
      /* check and remove hash */
      pos2=GWEN_Buffer_GetPos(secbuf);
      len=pos2-pos1;
    
      if (len>=20) {
        const uint8_t *p1;
        const uint8_t *p2;
    
        p1=(const uint8_t*)GWEN_Buffer_GetStart(secbuf)+pos1;      /* start of decrypted data */
        p2=(const uint8_t*)GWEN_Buffer_GetStart(secbuf)+(pos2-20); /* start of hash */
    
        rv=GWEN_PasswordStore_CheckDigest(p1, len-20, p2);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        else {
          GWEN_Buffer_Crop(secbuf, 0, pos2-20);
          break;
        }
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad data size (smaller than 20 bytes)");
        /* reset buffer */
        GWEN_Buffer_Crop(secbuf, 0, pos1);
      }
    }
  }

  return 0;
}



static int GWEN_PasswordStore_EncryptWriteFile(GWEN_PASSWD_STORE *sto, const uint8_t *sec, uint32_t len) {
  int rv;
  GWEN_BUFFER *sbuf;
  GWEN_BUFFER *tbuf;

  /* make sure the data dir exists */
  DBG_ERROR(0, "Looking for [%s]", sto->fileName);
  rv=GWEN_Directory_GetPath(sto->fileName, GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, "PASSWORD_STORE_");
  GWEN_Text_UnescapeToBufferTolerant(sto->fileName, tbuf);

  /* ask for passwd if not already set */
  if (sto->pw[0]==0) {
    uint32_t flags;

    flags=GWEN_GUI_INPUT_FLAGS_DIRECT;
    if (sto->isNew)
      flags|=GWEN_GUI_INPUT_FLAGS_CONFIRM;
    rv=GWEN_Gui_InputBox(flags,
                         I18N("Enter Password"),
                         I18N("Please enter the password for the password store\n"
                              "<html>"
                              "Please enter the password for the password store</br>"
                              "</html>"),
                         sto->pw,
                         4,
                         sizeof(sto->pw)-1,
                         0);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
  }

  GWEN_Buffer_free(tbuf);

  /* prepare data to write */
  sbuf=GWEN_Buffer_new(0, 256, 0, 1);
  tbuf=GWEN_Buffer_new(0, len+20, 0, 1);

  /* add clear text data */
  GWEN_Buffer_AppendBytes(tbuf, (const char*) sec, len);

  /* add hash (20 bytes) */
  rv=GWEN_PasswordStore_Digest((const uint8_t*) sec, len, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(sbuf);
    return rv;
  }

  /* encrypt cleartext */
  rv=GWEN_SmallTresor_Encrypt((const uint8_t*) GWEN_Buffer_GetStart(tbuf),
                              GWEN_Buffer_GetUsedBytes(tbuf),
                              sto->pw,
			      sbuf,
                              GWEN_PASSWDSTORE_PW_ITERATIONS,
                              GWEN_PASSWDSTORE_CRYPT_ITERATIONS);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(sbuf);
    return rv;
  }
  GWEN_Buffer_OverwriteContent(tbuf, 0);
  GWEN_Buffer_free(tbuf);

  /* write file */
  rv=writeFile(sto->fileName,
               GWEN_Buffer_GetStart(sbuf),
               GWEN_Buffer_GetUsedBytes(sbuf));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    return rv;
  }

  GWEN_Buffer_free(sbuf);

  return 0;
}




static int GWEN_PasswordStore_ReadFile(GWEN_PASSWD_STORE *sto) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  rv=GWEN_PasswordStore_ReadDecryptFile(sto, tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  sto->isNew=0;

  GWEN_PasswordStore_SafeFreeDb(sto);

  sto->dbPasswords=GWEN_DB_Group_new("passwords");
  rv=GWEN_DB_ReadFromString(sto->dbPasswords,
                            GWEN_Buffer_GetStart(tbuf),
                            GWEN_Buffer_GetUsedBytes(tbuf),
                            GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_OverwriteContent(tbuf, 0);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_DB_ModifyBranchFlagsDown(sto->dbPasswords, GWEN_DB_NODE_FLAGS_SAFE, GWEN_DB_NODE_FLAGS_SAFE);

  GWEN_Buffer_OverwriteContent(tbuf, 0);
  GWEN_Buffer_free(tbuf);
  return 0;
}



static int GWEN_PasswordStore_WriteFile(GWEN_PASSWD_STORE *sto) {
  if (sto->dbPasswords) {
    GWEN_BUFFER *tbuf;
    int rv;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_DB_WriteToBuffer(sto->dbPasswords, tbuf, GWEN_DB_FLAGS_DEFAULT);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_OverwriteContent(tbuf, 0);
      GWEN_Buffer_free(tbuf);
      return rv;
    }

    rv=GWEN_PasswordStore_EncryptWriteFile(sto,
                                           (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
                                           GWEN_Buffer_GetUsedBytes(tbuf));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_OverwriteContent(tbuf, 0);
      GWEN_Buffer_free(tbuf);
      return rv;
    }

    GWEN_Buffer_OverwriteContent(tbuf, 0);
    GWEN_Buffer_free(tbuf);
    return 0;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No password db");
    return GWEN_ERROR_INTERNAL;
  }
}




static int GWEN_PasswordStore__SetPassword(GWEN_PASSWD_STORE *sto, const char *token, const char *secret) {
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Text_EscapeToBufferTolerant(token, buf);

  if (secret==NULL)
    GWEN_DB_DeleteVar(sto->dbPasswords, GWEN_Buffer_GetStart(buf));
  else
    GWEN_DB_SetCharValue(sto->dbPasswords, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         GWEN_Buffer_GetStart(buf), secret);
  GWEN_DB_ModifyBranchFlagsDown(sto->dbPasswords, GWEN_DB_NODE_FLAGS_SAFE, GWEN_DB_NODE_FLAGS_SAFE);
  GWEN_Buffer_free(buf);

  return 0;
}



static int GWEN_PasswordStore__GetPassword(GWEN_PASSWD_STORE *sto, const char *token, char *buffer, int minLen, int maxLen) {
  GWEN_BUFFER *buf;
  const char *s;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Text_EscapeToBufferTolerant(token, buf);

  s=GWEN_DB_GetCharValue(sto->dbPasswords,
                         GWEN_Buffer_GetStart(buf),
                         0, NULL);
  if (s) {
    int i;

    i=strlen(s);
    if (i>=minLen && i < maxLen) {
      memmove(buffer, s, i+1);
      GWEN_Buffer_free(buf);
      return 0;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Stored password [%s] is not within size limits (%d), rejecting.",
                GWEN_Buffer_GetStart(buf), i);
    }
  }

  GWEN_Buffer_free(buf);
  return GWEN_ERROR_NOT_FOUND;
}





int GWEN_PasswordStore_SetPassword(GWEN_PASSWD_STORE *sto, const char *token, const char *secret) {
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT rs;
  int rv;

  /* make sure path exists */
  rv=GWEN_Directory_GetPath(sto->fileName, GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* lock file */
  lck=GWEN_FSLock_new(sto->fileName, GWEN_FSLock_TypeFile);
  rs=GWEN_FSLock_Lock(lck, 60*1000, 0);
  if (rs!=GWEN_FSLock_ResultOk) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rs);
    return GWEN_ERROR_IO;
  }

  /* read and decrypt file */
  rv=GWEN_PasswordStore_ReadFile(sto);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND || rv==GWEN_ERROR_NO_DATA) {
      DBG_INFO(GWEN_LOGDOMAIN, "Will create password store [%s]", sto->fileName);
      if (sto->dbPasswords==NULL) {
        sto->dbPasswords=GWEN_DB_Group_new("passwords");
        GWEN_DB_ModifyBranchFlagsDown(sto->dbPasswords, GWEN_DB_NODE_FLAGS_SAFE, GWEN_DB_NODE_FLAGS_SAFE);
      }
      sto->isNew=1;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
      return rv;
    }
  }

  /* set password in db */
  rv=GWEN_PasswordStore__SetPassword(sto, token, secret);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
    return rv;
  }

  /* write file back */
  rv=GWEN_PasswordStore_WriteFile(sto);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
    return rv;
  }

  /* unlock file */
  GWEN_FSLock_Unlock(lck);
  GWEN_FSLock_free(lck);

  /* release passwords */
  GWEN_PasswordStore_SafeFreeDb(sto);

  return 0;
}




int GWEN_PasswordStore_GetPassword(GWEN_PASSWD_STORE *sto, const char *token, char *buffer, int minLen, int maxLen) {
  int rv;
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT rs;

  /* make sure path exists */
  rv=GWEN_Directory_GetPath(sto->fileName, GWEN_PATH_FLAGS_VARIABLE);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* lock file */
  lck=GWEN_FSLock_new(sto->fileName, GWEN_FSLock_TypeFile);
  rs=GWEN_FSLock_Lock(lck, 60*1000, 0);
  if (rs!=GWEN_FSLock_ResultOk) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rs);
    return GWEN_ERROR_IO;
  }

  /* read and decode file */
  rv=GWEN_PasswordStore_ReadFile(sto);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
    return rv;
  }

  /* unlock file */
  GWEN_FSLock_Unlock(lck);
  GWEN_FSLock_free(lck);

  /* finally get password, if possible */
  rv=GWEN_PasswordStore__GetPassword(sto, token, buffer, minLen, maxLen);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* release passwords */
  GWEN_PasswordStore_SafeFreeDb(sto);

  return 0;
}




