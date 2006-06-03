/***************************************************************************
 $RCSfile: account.h,v $
 -------------------
 cvs         : $Id: account.h,v 1.16 2006/02/22 19:38:54 aquamaniac Exp $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "lf_storage_p.h"
#include "lf_type_l.h"
#include "lf_object_l.h"
#include <gwenhywfar/smp_storage_be.h>
#include <gwenhywfar/smp_type_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



GWEN_INHERIT(GWEN_STO_STORAGE, LOCALFILES_STORAGE)




GWEN_STO_STORAGE *LocalFilesStorage_new(const char *address) {
  GWEN_STO_STORAGE *st;
  LOCALFILES_STORAGE *xst;

  assert(address);
  st=GWEN_SmpStoStorage_new("localfiles", address);
  GWEN_NEW_OBJECT(LOCALFILES_STORAGE, xst);

  GWEN_INHERIT_SETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st, xst,
                       LocalFilesStorage_FreeData);

  /* set virtual functions */
  GWEN_SmpSto_SetNextUniqueIdFn(st, LocalFiles_NextUniqueId);
  GWEN_SmpSto_SetCreateDbFn(st, LocalFiles_CreateDb);
  GWEN_SmpSto_SetLoadDbFn(st, LocalFiles_LoadDb);
  GWEN_SmpSto_SetCreateTypeFn(st, LocalFiles_CreateType);
  GWEN_SmpSto_SetWriteTypeFn(st, LocalFiles_WriteType);
  GWEN_SmpSto_SetDupTypeFn(st, LocalFiles_DupType);
  GWEN_SmpSto_SetReadObjectFn(st, LocalFiles_ReadObject);
  GWEN_SmpSto_SetWriteObjectFn(st, LocalFiles_WriteObject);
  GWEN_SmpSto_SetCreateObjectFn(st, LocalFiles_CreateObject);
  GWEN_SmpSto_SetDeleteObjectFn(st, LocalFiles_DeleteObject);

  GWEN_SmpSto_SetAddLogFn(st, LocalFiles_AddLog);

  return st;
}



void LocalFilesStorage_FreeData(void *bp, void *p) {
  LOCALFILES_STORAGE *xst;

  xst=(LOCALFILES_STORAGE*) p;
  GWEN_FREE_OBJECT(xst);
}



int LocalFiles_NextUniqueId(GWEN_STO_STORAGE *st,
                            GWEN_STO_TYPE *ty,
                            GWEN_TYPE_UINT32 *pid) {
  LOCALFILES_STORAGE *xst;
  FILE *f;
  unsigned int lastId=0;
  int existed=1;
  GWEN_BUFFER *fbuf;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  if (ty) {
    if (GWEN_StoType_GetId(ty) & 0xffffff00) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Type id is too high, too many types? (%x [%s/%s])",
                GWEN_StoType_GetId(ty),
                GWEN_StoType_GetTypeName(ty),
                GWEN_StoType_GetName(ty));
      return GWEN_ERROR_IO;
    }
  }

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ty)
    GWEN_Buffer_AppendString(fbuf, LocalFilesType_GetBaseFolder(ty));
  else
    GWEN_Buffer_AppendString(fbuf, GWEN_StoStorage_GetAddress(st));
  GWEN_Buffer_AppendString(fbuf, DIRSEP "lastid");

  f=fopen(GWEN_Buffer_GetStart(fbuf), "r+");
  if (!f) {
    if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(fbuf),
                               GWEN_PATH_FLAGS_CHECKROOT |
                               GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST |
                               GWEN_PATH_FLAGS_VARIABLE)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not create file \"%s\"",
                GWEN_Buffer_GetStart(fbuf));
      GWEN_Buffer_free(fbuf);
      return GWEN_ERROR_IO;
    }

    f=fopen(GWEN_Buffer_GetStart(fbuf), "w+");
    existed=0;
  }
  if (f) {
    GWEN_FSLOCK *lck;
    GWEN_FSLOCK_RESULT res;
    int rv;

    lck=GWEN_FSLock_new(GWEN_Buffer_GetStart(fbuf), GWEN_FSLock_TypeFile);
    res=GWEN_FSLock_Lock(lck, 10);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not lock lastid file (%d)", res);
      GWEN_FSLock_free(lck);
      fclose(f);
      GWEN_Buffer_free(fbuf);
      return GWEN_ERROR_IO;
    }
    if (existed) {
      if (1!=fscanf(f, "%08x", &lastId)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unable to read id from lastid file");
        GWEN_FSLock_Unlock(lck);
        GWEN_FSLock_free(lck);
        fclose(f);
        GWEN_Buffer_free(fbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      rewind(f);
    }
    lastId++;
    fprintf(f, "%08x\n", lastId);
    if (fclose(f)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s",
                GWEN_Buffer_GetStart(fbuf),
                strerror(errno));
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
      fclose(f);
      GWEN_Buffer_free(fbuf);
      return GWEN_ERROR_IO;
    }
    rv=GWEN_FSLock_Unlock(lck);
    if (rv) {
      DBG_WARN(GWEN_LOGDOMAIN, "Error removing lock (%d)", rv);
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s",
              GWEN_Buffer_GetStart(fbuf),
              strerror(errno));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }
  GWEN_Buffer_free(fbuf);

  if (ty) {
    if (lastId & 0xff000000) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Too many ids for type %x [%s/%s]",
                GWEN_StoType_GetId(ty),
                GWEN_StoType_GetTypeName(ty),
                GWEN_StoType_GetName(ty));
      return GWEN_ERROR_IO;
    }

    lastId&=0x00ffffff;
    lastId|=(GWEN_StoType_GetId(ty) & 0xff)<<24;
  }
  else {
    if (lastId & 0xffffff00) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Id is too high, too many types?");
      return GWEN_ERROR_IO;
    }
  }

  *pid=lastId;

  return 0;
}



