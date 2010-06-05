/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "mdigest_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/gui.h>




GWEN_INHERIT_FUNCTIONS(GWEN_MDIGEST)
GWEN_LIST_FUNCTIONS(GWEN_MDIGEST, GWEN_MDigest)
GWEN_LIST2_FUNCTIONS(GWEN_MDIGEST, GWEN_MDigest)





GWEN_MDIGEST *GWEN_MDigest_new(GWEN_CRYPT_HASHALGOID a) {
  GWEN_MDIGEST *md;

  GWEN_NEW_OBJECT(GWEN_MDIGEST, md)
  md->refCount=1;
  GWEN_INHERIT_INIT(GWEN_MDIGEST, md)
  GWEN_LIST_INIT(GWEN_MDIGEST, md)

  md->hashAlgoId=a;
  return md;
}



void GWEN_MDigest_free(GWEN_MDIGEST *md) {
  if (md) {
    assert(md->refCount);
    if (md->refCount==1) {
      free(md->pDigest);
      md->refCount=0;
      GWEN_FREE_OBJECT(md);
    }
    else
      md->refCount--;
  }
}



GWEN_CRYPT_HASHALGOID GWEN_MDigest_GetHashAlgoId(const GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  return md->hashAlgoId;
}



uint8_t *GWEN_MDigest_GetDigestPtr(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  return md->pDigest;
}



unsigned int GWEN_MDigest_GetDigestSize(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  return md->lDigest;
}



void GWEN_MDigest_SetDigestBuffer(GWEN_MDIGEST *md, uint8_t *buf, unsigned int l) {
  assert(md);
  assert(md->refCount);

  if (l) {
    assert(buf);
  }

  if (md->pDigest && md->lDigest)
    free(md->pDigest);
  md->pDigest=buf;
  md->lDigest=l;
}



void GWEN_MDigest_SetDigestLen(GWEN_MDIGEST *md, unsigned int l) {
  assert(md);
  assert(md->refCount);

  if (md->pDigest && md->lDigest)
    free(md->pDigest);
  md->pDigest=NULL;
  md->lDigest=l;
}



int GWEN_MDigest_Begin(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  if (md->beginFn)
    return md->beginFn(md);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_MDigest_End(GWEN_MDIGEST *md) {
  assert(md);
  assert(md->refCount);
  if (md->endFn)
    return md->endFn(md);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_MDigest_Update(GWEN_MDIGEST *md, const uint8_t *buf, unsigned int l) {
  assert(md);
  assert(md->refCount);
  if (md->updateFn)
    return md->updateFn(md, buf, l);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_MDIGEST_BEGIN_FN GWEN_MDigest_SetBeginFn(GWEN_MDIGEST *md, GWEN_MDIGEST_BEGIN_FN f) {
  GWEN_MDIGEST_BEGIN_FN of;

  assert(md);
  assert(md->refCount);
  of=md->beginFn;
  md->beginFn=f;

  return of;
}



GWEN_MDIGEST_END_FN GWEN_MDigest_SetEndFn(GWEN_MDIGEST *md, GWEN_MDIGEST_END_FN f) {
  GWEN_MDIGEST_END_FN of;

  assert(md);
  assert(md->refCount);
  of=md->endFn;
  md->endFn=f;

  return of;
}



GWEN_MDIGEST_UPDATE_FN GWEN_MDigest_SetUpdateFn(GWEN_MDIGEST *md, GWEN_MDIGEST_UPDATE_FN f) {
  GWEN_MDIGEST_UPDATE_FN of;

  assert(md);
  assert(md->refCount);
  of=md->updateFn;
  md->updateFn=f;

  return of;
}



int GWEN_MDigest_PKPDF2(GWEN_MDIGEST *md,
			const char *password,
			const uint8_t *pSalt,
			uint32_t lSalt,
                        uint8_t *pKey,
			uint32_t lKey,
			uint32_t iterations) {
  int rv;
  uint8_t hash[128];
  uint32_t hsize;
  uint32_t i;

  hsize=GWEN_MDigest_GetDigestSize(md);
  if (lKey>hsize || lKey>sizeof(hash)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Derived key too long");
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_End(md);
    return rv;
  }

  /* hash password */
  rv=GWEN_MDigest_Update(md, (const uint8_t*) password, strlen(password));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_End(md);
    return rv;
  }

  /* hash salt */
  rv=GWEN_MDigest_Update(md, pSalt, lSalt);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_End(md);
    return rv;
  }

  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_End(md);
    return rv;
  }

  /* use that hash now for the iterations */
  memmove(hash, GWEN_MDigest_GetDigestPtr(md), hsize);

  for (i=2; i<iterations; i++) {
    rv=GWEN_MDigest_Begin(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_MDigest_End(md);
      return rv;
    }
    rv=GWEN_MDigest_Update(md, hash, hsize);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_MDigest_End(md);
      return rv;
    }

    rv=GWEN_MDigest_End(md);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_MDigest_End(md);
      return rv;
    }

    /* use that hash now for the next iteration */
    memmove(hash, GWEN_MDigest_GetDigestPtr(md), hsize);
  }

  /* done, copy key */
  memmove(pKey, hash, lKey);
  memset(hash, 0, sizeof(hash));

  return 0;
}



