/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Oct 30 2003
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

#include "csv.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int GWEN_CSV_GetNameAndIndex(const char *name,
                             char *buffer,
                             unsigned int size) {
  unsigned int i;
  int rv;

  i=0;
  rv=0;
  /* read and copy name */
  while(name[i] && name[i]!='[' && i<size) {
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
    while(name[i] && name[i]!=']' && j<sizeof(numbuffer)) {
      numbuffer[j]=name[i];
      i++;
      j++;
    } /* while */
    if (j>=sizeof(numbuffer)) {
      DBG_INFO(0, "Index number too long (%d>=%d)", j, sizeof(numbuffer));
      return -1;
    }
    numbuffer[j]=0;
    rv=atoi(numbuffer);
  }

  return rv;
}



int GWEN_CSV_Write(GWEN_BUFFEREDIO *bio,
                   GWEN_DB_NODE *cfg,
                   GWEN_DB_NODE *data){
  GWEN_DB_NODE *colgr;
  GWEN_DB_NODE *n;
  int delimiter;
  int quote;
  const char *p;
  const char *groupName;
  GWEN_ERRORCODE err;
  unsigned int column;
  int title;

  assert(bio);
  assert(cfg);
  assert(data);

  /* get general configuration */
  colgr=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "columns");
  if (!colgr) {
    DBG_ERROR(0, "Error in configuration: No columns specified");
    return -1;
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
        err=GWEN_BufferedIO_WriteLine(bio, "");
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "Called from here");
          return -1;
        }
        DBG_INFO(0, "No colums left, line finished");
        break;
      }
      /* break down to name and index */
      idx=GWEN_CSV_GetNameAndIndex(p, namebuffer, sizeof(namebuffer));
      if (idx==-1) {
        DBG_INFO(0, "Error in configuration: Bad name for column %d",
                 column);
        return -1;
      }

      /* add idx to name, if not 0 */
      if (idx) {
        GWEN_Text_NumToString(idx, numbuffer, sizeof(numbuffer), 0);
        if (strlen(namebuffer)+strlen(numbuffer)+1>=sizeof(namebuffer)) {
          DBG_ERROR(0, "Internal: namebuffer too small");
          return -1;
        }
        strcat(namebuffer, numbuffer);
      }
      /* convert slashes to underscores */
      np=namebuffer;
      while(*np) {
        if (*np=='/')
          *np='_';
        np++;
      }

      if (column!=1) {
        /* write delimiter */
        err=GWEN_BufferedIO_WriteChar(bio, delimiter);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "Called from here");
          return -1;
        }
      } /* if not first column */
      if (quote) {
        /* write quotation mark */
        err=GWEN_BufferedIO_WriteChar(bio, '\"');
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "Called from here");
          return -1;
        }
      } /* if quote */
      /* write value */
      err=GWEN_BufferedIO_Write(bio, namebuffer);
      if (!GWEN_Error_IsOk(err)) {
        DBG_INFO(0, "Called from here");
        return -1;
      }
      if (quote) {
        /* write quotation mark */
        err=GWEN_BufferedIO_WriteChar(bio, '\"');
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "Called from here");
          return -1;
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

        /* create name for column */
        GWEN_Text_NumToString(column, numbuffer, sizeof(numbuffer), 0);
        p=GWEN_DB_GetCharValue(colgr, numbuffer, 0, 0);
        if (!p) {
          /* no value. finished */
          err=GWEN_BufferedIO_WriteLine(bio, "");
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "Called from here");
            return -1;
          }
          DBG_INFO(0, "No colums left, line finished");
          break;
        }
        /* break down to name and index */
        idx=GWEN_CSV_GetNameAndIndex(p, namebuffer, sizeof(namebuffer));
        if (idx==-1) {
          DBG_INFO(0, "Error in configuration: Bad name for column %d",
                   column);
          return -1;
        }
        /* get data */
        DBG_DEBUG(0, "Checking value of %s[%d]", namebuffer, idx);
        p=GWEN_DB_GetCharValue(n, namebuffer, idx, "");
        if (column!=1) {
          /* write delimiter */
          err=GWEN_BufferedIO_WriteChar(bio, delimiter);
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "Called from here");
            return -1;
          }
        } /* if not first column */
        if (quote) {
          /* write quotation mark */
          err=GWEN_BufferedIO_WriteChar(bio, '\"');
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "Called from here");
            return -1;
          }
        } /* if quote */
        /* write value */
        err=GWEN_BufferedIO_Write(bio, p);
        if (!GWEN_Error_IsOk(err)) {
          DBG_INFO(0, "Called from here");
          return -1;
        }
        if (quote) {
          /* write quotation mark */
          err=GWEN_BufferedIO_WriteChar(bio, '\"');
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "Called from here");
            return -1;
          }
        } /* if quote */

      } /* for */
    } /* if group name matches */
    n=GWEN_DB_GetNextGroup(n);
  } /* while n */
  return 0;
}



int GWEN_CSV_Read(GWEN_BUFFEREDIO *bio,
                  GWEN_DB_NODE *cfg,
                  GWEN_DB_NODE *data){
  DBG_ERROR(0, "Function not yet implemented");
  return -1;
}