int LocalFiles_CreateDb(GWEN_STO_STORAGE *st) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, GWEN_StoStorage_GetAddress(st));

  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(fbuf),
                             GWEN_PATH_FLAGS_CHECKROOT |
                             GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not create folder \"%s\"",
              GWEN_Buffer_GetStart(fbuf));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }
  GWEN_Buffer_free(fbuf);

  return 0;
}





int LocalFiles__ScanObject_Files(GWEN_STO_STORAGE *st,
                                 GWEN_STO_TYPE *ty,
                                 const char *folder,
                                 GWEN_TYPE_UINT32 idSoFar) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  GWEN_DIRECTORYDATA *d;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, folder);

  d=GWEN_Directory_new();
  if (!GWEN_Directory_Open(d, GWEN_Buffer_GetStart(fbuf))) {
    GWEN_TYPE_UINT32 pos;
    char nbuf[256];

    GWEN_Buffer_AppendString(fbuf, DIRSEP);
    pos=GWEN_Buffer_GetPos(fbuf);
    while(!GWEN_Directory_Read(d, nbuf, sizeof(nbuf))) {
      if (nbuf[0]!='.') {
        struct stat s;

        GWEN_Buffer_AppendString(fbuf, nbuf);
        if (stat(GWEN_Buffer_GetStart(fbuf), &s)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "stat(%s): %s",
                    GWEN_Buffer_GetStart(fbuf), strerror(errno));
        }
        else {
          if (!S_ISDIR(s.st_mode)) {
            int l;

            l=strlen(nbuf);
            if (l==6) {
              if (strcasecmp(nbuf+2, ".gob")==0) {
                unsigned int by;

                nbuf[2]=0;
                if (1==sscanf(nbuf, "%02x", &by)) {
                  GWEN_TYPE_UINT32 id;

                  id=(idSoFar<<8)+by;

                  GWEN_IdList_AddId(GWEN_SmpStoType_GetObjectIdList(ty), id);
                }
              }
            }
          }
        }
      } /* if normal file name */
      GWEN_Buffer_Crop(fbuf, 0, pos);
    } /* while */
    GWEN_Directory_Close(d);
  }
  GWEN_Directory_free(d);
  GWEN_Buffer_free(fbuf);

  return 0;
}




