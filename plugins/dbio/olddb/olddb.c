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

#include "olddb_p.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



GWEN_DB_NODE *GWEN_DBIO_OldDb__ParseLine(GWEN_DB_NODE *root,
					 GWEN_DB_NODE *group,
					 const char *s,
					 GWEN_TYPE_UINT32 mode) {
  char name[256];
  char *np;
  char *p;
  const char *g;
  int i;
  int quotes;
  int esc;
  int firstval;

  assert(s);
  name[0]=0;

  /* check for group definition */
  g=s;
  while(*g && (unsigned char)(*g)<33)
    g++;
  if (*g=='[') {
    /* ok, parse group name */
    GWEN_DB_NODE *grp;

    s=g;
    s++;
    while(*s && (unsigned char)(*s)<33)
      s++;
    p=name;
    i=sizeof(name)-1;
    while ((unsigned char)(*s)>31 && i && *s!=']' && *s!='#') {
      *p=*s;
      p++;
      s++;
    } /* while */
    if (!i) {
      DBG_ERROR(0, "Groupname is too long (limit is %d chars)",
		sizeof(name)-1);
      return 0;
    }
    if (*s!=']') {
      DBG_ERROR(0, "\"]\" expected");
      return 0;
    }
    *p=0;
    DBG_VERBOUS(0, "Selecting group \"%s\"",name);
    grp=GWEN_DB_GetGroup(root, mode, name);
    if (!grp) {
      DBG_DEBUG(0, "Group \"%s\" is not available", name);
      return 0;
    }
    return grp;
  }

  /* get name */
  while(*s && (unsigned char)(*s)<33)
    s++;
  i=sizeof(name)-1;
  p=name;
  while ((unsigned char)(*s)>31 && i-- && *s!='=' && *s!='#') {
    *p=*s;
    p++;
    s++;
  } /* while */
  if (!i) {
    DBG_ERROR(0, "Name is too long (limit is %d chars)", sizeof(name)-1);
    return 0;
  }
  *p=0;
  np=name;

  /* post process name */
  i=strlen(name)-1;
  while (i>=0) {
    if ((unsigned char)(name[i])<33)
      name[i]=0;
    else
      break;
    i--;
  }

  i=strlen(name);
  if (i>1) {
    if (name[i-1]=='"' &&
	name[0]=='"') {
      name[i-1]=0;
      np++;
    }
  }

  if ((unsigned char)(*s)<31 || *s=='#') {
    DBG_VERBOUS(0, "Empty line");
    return group;
  }

  /* get equation mark */
  if (*s!='=') {
    DBG_ERROR(0, "\"=\" expected");
    return 0;
  }
  s++;

  if (strlen(np)==0) {
    DBG_ERROR(0, "Variable name must not be empty");
    return 0;
  }

  DBG_VERBOUS(0, "Creating variable \"%s\"",np);


  firstval=1;
  /* read komma separated values */
  while ((unsigned char)(*s)>31) {
    char value[1024];
    char *vp;

    value[0]=0;

    /* skip komma that may occur */
    while(*s && (unsigned char)(*s)<33)
      s++;
    if (*s==0) {
      break;
    }
    if (*s==',') {
      if (firstval) {
	DBG_ERROR(0, "Unexpected comma");
	return 0;
      }
      s++;
    }
    else {
      if (!firstval) {
	DBG_ERROR(0, "Comma expected");
	return 0;
      }
    }

    /* get value */
    while(*s && (unsigned char)(*s)<33)
      s++;
    i=sizeof(value)-1;
    p=value;
    /* copy value */
    quotes=0;
    esc=0;
    while ((unsigned char)(*s)>31 && i) {
      if (esc) {
	*p=*s;
	p++;
        i--;
	esc=0;
      }
      else {
	if (*s=='\\')
	  esc=1;
	else if (*s=='"') {
	  quotes++;
	  if (quotes==2) {
	    s++;
	    break;
	  }
	}
	else if (*s=='#' && !(quotes&1))
	  break;
	else if (*s==',' && !(quotes&1))
	  break;
	else {
	  *p=*s;
	  p++;
	  i--;
	}
      }
      s++;
    } /* while */
    if (!i) {
      DBG_ERROR(0, "Value is too long (limit is %d chars)", sizeof(value)-1);
      return 0;
    }
    if (quotes&1) {
      DBG_ERROR(0, "Unbalanced quotation marks");
      return 0;
    }
    if (esc)
      DBG_WARN(0, "Backslash at the end of the line");
    *p=0;
    vp=value;
    /* post process value */
    if (quotes==0) {
      i=strlen(value)-1;
      while (i>=0) {
	if ((unsigned char)(value[i])<33)
	  value[i]=0;
	else
	  break;
	i--;
      }

      i=strlen(value);
      if (i>1) {
	if (value[i-1]=='"' &&
	    value[0]=='"') {
	  value[i-1]=0;
	  vp++;
	}
      }
    }

    /* create value, append it */
    DBG_VERBOUS(0, " Creating value \"%s\"", vp);
    GWEN_DB_SetCharValue(group, mode, np, vp);

    if (*s=='#')
      break;
    firstval=0;
  } /* while (reading values) */

  return group;
}



