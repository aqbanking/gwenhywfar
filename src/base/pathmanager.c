/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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


#include "pathmanager_p.h"
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef OS_WIN32
# include <windows.h>
# define DIRSEP "\\"
# define DIRSEP_C '\\'
#else
# define DIRSEP "/"
# define DIRSEP_C '/'
#endif


static GWEN_DB_NODE *gwen__paths=0;


int GWEN_PathManager_ModuleInit(){
  gwen__paths=GWEN_DB_Group_new("paths");

  return 0;
}



int GWEN_PathManager_ModuleFini(){
  GWEN_DB_Group_free(gwen__paths);
  gwen__paths=0;
  return 0;
}



int GWEN_PathManager_DefinePath(const char *destLib,
                                const char *pathName) {
  GWEN_DB_NODE *dbT;

  assert(destLib);
  assert(pathName);
  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_DB_FLAGS_DEFAULT,
                       destLib);
  assert(dbT);
  if (GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, pathName)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Path \"%s/%s\" already exists",
              destLib, pathName);
    return GWEN_ERROR_INVALID;
  }
  dbT=GWEN_DB_GetGroup(dbT,
                       GWEN_DB_FLAGS_DEFAULT,
                       pathName);
  return 0;
}



int GWEN_PathManager_UndefinePath(const char *destLib,
                                  const char *pathName) {
  GWEN_DB_NODE *dbT;

  assert(destLib);
  assert(pathName);
  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;

  dbT=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_DEFAULT,
                       pathName);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;
  GWEN_DB_UnlinkGroup(dbT);
  GWEN_DB_Group_free(dbT);
  return 0;
}




int GWEN_PathManager_AddPath(const char *callingLib,
                             const char *destLib,
                             const char *pathName,
			     const char *pathValue) {
  GWEN_DB_NODE *dbT;
  GWEN_BUFFER *buf;

  assert(destLib);
  assert(pathName);
  assert(pathValue);
  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;
  dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       pathName);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;
  dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_CREATE_GROUP,
                       "pair");
  assert(dbT);

  if (callingLib) {
    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                         "lib", callingLib);
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Directory_OsifyPath(pathValue, buf, 1);

  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
		       "path",
		       GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);

  return 0;
}



int GWEN_PathManager_AddRelPath(const char *callingLib,
				const char *destLib,
				const char *pathName,
				const char *pathValue,
				GWEN_PATHMANAGER_RELMODE rm) {
  char cwd[256];

  switch(rm) {
  case GWEN_PathManager_RelModeCwd: {
    const char *pcwd;

    pcwd=getcwd(cwd, sizeof(cwd)-1);
    if (pcwd) {
      GWEN_BUFFER *buf;
      int rv;

      buf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(buf, cwd);
      if (*pathValue!=DIRSEP_C)
	GWEN_Buffer_AppendString(buf, DIRSEP);
      GWEN_Buffer_AppendString(buf, pathValue);
      rv=GWEN_PathManager_AddPath(callingLib, destLib, pathName,
				  GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
      return rv;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "getcwd(): %s", strerror(errno));
      return GWEN_ERROR_IO;
    }
    break;
  }

  case GWEN_PathManager_RelModeExe: {
    int rv;

    rv=GWEN_Directory_GetPrefixDirectory(cwd, sizeof(cwd)-1);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else {
      GWEN_BUFFER *buf;

      buf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(buf, cwd);
      if (*pathValue!=DIRSEP_C)
	GWEN_Buffer_AppendString(buf, DIRSEP);
      GWEN_Buffer_AppendString(buf, pathValue);
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Adding path [%s]",
	       GWEN_Buffer_GetStart(buf));
      rv=GWEN_PathManager_AddPath(callingLib, destLib, pathName,
				  GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
      return rv;
    }
  }

  case GWEN_PathManager_RelModeHome: {
    GWEN_BUFFER *buf;
    int rv;

    rv=GWEN_Directory_GetHomeDirectory(cwd, sizeof(cwd)-1);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not determine HOME directory (%d)",
		rv);
      return rv;
    }
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(buf, cwd);
    if (*pathValue!=DIRSEP_C)
      GWEN_Buffer_AppendString(buf, DIRSEP);
    GWEN_Buffer_AppendString(buf, pathValue);
    rv=GWEN_PathManager_AddPath(callingLib, destLib, pathName,
				GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return rv;
  }

  default:
    DBG_INFO(GWEN_LOGDOMAIN, "Unknown relative mode %d", rm);
    return GWEN_ERROR_INVALID;
  }

}