int LocalFiles__ScanObject_Folders(GWEN_STO_STORAGE *st,
                                   GWEN_STO_TYPE *ty,
                                   const char *folder,
                                   GWEN_TYPE_UINT32 idSoFar,
                                   int level) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  GWEN_DIRECTORYDATA *d;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, folder);

  d=GWEN_Directory_new();
  if (!GWEN_Directory_Open(d, GWEN_Buffer_GetStart(fbuf))) {
    GWEN_TYPE_UINT32 pos;
    char nbuf[256];

    GWEN_Buffer_AppendString(fbuf, DIRSEP);
    pos=GWEN_Buffer_GetPos(fbuf);
    while(!GWEN_Directory_Read(d, nbuf, sizeof(nbuf))) {
      if (nbuf[0]!='.') {
        struct stat s;

        GWEN_Buffer_AppendString(fbuf, nbuf);
        if (stat(GWEN_Buffer_GetStart(fbuf), &s)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "stat(%s): %s",
                    GWEN_Buffer_GetStart(fbuf), strerror(errno));
        }
        else {
          if (S_ISDIR(s.st_mode) && strlen(nbuf)==2) {
            unsigned int by;

            if (1==sscanf(nbuf, "%02x", &by)) {
              GWEN_TYPE_UINT32 id;

              id=(idSoFar<<8)+by;

              if (level>0) {
                rv=LocalFiles__ScanObject_Folders(st, ty,
                                                  GWEN_Buffer_GetStart(fbuf),
                                                  id,
                                                  level-1);
                if (rv) {
                  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
                }
              }
              else {
                rv=LocalFiles__ScanObject_Files(st, ty,
                                                GWEN_Buffer_GetStart(fbuf),
                                                id);
                if (rv) {
                  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
                }
              }
            }
          }
        }
      } /* if normal file name */
      GWEN_Buffer_Crop(fbuf, 0, pos);
    } /* while */
    GWEN_Directory_Close(d);
  }
  GWEN_Directory_free(d);
  GWEN_Buffer_free(fbuf);

  return 0;
}



int LocalFiles__ScanTypeObjects(GWEN_STO_STORAGE *st, GWEN_STO_TYPE *ty) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, LocalFilesType_GetBaseFolder(ty));

  rv=LocalFiles__ScanObject_Folders(st, ty,
                                    GWEN_Buffer_GetStart(fbuf),
                                    0,  /* idSoFa */
                                    2); /* levels */
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Buffer_free(fbuf);

  return rv;
}







int LocalFiles__LoadType(GWEN_STO_STORAGE *st,
                         const char *typeFolder) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  GWEN_DIRECTORYDATA *d;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, typeFolder);

  d=GWEN_Directory_new();
  if (!GWEN_Directory_Open(d, GWEN_Buffer_GetStart(fbuf))) {
    GWEN_TYPE_UINT32 pos;
    char nbuf[256];

    GWEN_Buffer_AppendString(fbuf, DIRSEP);
    pos=GWEN_Buffer_GetPos(fbuf);
    while(!GWEN_Directory_Read(d, nbuf, sizeof(nbuf))) {
      if (nbuf[0]!='.') {
        struct stat s;
        GWEN_TYPE_UINT32 pos2;

        GWEN_Buffer_AppendString(fbuf, nbuf);
        pos2=GWEN_Buffer_GetPos(fbuf);
        if (stat(GWEN_Buffer_GetStart(fbuf), &s)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "stat(%s): %s",
                    GWEN_Buffer_GetStart(fbuf), strerror(errno));
        }
        else {
          if (S_ISDIR(s.st_mode)) {
            GWEN_DB_NODE *db;

            GWEN_Buffer_AppendString(fbuf, DIRSEP "settings.conf");
            db=GWEN_DB_Group_new("type");
            if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(fbuf),
                                 GWEN_DB_FLAGS_DEFAULT |
                                 GWEN_PATH_FLAGS_CREATE_GROUP)) {
              DBG_INFO(GWEN_LOGDOMAIN,
                       "Could not read file [%s]",
                       GWEN_Buffer_GetStart(fbuf));
            }
            else {
              GWEN_STO_TYPE *ty;

              GWEN_Buffer_Crop(fbuf, 0, pos2);
              ty=LocalFilesType_fromDb(st, db, GWEN_Buffer_GetStart(fbuf));
              if (ty) {
                int rv;

                DBG_NOTICE(GWEN_LOGDOMAIN,
                           "Loaded type %x [%s/%s]",
                           GWEN_StoType_GetId(ty),
                           GWEN_StoType_GetTypeName(ty),
                           GWEN_StoType_GetName(ty));
                GWEN_StoStorage_AddType(st, ty);
                rv=LocalFiles__ScanTypeObjects(st, ty);
                if (rv) {
                  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
                }
              }
            }
          }
        }
      } /* if normal file name */
      GWEN_Buffer_Crop(fbuf, 0, pos);
    } /* while */
    GWEN_Directory_Close(d);
  }
  GWEN_Directory_free(d);
  GWEN_Buffer_free(fbuf);

  return 0;
}





