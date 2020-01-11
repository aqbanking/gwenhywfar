/***************************************************************************
 begin       : Thu Oct 30 2003
 copyright   : (C) 2003-2013 by Martin Preuss
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

/* disable DBG_DEBUG() and DBG_VERBOUS() */
#define DISABLE_DEBUGLOG

#include "csv_p.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/dbio_be.h>
#include <gwenhywfar/syncio_file.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int GWEN_CSV_GetNameAndIndex(const char *name,
                             char *buffer,
                             unsigned int size)
{
  unsigned int i;
  int rv;

  i=0;
  rv=0;
  /* read and copy name */
  while (name[i] && name[i]!='[' && i<size) {
    buffer[i]=name[i];
    i++;
  } /* while */

  if (i>=size) {
    DBG_INFO(0, "Name too long (%d>=%d)", i, size);
    return -1;
  }
  buffer[i]=0;

  /* read and copy index, if any */
  if (name[i]=='[') {
    char numbuffer[16];
    unsigned int j;

    j=0;
    i++;
    while (name[i] && name[i]!=']' && j<sizeof(numbuffer)) {
      numbuffer[j]=name[i];
      i++;
      j++;
    } /* while */
    if (j>=sizeof(numbuffer)) {
      DBG_INFO(0, "Index number too long (%u>=%d)", j,
               (int)(sizeof(numbuffer)));
      return -1;
    }
    numbuffer[j]=0;
    rv=atoi(numbuffer);
  }

  return rv;
}