static int GWEN_MDigest__HashFile(GWEN_MDIGEST *md,
				  const char *fname,
				  GWEN_BUFFER *hbuf) {
  GWEN_SYNCIO *sio;
  int rv;
  uint8_t buffer[1024];

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_SetFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  while(1) {
    rv=GWEN_SyncIo_Read(sio, buffer, sizeof(buffer));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    else if (rv==0)
      break;
    else {
      rv=GWEN_MDigest_Update(md, (const uint8_t*) buffer, rv);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_SyncIo_Disconnect(sio);
	GWEN_SyncIo_free(sio);
	return rv;
      }
    }
  }

  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  rv=GWEN_Text_ToHexBuffer((const char*) GWEN_MDigest_GetDigestPtr(md),
			   GWEN_MDigest_GetDigestSize(md),
                           hbuf, 0, 0, 0);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



static int GWEN_MDigest__HashFileTree(GWEN_MDIGEST *md,
				      const char *baseFolder,
				      const char *relFolder,
				      const char *ignoreFile,
				      GWEN_STRINGLIST *sl) {
  GWEN_STRINGLIST *files;
  GWEN_STRINGLISTENTRY *se;
  GWEN_BUFFER *pbuf;
  uint32_t ppos;
  uint32_t rpos;
  int rv;

  files=GWEN_StringList_new();
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, baseFolder);
  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  rpos=GWEN_Buffer_GetPos(pbuf);
  if (relFolder) {
    GWEN_Buffer_AppendString(pbuf, relFolder);
    GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  }
  ppos=GWEN_Buffer_GetPos(pbuf);

  rv=GWEN_Directory_GetFileEntriesWithType(GWEN_Buffer_GetStart(pbuf), files, NULL);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    GWEN_StringList_free(files);
    return rv;
  }

  se=GWEN_StringList_FirstEntry(files);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, s+1);
      if (*s=='d') {
	if (strcasecmp(s+1, ".")!=0 && strcasecmp(s+1, "..")!=0) {
	  rv=GWEN_MDigest__HashFileTree(md,
					baseFolder,
					GWEN_Buffer_GetStart(pbuf)+rpos,
					ignoreFile,
					sl);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(pbuf);
	    GWEN_StringList_free(files);
            return rv;
	  }
	}
      }
      else if (*s=='f') {
	if (!(ignoreFile && strcasecmp(ignoreFile, s+1)==0)) {
	  GWEN_BUFFER *tbuf;

	  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

	  /* add relative path to line buffer */
	  GWEN_Buffer_AppendString(tbuf, "F");
	  rv=GWEN_Text_EscapeToBuffer(GWEN_Buffer_GetStart(pbuf)+rpos, tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(pbuf);
	    GWEN_StringList_free(files);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, ":");

          /* hash file */
	  rv=GWEN_MDigest__HashFile(md, GWEN_Buffer_GetStart(pbuf), tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(pbuf);
	    GWEN_StringList_free(files);
	    return rv;
	  }

	  /* append line to stringlist */
          GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(tbuf), 0, 0);
	  GWEN_Buffer_free(tbuf);
	}
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Unknown file type in [%s]", s);
      }
      GWEN_Buffer_Crop(pbuf, 0, ppos);
    }
    se=GWEN_StringListEntry_Next(se);
  }

  GWEN_Buffer_free(pbuf);
  GWEN_StringList_free(files);
  return 0;
}