int LocalFiles__LoadTypes(GWEN_STO_STORAGE *st) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  GWEN_DIRECTORYDATA *d;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, GWEN_StoStorage_GetAddress(st));

  d=GWEN_Directory_new();
  if (!GWEN_Directory_Open(d, GWEN_Buffer_GetStart(fbuf))) {
    GWEN_TYPE_UINT32 pos;
    char nbuf[256];

    GWEN_Buffer_AppendString(fbuf, DIRSEP);
    pos=GWEN_Buffer_GetPos(fbuf);
    while(!GWEN_Directory_Read(d, nbuf, sizeof(nbuf))) {
      if (nbuf[0]!='.') {
        struct stat s;

        GWEN_Buffer_AppendString(fbuf, nbuf);
        if (stat(GWEN_Buffer_GetStart(fbuf), &s)) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "stat(%s): %s",
                    GWEN_Buffer_GetStart(fbuf), strerror(errno));
        }
        else {
          if (S_ISDIR(s.st_mode)) {
            rv=LocalFiles__LoadType(st, GWEN_Buffer_GetStart(fbuf));
            if (rv) {
              DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            }
          }
        }
      } /* if normal file name */
      GWEN_Buffer_Crop(fbuf, 0, pos);
    } /* while */
    GWEN_Directory_Close(d);
  }
  GWEN_Directory_free(d);
  GWEN_Buffer_free(fbuf);

  return 0;
}






int LocalFiles_LoadDb(GWEN_STO_STORAGE *st) {
  int rv;

  rv=LocalFiles__LoadTypes(st);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  DBG_NOTICE(GWEN_LOGDOMAIN, "Database loaded.");
  return 0;
}



int LocalFiles_WriteType(GWEN_STO_STORAGE *st,
                         GWEN_STO_TYPE *ty) {
  LOCALFILES_STORAGE *xst;
  GWEN_DB_NODE *db;
  GWEN_BUFFER *fbuf;
  char *realFileName;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  db=GWEN_DB_Group_new("type");
  LocalFilesType_toDb(ty, db);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, LocalFilesType_GetBaseFolder(ty));
  GWEN_Buffer_AppendString(fbuf, DIRSEP "settings.conf");
  realFileName=strdup(GWEN_Buffer_GetStart(fbuf));
  GWEN_Buffer_AppendString(fbuf, ".tmp");

  /* write type to temporary file */
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(fbuf),
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error writing type file");
    free(realFileName);
    GWEN_Buffer_free(fbuf);
    GWEN_DB_Group_free(db);
    return GWEN_ERROR_IO;
  }

  /* rename to final filename */
#ifdef OS_WIN32
  unlink(realFileName);
#endif
  if (rename(GWEN_Buffer_GetStart(fbuf), realFileName)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "rename(%s): %s",
              GWEN_Buffer_GetStart(fbuf),
              strerror(errno));
    free(realFileName);
    GWEN_Buffer_free(fbuf);
    GWEN_DB_Group_free(db);
    return GWEN_ERROR_IO;
  }

  free(realFileName);
  GWEN_Buffer_free(fbuf);
  GWEN_DB_Group_free(db);

  return 0;
}



int LocalFiles_CreateType(GWEN_STO_STORAGE *st,
                          const char *typeName,
                          const char *name,
                          GWEN_STO_TYPE **pty) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  int rv;
  GWEN_STO_TYPE *ty;
  GWEN_TYPE_UINT32 id;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, GWEN_StoStorage_GetAddress(st));
  GWEN_Buffer_AppendString(fbuf, DIRSEP);
  GWEN_Text_EscapeToBuffer(typeName, fbuf);
  GWEN_Buffer_AppendString(fbuf, DIRSEP);
  if (name && *name)
    GWEN_Text_EscapeToBuffer(name, fbuf);
  else
    GWEN_Buffer_AppendString(fbuf, "unnamed");

  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(fbuf),
                             GWEN_PATH_FLAGS_CHECKROOT |
                             GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not create folder \"%s\"",
              GWEN_Buffer_GetStart(fbuf));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }

  rv=GWEN_SmpSto_NextUniqueId(st, 0, &id);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(fbuf);
    return rv;
  }

  ty=LocalFilesType_new(st, id, typeName, name,
                        GWEN_Buffer_GetStart(fbuf));
  *pty=ty;

  GWEN_Buffer_free(fbuf);
  return 0;
}