int GWEN_DBIO_CSV_Export(GWEN_DBIO *dbio,
                         GWEN_SYNCIO *sio,
                         GWEN_DB_NODE *data,
                         GWEN_DB_NODE *cfg,
                         GWEN_UNUSED uint32_t flags)
{
  GWEN_DB_NODE *colgr;
  GWEN_DB_NODE *n;
  int delimiter;
  int quote;
  const char *p;
  const char *groupName;
  int err;
  unsigned int column;
  int title;
  GWEN_FAST_BUFFER *fb;

  assert(dbio);
  assert(sio);
  assert(cfg);
  assert(data);

  fb=GWEN_FastBuffer_new(512, sio);

  /* get general configuration */
  colgr=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "columns");
  if (!colgr) {
    DBG_ERROR(0, "Error in configuration: No columns specified");
    GWEN_FastBuffer_free(fb);
    return GWEN_ERROR_INVALID;
  }
  p=GWEN_DB_GetCharValue(cfg, "delimiter", 0, ";");
  if (strcasecmp(p, "TAB")==0)
    delimiter=9;
  else if (strcasecmp(p, "SPACE")==0)
    delimiter=32;
  else
    delimiter=p[0];
  quote=GWEN_DB_GetIntValue(cfg, "quote", 0, 1);
  groupName=GWEN_DB_GetCharValue(cfg, "group", 0, "");
  title=GWEN_DB_GetIntValue(cfg, "title", 0, 1);

  if (title) {
    /* write title */
    for (column=1; ; column++) {
      int idx;
      char namebuffer[64];
      char numbuffer[16];
      char *np;

      /* create name for column */
      GWEN_Text_NumToString(column, numbuffer, sizeof(numbuffer), 0);
      p=GWEN_DB_GetCharValue(colgr, numbuffer, 0, 0);
      if (!p) {
        /* no value. finished */
        GWEN_FASTBUFFER_WRITELINE(fb, err, "");
        if (err<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
          GWEN_FastBuffer_free(fb);
          return err;
        }
        DBG_VERBOUS(GWEN_LOGDOMAIN, "No colums left, line finished");
        break;
      }
      /* break down to name and index */
      idx=GWEN_CSV_GetNameAndIndex(p, namebuffer, sizeof(namebuffer));
      if (idx==-1) {
        DBG_INFO(0, "Error in configuration: Bad name for column %d",
                 column);
        GWEN_FastBuffer_free(fb);
        return GWEN_ERROR_GENERIC;
      }

      /* add idx to name, if not 0 */
      if (idx) {
        GWEN_Text_NumToString(idx, numbuffer, sizeof(numbuffer), 0);
        if (strlen(namebuffer)+strlen(numbuffer)+1>=sizeof(namebuffer)) {
          DBG_ERROR(0, "Internal: namebuffer too small");
          GWEN_FastBuffer_free(fb);
          return -1;
        }
        strcat(namebuffer, numbuffer);
      }
      /* convert slashes to underscores */
      np=namebuffer;
      while (*np) {
        if (*np=='/')
          *np='_';
        np++;
      }

      if (column!=1) {
        /* write delimiter */
        GWEN_FASTBUFFER_WRITEBYTE(fb, err, delimiter);
        if (err<0) {
          DBG_INFO(0, "Called from here");
          GWEN_FastBuffer_free(fb);
          return err;
        }
      } /* if not first column */
      if (quote) {
        /* write quotation mark */
        GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
        if (err<0) {
          DBG_INFO(0, "Called from here");
          GWEN_FastBuffer_free(fb);
          return err;
        }
      } /* if quote */
      /* write value */
      GWEN_FASTBUFFER_WRITEFORCED(fb, err, namebuffer, -1);
      if (err<0) {
        DBG_INFO(0, "Called from here");
        GWEN_FastBuffer_free(fb);
        return err;
      }
      if (quote) {
        /* write quotation mark */
        GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
        if (err<0) {
          DBG_INFO(0, "Called from here");
          GWEN_FastBuffer_free(fb);
          return err;
        }
      } /* if quote */
    } /* for */
  } /* if title */

  n=GWEN_DB_GetFirstGroup(data);
  while (n) {
    if (*groupName==0 || strcasecmp(groupName, GWEN_DB_GroupName(n))==0) {
      for (column=1; ; column++) {
        int idx;
        char namebuffer[64];
        char numbuffer[16];
        GWEN_DB_NODE_TYPE vt;
        char valbuffer[64];
        int iv;

        /* create name for column */
        GWEN_Text_NumToString(column, numbuffer, sizeof(numbuffer), 0);
        p=GWEN_DB_GetCharValue(colgr, numbuffer, 0, 0);
        if (!p) {
          /* no value. finished */
          GWEN_FASTBUFFER_WRITELINE(fb, err, "");
          if (err<0) {
            DBG_INFO(0, "Called from here");
            GWEN_FastBuffer_free(fb);
            return err;
          }
          DBG_VERBOUS(GWEN_LOGDOMAIN, "No colums left, line finished");
          break;
        }

        /* break down to name and index */
        idx=GWEN_CSV_GetNameAndIndex(p, namebuffer, sizeof(namebuffer));
        if (idx==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "Error in configuration: Bad name for column %d",
                   column);
          GWEN_FastBuffer_free(fb);
          return GWEN_ERROR_GENERIC;
        }
        /* get data */
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Checking value of %s[%d]", namebuffer, idx);
        if (GWEN_DB_VariableExists(n, namebuffer)) {
          vt=GWEN_DB_GetValueTypeByPath(n, namebuffer, idx);
          switch (vt) {
          case GWEN_DB_NodeType_ValueChar:
            p=GWEN_DB_GetCharValue(n, namebuffer, idx, "");
            break;
          case GWEN_DB_NodeType_ValueInt:
            iv=GWEN_DB_GetIntValue(n, namebuffer, idx, 0);
            snprintf(valbuffer, sizeof(valbuffer), "%d", iv);
            p=valbuffer;
            break;
          default:
            DBG_DEBUG(GWEN_LOGDOMAIN, "Unhandled value type %d", vt);
            p="";
          }
        }
        else
          p="";

        if (column!=1) {
          /* write delimiter */
          GWEN_FASTBUFFER_WRITEBYTE(fb, err, delimiter);
          if (err<0) {
            DBG_INFO(0, "Called from here");
            GWEN_FastBuffer_free(fb);
            return err;
          }
        } /* if not first column */
        if (quote) {
          /* write quotation mark */
          GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
          if (err<0) {
            DBG_INFO(0, "Called from here");
            GWEN_FastBuffer_free(fb);
            return err;
          }
        } /* if quote */
        /* write value */
        GWEN_FASTBUFFER_WRITEFORCED(fb, err, p, -1);
        if (err<0) {
          DBG_INFO(0, "Called from here");
          GWEN_FastBuffer_free(fb);
          return err;
        }
        if (quote) {
          /* write quotation mark */
          GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
          if (err<0) {
            DBG_INFO(0, "Called from here");
            GWEN_FastBuffer_free(fb);
            return err;
          }
        } /* if quote */

      } /* for */
    } /* if group name matches */
    n=GWEN_DB_GetNextGroup(n);
  } /* while n */

  /* flush */
  GWEN_FASTBUFFER_FLUSH(fb, err);
  if (err<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
    GWEN_FastBuffer_free(fb);
    return err;
  }

  GWEN_FastBuffer_free(fb);
  return 0;
}