int GWEN_MDigest_HashFileTree(GWEN_MDIGEST *md,
			      const char *folder,
			      const char *ignoreFile,
			      GWEN_STRINGLIST *sl) {
  int rv;

  rv=GWEN_MDigest__HashFileTree(md, folder, NULL, ignoreFile, sl);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_MDigest_CheckFileTree(GWEN_MDIGEST *md,
			       const char *folder,
			       const char *checksumFile,
			       int strictCheck,
			       uint32_t pid) {
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLIST *savedList;
  GWEN_BUFFER *tbuf;
  GWEN_STRINGLISTENTRY *se;
  int rv;
  int allHashesOk=1;
  int validLines=0;

  sl=GWEN_StringList_new();

  /* generate hash list */
  rv=GWEN_MDigest_HashFileTree(md, folder, checksumFile, sl);
  if (rv<0) {
    GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error,
			  I18N("Error unpacking program (%d)"), rv);
    GWEN_StringList_free(sl);
    return rv;
  }

  savedList=GWEN_StringList_new();

  /* read checksums from file */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, folder);
  GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendString(tbuf, checksumFile);
  rv=GWEN_SyncIo_Helper_ReadFileToStringList(GWEN_Buffer_GetStart(tbuf),
					     -1,
					     savedList);
  if (rv<0) {
    GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error,
			  I18N("Error loading checksum file (%d)"), rv);
    GWEN_Buffer_free(tbuf);
    GWEN_StringList_free(savedList);
    GWEN_StringList_free(sl);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  /* check checksums */
  se=GWEN_StringList_FirstEntry(savedList);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      validLines++;
      if (0==GWEN_StringList_RemoveString(sl, s)) {
	DBG_ERROR(0, "Hash not found: %s", s);
	allHashesOk=0;
      }
    }
    se=GWEN_StringListEntry_Next(se);
  }

  if (validLines==0) {
    GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error,
			  I18N("Checksum file does not contain valid lines"));
    GWEN_StringList_free(savedList);
    GWEN_StringList_free(sl);
    return GWEN_ERROR_VERIFY;
  }

  if (allHashesOk==0) {
    GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error,
			  I18N("Integrity check on folder failed"));
    GWEN_StringList_free(savedList);
    GWEN_StringList_free(sl);
    return GWEN_ERROR_VERIFY;
  }

  /* check for additional files */
  if (GWEN_StringList_Count(sl)) {
    if (strictCheck) {
      GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Error,
			    I18N("Folder contains %d files without checksum"),
			    GWEN_StringList_Count(sl));
      GWEN_StringList_free(savedList);
      GWEN_StringList_free(sl);
    }
    else
      GWEN_Gui_ProgressLog2(pid, GWEN_LoggerLevel_Warning,
			    I18N("Folder contains %d files without checksum"),
			    GWEN_StringList_Count(sl));
  }
  GWEN_StringList_free(savedList);
  GWEN_StringList_free(sl);

  return 0;
}