int LocalFiles_DupType(GWEN_STO_STORAGE *st,
                       const GWEN_STO_TYPE *ty,
                       GWEN_STO_TYPE **pty) {
  LOCALFILES_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  *pty=LocalFilesType_dup(ty);
  return 0;
}



void LocalFiles__GetObjectPath(GWEN_STO_STORAGE *st,
                               GWEN_STO_TYPE *ty,
                               GWEN_TYPE_UINT32 id,
                               GWEN_BUFFER *fbuf) {
  char numbuf[10];

  GWEN_Buffer_AppendString(fbuf, LocalFilesType_GetBaseFolder(ty));
  GWEN_Buffer_AppendString(fbuf, DIRSEP);

  snprintf(numbuf, sizeof(numbuf), "%08x", id);
  assert(strlen(numbuf)==8);
  GWEN_Buffer_AppendBytes(fbuf, numbuf, 2);
  GWEN_Buffer_AppendString(fbuf, DIRSEP);
  GWEN_Buffer_AppendBytes(fbuf, numbuf+2, 2);
  GWEN_Buffer_AppendString(fbuf, DIRSEP);
  GWEN_Buffer_AppendBytes(fbuf, numbuf+4, 2);
  GWEN_Buffer_AppendString(fbuf, DIRSEP);
  GWEN_Buffer_AppendBytes(fbuf, numbuf+6, 2);
  GWEN_Buffer_AppendString(fbuf, ".gob");
}



int LocalFiles_ReadObject(GWEN_STO_STORAGE *st,
                          GWEN_STO_TYPE *ty,
                          GWEN_TYPE_UINT32 id,
                          GWEN_STO_OBJECT **po) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbMeta;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  LocalFiles__GetObjectPath(st, ty, id, fbuf);

  db=GWEN_DB_Group_new("object");
  if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(fbuf),
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Object %x not found", id);
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_NOT_FOUND;
  }

  dbMeta=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "meta");
  dbData=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data");
  *po=LocalFilesObject_fromDb(ty, id, dbData, GWEN_Buffer_GetStart(fbuf));
  GWEN_StoObject_SetRefCount(*po,
                             GWEN_DB_GetIntValue(dbMeta, "refCount", 0, 0));
  GWEN_DB_Group_free(db);
  GWEN_Buffer_free(fbuf);
  return 0;
}



int LocalFiles_WriteObject(GWEN_STO_STORAGE *st,
                           GWEN_STO_OBJECT *o) {
  LOCALFILES_STORAGE *xst;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbMeta;
  GWEN_BUFFER *fbuf;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, LocalFilesObject_GetFileName(o));
  GWEN_Buffer_AppendString(fbuf, ".tmp");

  /* check for path of the file. This will create all necessary folders */
  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(fbuf),
                             GWEN_PATH_FLAGS_CHECKROOT |
                             GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST |
                             GWEN_PATH_FLAGS_VARIABLE |
                             GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not create file \"%s\"",
              GWEN_Buffer_GetStart(fbuf));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }

  /* store object in DB */
  db=GWEN_DB_Group_new("object");
  dbMeta=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "meta");
  GWEN_DB_SetIntValue(dbMeta, GWEN_DB_FLAGS_DEFAULT,
                      "refCount", GWEN_StoObject_GetRefCount(o));
  dbData=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "data");
  LocalFilesObject_toDb(o, dbData);

  /* write to temporary file */
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(fbuf),
                        GWEN_DB_FLAGS_DEFAULT|
                        GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not write object %x",
              GWEN_StoObject_GetId(o));
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }
  GWEN_DB_Group_free(db);

  /* rename to final filename */
#ifdef OS_WIN32
  unlink(LocalFilesObject_GetFileName(o));
#endif
  if (rename(GWEN_Buffer_GetStart(fbuf), LocalFilesObject_GetFileName(o))) {
    DBG_ERROR(GWEN_LOGDOMAIN, "rename(%s): %s",
              GWEN_Buffer_GetStart(fbuf),
              strerror(errno));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }

  GWEN_Buffer_free(fbuf);

  return 0;
}



int LocalFiles_CreateObject(GWEN_STO_STORAGE *st,
                            GWEN_STO_TYPE *ty,
                            GWEN_STO_OBJECT **po) {
  LOCALFILES_STORAGE *xst;
  GWEN_BUFFER *fbuf;
  GWEN_TYPE_UINT32 id;
  int rv;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  rv=GWEN_SmpSto_NextUniqueId(st, ty, &id);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  LocalFiles__GetObjectPath(st, ty, id, fbuf);

  *po=LocalFilesObject_new(ty, id, GWEN_Buffer_GetStart(fbuf));
  GWEN_Buffer_free(fbuf);
  return 0;
}