int GWEN_DBIO_OldDb_Import(GWEN_DBIO *dbio,
			   GWEN_BUFFEREDIO *bio,
                           GWEN_TYPE_UINT32 flags,
			   GWEN_DB_NODE *data,
			   GWEN_DB_NODE *cfg) {
  char lbuffer[2048];
  GWEN_DB_NODE *curr;
  int ln;
  GWEN_ERRORCODE gerr;

  assert(data);

  curr=data;
  ln=1;
  gerr=0;
  while (!GWEN_BufferedIO_CheckEOF(bio)) {
    lbuffer[0]=0;
    gerr=GWEN_BufferedIO_ReadLine(bio, lbuffer, sizeof(lbuffer)-1);
    if (!GWEN_Error_IsOk(gerr)) {
      DBG_ERROR_ERR(0, gerr);
      return 1;
    }
    curr=GWEN_DBIO_OldDb__ParseLine(data, curr, lbuffer, flags);
    if (!curr) {
      DBG_ERROR(0, "Error in input stream (line %d)", ln);
      return 1;
    }
    ln++;
  } /* while */
  return 0;
}



int GWEN_DBIO_OldDb_Export(GWEN_DBIO *dbio,
			   GWEN_BUFFEREDIO *bio,
			   GWEN_TYPE_UINT32 flags,
			   GWEN_DB_NODE *data,
			   GWEN_DB_NODE *cfg){
  DBG_ERROR(GWEN_LOGDOMAIN, "Export function not supported");
  return -1;
}



GWEN_DBIO_CHECKFILE_RESULT GWEN_DBIO_OldDb_CheckFile(GWEN_DBIO *dbio,
						     const char *fname){
  GWEN_BUFFEREDIO *bio;
  int fd;
  int rv;
  GWEN_DB_NODE *dbTmp;
  GWEN_DB_NODE *dbCfg;

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "open(%s): %s",
              fname,
	      strerror(errno));
    return 0;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  assert(bio);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  dbTmp=GWEN_DB_Group_new("tmp");
  dbCfg=GWEN_DB_Group_new("cfg");
  rv=GWEN_DBIO_OldDb_Import(dbio, bio, GWEN_DB_FLAGS_DEFAULT, dbTmp, dbCfg);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  GWEN_DB_Group_free(dbCfg);
  GWEN_DB_Group_free(dbTmp);
  if (rv) {
    return GWEN_DBIO_CheckFileResultNotOk;
  }
  return GWEN_DBIO_CheckFileResultOk;
}



GWEN_DBIO *GWEN_DBIO_OldDb_Factory(GWEN_PLUGIN *pl) {
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("OldDb", "Imports and exports Old OpenHBCI data");
  GWEN_DBIO_SetImportFn(dbio, GWEN_DBIO_OldDb_Import);
  GWEN_DBIO_SetExportFn(dbio, GWEN_DBIO_OldDb_Export);
  GWEN_DBIO_SetCheckFileFn(dbio, GWEN_DBIO_OldDb_CheckFile);
  return dbio;
}



GWEN_PLUGIN *dbio_olddb_factory(GWEN_PLUGIN_MANAGER *pm,
                                const char *modName,
                                const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_DBIO_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_DBIO_Plugin_SetFactoryFn(pl, GWEN_DBIO_OldDb_Factory);

  return pl;

}