int GWEN_DBIO_CSV_Import(GWEN_DBIO *dbio,
                         GWEN_SYNCIO *sio,
                         GWEN_DB_NODE *data,
                         GWEN_DB_NODE *cfg,
                         GWEN_UNUSED uint32_t flags)
{
  GWEN_DB_NODE *colgr;
  int delimiter;
  /*int quote;*/
  const char *p;
  const char *groupName;
  int err;
  int title;
  GWEN_STRINGLIST *sl;
  GWEN_BUFFER *lbuffer;
  char delimiters[2];
  int lines;
  int ignoreLines;
  int fixedWidth;
  int condense;
  GWEN_FAST_BUFFER *fb;

  assert(dbio);
  assert(sio);
  assert(cfg);
  assert(data);

  fb=GWEN_FastBuffer_new(512, sio);

  /* get general configuration */
  colgr=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "columns");
  if (!colgr) {
    DBG_ERROR(0, "Error in configuration: No columns specified");
    GWEN_FastBuffer_free(fb);
    return GWEN_ERROR_INVALID;
  }
  p=GWEN_DB_GetCharValue(cfg, "delimiter", 0, ";");
  if (strcasecmp(p, "TAB")==0)
    delimiter=9;
  else if (strcasecmp(p, "SPACE")==0)
    delimiter=32;
  else
    delimiter=p[0];
  delimiters[0]=delimiter;
  delimiters[1]=0;
  /*quote=GWEN_DB_GetIntValue(cfg, "quote", 0, 1);*/
  fixedWidth=GWEN_DB_GetIntValue(cfg, "fixedWidth", 0, 0);
  condense=GWEN_DB_GetIntValue(cfg, "condense", 0, 0);
  groupName=GWEN_DB_GetCharValue(cfg, "group", 0, "line");
  title=GWEN_DB_GetIntValue(cfg, "title", 0, 1);
  ignoreLines=GWEN_DB_GetIntValue(cfg, "ignoreLines", 0, 0);
  if (title)
    ignoreLines++;

  sl=GWEN_StringList_new();
  lbuffer=GWEN_Buffer_new(0, 256, 0, 1);

  lines=0;
  for (;;) {
    GWEN_BUFFER *wbuffer;
    int rv;
    const char *s;
    GWEN_STRINGLISTENTRY *se;
    int col;
    GWEN_DB_NODE *n;

    /* read line */
    DBG_DEBUG(GWEN_LOGDOMAIN, "Reading line %d", lines);
    GWEN_Buffer_Reset(lbuffer);
    err=GWEN_FastBuffer_ReadLineToBuffer(fb, lbuffer);
    if (err<0) {
      if (err==GWEN_ERROR_EOF) {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "EOF met");
        break;
      }
      else {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        GWEN_Buffer_free(lbuffer);
        GWEN_StringList_free(sl);
        GWEN_FastBuffer_free(fb);
        return err;
      }
    }

    if (lines<ignoreLines) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Ignoring line %d", lines);
    }
    else {
      /* read columns */
      wbuffer=GWEN_Buffer_new(0, 256, 0, 1);

      s=GWEN_Buffer_GetStart(lbuffer);
      if (fixedWidth) {
        int i;
        unsigned int llength;
        unsigned int lpos=0;

        llength=strlen(s);
        for (i=0; ; i++) {
          int w;
          char *t=0;
          int left;

          left=llength-lpos;
          w=GWEN_DB_GetIntValue(cfg, "width", i, -1);
          if (w<1)
            break;
          if (w>left)
            w=left;
          if (w<1)
            break;
          t=(char *)malloc(w+1);
          memmove(t, s, w);
          t[w]=0;
          if (condense) {
            int j;

            for (j=w-1; j>=0; j--) {
              if ((unsigned char)(t[j])>32) {
                break;
              }
              t[j]=0;
            }
          }
          /* take over new string */
          GWEN_StringList_AppendString(sl, t, 1, 0);
          s+=w;
          lpos+=w;
        }
      }
      else {
        while (*s) {
          rv=GWEN_Text_GetWordToBuffer(s, delimiters, wbuffer,
                                       GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                                       GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                                       GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
                                       GWEN_TEXT_FLAGS_DEL_QUOTES,
                                       &s);
          if (rv) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(wbuffer);
            GWEN_Buffer_free(lbuffer);
            GWEN_StringList_free(sl);
            GWEN_FastBuffer_free(fb);
            return rv;
          }
          GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(wbuffer), 0, 0);
          GWEN_Buffer_Reset(wbuffer);
          if (*s) {
            if (strchr(delimiters, *s))
              s++;
          }
        } /* while */
      }
      GWEN_Buffer_free(wbuffer);

      /* store columns to db */
      n=GWEN_DB_Group_new(groupName);
      se=GWEN_StringList_FirstEntry(sl);
      col=1;
      while (se) {
        char nbuff[16];
        const char *vcol;

        DBG_DEBUG(0, "Handling column %d", col);
        nbuff[0]=0;
        snprintf(nbuff, sizeof(nbuff)-1, "%i", col);
        nbuff[sizeof(nbuff)-1]=0;

        vcol=GWEN_DB_GetCharValue(colgr, nbuff, 0, 0);
        if (vcol) {
          const char *bracket;
          GWEN_BUFFER *vname;

          bracket=strchr(vcol, '[');
          if (bracket) {
            /* copy column name without index */
            vname=GWEN_Buffer_new(0, bracket-vcol+1, 0, 1);
            GWEN_Buffer_AppendBytes(vname, vcol, bracket-vcol);
            vcol=GWEN_Buffer_GetStart(vname);
          }
          else
            vname=0;
          GWEN_DB_SetCharValue(n, GWEN_DB_FLAGS_DEFAULT,
                               vcol, GWEN_StringListEntry_Data(se));
          GWEN_Buffer_free(vname);
        }

        se=GWEN_StringListEntry_Next(se);
        col++;
      } /* while */

      /* add db to data */
      GWEN_DB_AddGroup(data, n);
    } /* if this is not the title line */
    GWEN_StringList_Clear(sl);
    lines++;
  } /* while */

  GWEN_Buffer_free(lbuffer);
  GWEN_StringList_free(sl);

  GWEN_FastBuffer_free(fb);

  return 0;
}