int LocalFiles_DeleteObject(GWEN_STO_STORAGE *st,
                            GWEN_STO_OBJECT *o) {
  LOCALFILES_STORAGE *xst;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  if (unlink(LocalFilesObject_GetFileName(o))) {
    DBG_ERROR(GWEN_LOGDOMAIN, "unlink(%s): %s",
              LocalFilesObject_GetFileName(o),
              strerror(errno));
    return GWEN_ERROR_IO;
  }
  return 0;
}



int LocalFiles_AddLog(GWEN_STO_STORAGE *st, const GWEN_STO_LOG *log) {
  LOCALFILES_STORAGE *xst;
  FILE *f;
  GWEN_BUFFER *fbuf;
  GWEN_FSLOCK *lck;
  GWEN_FSLOCK_RESULT res;
  const char *s;
  int rv;
  GWEN_BUFFER *lbuf;
  char numbuf[32];
  GWEN_TYPE_UINT32 id;

  assert(st);
  xst=GWEN_INHERIT_GETDATA(GWEN_STO_STORAGE, LOCALFILES_STORAGE, st);
  assert(xst);

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, GWEN_StoStorage_GetAddress(st));
  GWEN_Buffer_AppendString(fbuf, DIRSEP "log");

  f=fopen(GWEN_Buffer_GetStart(fbuf), "a+");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not create file \"%s\"",
              GWEN_Buffer_GetStart(fbuf));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }

  lck=GWEN_FSLock_new(GWEN_Buffer_GetStart(fbuf), GWEN_FSLock_TypeFile);
  res=GWEN_FSLock_Lock(lck, 10);
  if (res!=GWEN_FSLock_ResultOk) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not lock lastid file (%d)", res);
    GWEN_FSLock_free(lck);
    fclose(f);
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }

  lbuf=GWEN_Buffer_new(0, 512, 0, 1);
#define ADDSTRING(m)                           \
  s=GWEN_StoLog_Get##m(log);                   \
  if (s)                                       \
    LocalFiles__EscapeToBufferVeryTolerant(s, lbuf); \
  GWEN_Buffer_AppendByte(lbuf, '\t');

  ADDSTRING(UserName);
  s=GWEN_StoLog_Action_toString(GWEN_StoLog_GetLogAction(log));
  LocalFiles__EscapeToBufferVeryTolerant(s, lbuf);
  GWEN_Buffer_AppendByte(lbuf, '\t');
  ADDSTRING(TypeBaseName);
  ADDSTRING(TypeName);
  id=GWEN_StoLog_GetObjectId(log);
  if (id) {
    snprintf(numbuf, sizeof(numbuf), "%x", id);
    LocalFiles__EscapeToBufferVeryTolerant(numbuf, lbuf);
  }
  GWEN_Buffer_AppendByte(lbuf, '\t');
  ADDSTRING(Param1);
  ADDSTRING(Param2);
  ADDSTRING(Param3);
#undef ADDSTRING

  fprintf(f, "%s\n", GWEN_Buffer_GetStart(lbuf));
  GWEN_Buffer_free(lbuf);

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s",
              GWEN_Buffer_GetStart(fbuf),
              strerror(errno));
    GWEN_FSLock_Unlock(lck);
    GWEN_FSLock_free(lck);
    fclose(f);
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_IO;
  }
  rv=GWEN_FSLock_Unlock(lck);
  if (rv) {
    DBG_WARN(GWEN_LOGDOMAIN, "Error removing lock (%d)", rv);
  }
  GWEN_Buffer_free(fbuf);

  return 0;
}




int LocalFiles__EscapeToBufferVeryTolerant(const char *src, GWEN_BUFFER *buf){
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!(
          (x>='A' && x<='Z') ||
          (x>='a' && x<='z') ||
          (x>='0' && x<='9') ||
          strchr(" .,*?/:-_()[]{}=!&#'~", x))) {
      unsigned char c;

      GWEN_Buffer_AppendByte(buf, '%');
      c=(((unsigned char)(*src))>>4)&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
      c=((unsigned char)(*src))&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
    }
    else
      GWEN_Buffer_AppendByte(buf, *src);

    src++;
  } /* while */

  return 0;
}