int GWEN_PathManager_InsertPath(const char *callingLib,
                                const char *destLib,
                                const char *pathName,
                                const char *pathValue) {
  GWEN_DB_NODE *dbT;

  assert(destLib);
  assert(pathName);
  assert(pathValue);
  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;
  dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       pathName);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;

  dbT=GWEN_DB_GetGroup(dbT,
                       GWEN_PATH_FLAGS_CREATE_GROUP |
                       GWEN_DB_FLAGS_INSERT,
                       "pair");
  assert(dbT);

  if (callingLib) {
    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                         "lib", callingLib);
  }
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                       "path", pathValue);

  return 0;
}



int GWEN_PathManager_RemovePath(const char *callingLib,
                                const char *destLib,
                                const char *pathName,
                                const char *pathValue) {
  GWEN_DB_NODE *dbT;
  const char *s;
  const char *p;

  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;
  dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       pathName);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;

  dbT=GWEN_DB_FindFirstGroup(dbT, "pair");
  while(dbT) {
    p=GWEN_DB_GetCharValue(dbT, "path", 0, 0);
    assert(p);
    s=GWEN_DB_GetCharValue(dbT, "lib", 0, 0);

    if (
        (
         (!callingLib && !s) ||
         (callingLib && s && strcasecmp(s, callingLib)==0)
        ) &&
        strcasecmp(p, pathValue)==0
       )
      break;
    dbT=GWEN_DB_FindNextGroup(dbT, "pair");
  }

  if (dbT) {
    GWEN_DB_UnlinkGroup(dbT);
    GWEN_DB_Group_free(dbT);
    return 0;
  }
  else
    return 1;

}



int GWEN_PathManager_RemovePaths(const char *callingLib) {
  GWEN_DB_NODE *dbT;
  const char *s;

  assert(gwen__paths);
  GWEN_DB_DeleteGroup(gwen__paths, callingLib);

  dbT=GWEN_DB_GetFirstGroup(gwen__paths);
  while(dbT) {
    GWEN_DB_NODE *dbN;

    dbN=GWEN_DB_GetFirstGroup(dbT);
    while(dbN) {
      GWEN_DB_NODE *dbNN;

      dbNN=GWEN_DB_FindFirstGroup(dbN, "pair");
      while(dbNN) {
        GWEN_DB_NODE *dbNext;

        dbNext=GWEN_DB_FindNextGroup(dbNN, "pair");
        s=GWEN_DB_GetCharValue(dbNN, "lib", 0, 0);
        assert(s);

        if (s && strcasecmp(s, callingLib)==0) {
          GWEN_DB_UnlinkGroup(dbNN);
          GWEN_DB_Group_free(dbNN);
        }
        dbNN=dbNext;
      } /* while pairs */
      dbN=GWEN_DB_GetNextGroup(dbN);
    } /* while paths */
    dbT=GWEN_DB_GetNextGroup(dbT);
  } /* while destLibs */

  return 0;
}



int GWEN_PathManager_PathChanged(const char *destLib,
                                 const char *pathName) {
  GWEN_DB_NODE *dbT;

  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;

  dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       pathName);
  if (!dbT)
    return GWEN_ERROR_NOT_FOUND;

  if ((GWEN_DB_GetNodeFlags(dbT) & GWEN_DB_NODE_FLAGS_DIRTY))
    return 1;

  return 0;
}



GWEN_STRINGLIST *GWEN_PathManager_GetPaths(const char *destLib,
                                           const char *pathName) {
  GWEN_DB_NODE *dbT;

  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (dbT) {
    dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         pathName);
    if (dbT) {
      GWEN_STRINGLIST *sl;
      int i;
      const char *s;
      GWEN_DB_NODE *dbN;

      sl=GWEN_StringList_new();

      /* then add all paths from other libs */
      dbN=GWEN_DB_FindFirstGroup(dbT, "pair");
      while(dbN) {
        for (i=0; ; i++) {
          s=GWEN_DB_GetCharValue(dbN, "path", i, 0);
          if (!s)
            break;
          GWEN_StringList_AppendString(sl, s, 0, 1);
        }

        dbN=GWEN_DB_FindNextGroup(dbN, "pair");
      }

      if (GWEN_StringList_Count(sl)==0) {
        GWEN_StringList_free(sl);
        DBG_DEBUG(GWEN_LOGDOMAIN, "no entries");
        return 0;
      }

      return sl;
    }
  }

  return 0;
}