int GWEN_DBIO_CSV__ReadLine(GWEN_FAST_BUFFER *fb, GWEN_STRINGLIST *sl)
{
  int err;
  const char *delimiters=";\t,";
  GWEN_BUFFER *lbuffer;
  GWEN_BUFFER *wbuffer;
  int rv;
  const char *s;

  assert(fb);

  /* read line */
  lbuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_Reset(lbuffer);
  err=GWEN_FastBuffer_ReadLineToBuffer(fb, lbuffer);
  if (err<0) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(lbuffer);
    return err;
  }

  /* read columns */
  wbuffer=GWEN_Buffer_new(0, 256, 0, 1);

  s=GWEN_Buffer_GetStart(lbuffer);
  while (*s) {
    rv=GWEN_Text_GetWordToBuffer(s, delimiters, wbuffer,
                                 GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                                 GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                                 GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
                                 GWEN_TEXT_FLAGS_DEL_QUOTES,
                                 &s);
    if (rv) {
      GWEN_Buffer_free(wbuffer);
      GWEN_Buffer_free(lbuffer);
      return rv;
    }
    GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(wbuffer), 0, 0);
    GWEN_Buffer_Reset(wbuffer);
    if (*s) {
      if (strchr(delimiters, *s))
        s++;
    }
  } /* while */
  GWEN_Buffer_free(wbuffer);
  GWEN_Buffer_free(lbuffer);

  return 0;
}



GWEN_DBIO_CHECKFILE_RESULT GWEN_DBIO_CSV_CheckFile(GWEN_UNUSED GWEN_DBIO *dbio, const char *fname)
{
  int i;
  int rv;
  GWEN_SYNCIO *sio;
  GWEN_STRINGLIST *sl;
  GWEN_FAST_BUFFER *fb;

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  fb=GWEN_FastBuffer_new(512, sio);

  /* read line into string list */
  sl=GWEN_StringList_new();
  if (GWEN_DBIO_CSV__ReadLine(fb, sl)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reading a line");
    GWEN_FastBuffer_free(fb);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  /* first column: number */
  i=GWEN_StringList_Count(sl);
  GWEN_StringList_free(sl);
  if (i) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Found %d columns, file might be supported", i);
    GWEN_FastBuffer_free(fb);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    /*return GWEN_DBIO_CheckFileResultOk; */
    return GWEN_DBIO_CheckFileResultUnknown;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Found no columns, file might not be supported");
    GWEN_FastBuffer_free(fb);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return GWEN_DBIO_CheckFileResultUnknown;
  }
}



GWEN_DBIO *GWEN_DBIO_CSV_Factory(GWEN_UNUSED GWEN_PLUGIN *pl)
{
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("csv", "Imports and exports CSV data");
  GWEN_DBIO_SetImportFn(dbio, GWEN_DBIO_CSV_Import);
  GWEN_DBIO_SetExportFn(dbio, GWEN_DBIO_CSV_Export);
  GWEN_DBIO_SetCheckFileFn(dbio, GWEN_DBIO_CSV_CheckFile);
  return dbio;
}



GWEN_PLUGIN *dbio_csv_factory(GWEN_PLUGIN_MANAGER *pm,
                              const char *modName,
                              const char *fileName)
{
  GWEN_PLUGIN *pl;

  pl=GWEN_DBIO_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_DBIO_Plugin_SetFactoryFn(pl, GWEN_DBIO_CSV_Factory);

  return pl;

}