int GWEN_PathManager_FindFile(const char *destLib,
			      const char *pathName,
			      const char *fileName,
			      GWEN_BUFFER *fbuf) {
  GWEN_DB_NODE *dbT;

  assert(gwen__paths);
  dbT=GWEN_DB_GetGroup(gwen__paths, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       destLib);
  if (dbT) {
    dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         pathName);
    if (dbT) {
      int i;
      const char *s;
      GWEN_DB_NODE *dbN;
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);

      /* check all paths */
      dbN=GWEN_DB_FindFirstGroup(dbT, "pair");
      while(dbN) {
        for (i=0; ; i++) {
          s=GWEN_DB_GetCharValue(dbN, "path", i, 0);
          if (!s)
	    break;
	  else {
	    FILE *f;
	
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, DIRSEP);
	    GWEN_Buffer_AppendString(tbuf, fileName);
	    DBG_DEBUG(GWEN_LOGDOMAIN, "Trying \"%s\"",
		      GWEN_Buffer_GetStart(tbuf));
	    f=fopen(GWEN_Buffer_GetStart(tbuf), "r");
	    if (f) {
	      fclose(f);
	      DBG_DEBUG(GWEN_LOGDOMAIN,
			"File \"%s\" found in folder \"%s\"",
			fileName,
			s);
	      GWEN_Buffer_AppendBuffer(fbuf, tbuf);
	      GWEN_Buffer_free(tbuf);
	      return 0;
	    }
	    GWEN_Buffer_Reset(tbuf);
	  }
	}

	dbN=GWEN_DB_FindNextGroup(dbN, "pair");
      }
      GWEN_Buffer_free(tbuf);
    }
  }

  DBG_INFO(GWEN_LOGDOMAIN, "File \"%s\" not found", fileName);
  return GWEN_ERROR_NOT_FOUND;
}





#ifdef OS_WIN32
int GWEN_PathManager_AddPathFromWinReg(const char *callingLib,
                                       const char *destLib,
                                       const char *pathName,
                                       const char *keypath,
                                       const char *varname){
  HKEY hkey;
  TCHAR nbuffer[MAX_PATH];
  BYTE vbuffer[MAX_PATH];
  DWORD nsize;
  DWORD vsize;
  DWORD typ;
  int i;

  snprintf(nbuffer, sizeof(nbuffer), keypath);

  /* open the key */
  if (RegOpenKey(HKEY_LOCAL_MACHINE, nbuffer, &hkey)){
    DBG_INFO(GWEN_LOGDOMAIN, "RegOpenKey %s failed.", keypath);
    return 1;
  }

  /* find the variablename  */
  for (i=0;; i++) {
    nsize=sizeof(nbuffer);
    vsize=sizeof(vbuffer);
    if (ERROR_SUCCESS!=RegEnumValue(hkey,
                                    i,    /* index */
                                    nbuffer,
                                    &nsize,
                                    0,       /* reserved */
                                    &typ,
                                    vbuffer,
                                    &vsize))
      break;
    if (strcasecmp(nbuffer, varname)==0 && typ==REG_SZ) {
      /* variable found */
      RegCloseKey(hkey);
      return GWEN_PathManager_AddPath(callingLib,
                                      destLib,
                                      pathName,
                                      (char*)vbuffer);
    }
  } /* for */

  RegCloseKey(hkey);
  DBG_INFO(GWEN_LOGDOMAIN,
	   "In RegKey \"%s\" the variable \"%s\" does not exist",
	   keypath, varname);
  return 1;
}

#else /* OS_WIN32 */


int GWEN_PathManager_AddPathFromWinReg(GWEN_UNUSED const char *callingLib,
				       GWEN_UNUSED const char *destLib,
                                       GWEN_UNUSED const char *pathName,
                                       GWEN_UNUSED const char *keypath,
				       GWEN_UNUSED const char *varname){
  return 0;
}

#endif /* OS_WIN32 */


