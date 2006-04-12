/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: olddb.c 1020 2006-03-26 17:16:05Z aquamaniac $
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


#include "rfc822_p.h"


#include <gwenhywfar/buffer.h>
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/dbio.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>



int GWEN_DBIO_Rfc822_Export(GWEN_DBIO *dbio,
			    GWEN_BUFFEREDIO *bio,
                            GWEN_TYPE_UINT32 flags,
                            GWEN_DB_NODE *data,
			    GWEN_DB_NODE *cfg) {
  GWEN_DB_NODE *dbVar;

  dbVar=GWEN_DB_GetFirstVar(data);
  while(dbVar) {
    GWEN_DB_NODE *dbVal;

    dbVal=GWEN_DB_GetFirstValue(dbVar);
    while(dbVal) {
      char valbuffer[64];
      int iv;
      GWEN_DB_VALUETYPE vt;
      const char *p;

      GWEN_BufferedIO_Write(bio, GWEN_DB_VariableName(dbVar));
      GWEN_BufferedIO_Write(bio, ": ");

      vt=GWEN_DB_GetValueType(dbVal);
      switch(vt) {
      case GWEN_DB_VALUETYPE_CHAR:
	p=GWEN_DB_GetCharValueFromNode(dbVal);
	break;
      case GWEN_DB_VALUETYPE_INT:
	iv=GWEN_DB_GetIntValueFromNode(dbVal);
	snprintf(valbuffer, sizeof(valbuffer), "%d", iv);
	p=valbuffer;
	break;
      default:
	DBG_DEBUG(GWEN_LOGDOMAIN, "Unhandled value type %d", vt);
	p="";
      }

      if (p) {
	for (;;) {
	  const char *t;
	  GWEN_ERRORCODE err;

	  t=strchr(p, 10);
	  if (t) {
	    unsigned int bsize;

            bsize=t-p;
	    err=GWEN_BufferedIO_WriteRawForced(bio, p, &bsize);
	    if (!GWEN_Error_IsOk(err)) {
	      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
	      return GWEN_Error_GetSimpleCode(err);
	    }
	    GWEN_BufferedIO_WriteLine(bio, "");
	    GWEN_BufferedIO_Write(bio, "\t");
	    t++;
            p=t;
	  }
	  else {
	    GWEN_BufferedIO_WriteLine(bio, p);
	    break;
	  }
	} /* for */
      }
      else
	GWEN_BufferedIO_WriteLine(bio, "");

      dbVal=GWEN_DB_GetNextValue(dbVal);
    }

    dbVar=GWEN_DB_GetNextVar(dbVar);
  }

  return 0;
}



int GWEN_DBIO_Rfc822__Import(GWEN_DBIO *dbio,
			     GWEN_BUFFEREDIO *bio,
			     GWEN_TYPE_UINT32 flags,
			     GWEN_DB_NODE *data,
			     int checkOnly) {
  GWEN_BUFFER *lineBuf;
  GWEN_ERRORCODE err;
  int cont=1;

  lineBuf=GWEN_Buffer_new(0, 256, 0, 1);

  while(cont) {
    char *p;
    char *name;
    char *values;

    for (;;) {
      unsigned int len;
      int i;
      unsigned char c;

      len=GWEN_Buffer_GetUsedBytes(lineBuf);
      err=GWEN_BufferedIO_ReadLine2Buffer(bio, lineBuf);
      if (!GWEN_Error_IsOk(err)) {
	DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
	GWEN_Buffer_free(lineBuf);
	return GWEN_Error_GetSimpleCode(err);
      }

      if (GWEN_Buffer_GetUsedBytes(lineBuf)==len &&
          (flags & GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE)) {
	cont=0;
	break;
      }

      i=GWEN_BufferedIO_PeekChar(bio);
      if (i<0) {
	GWEN_Buffer_free(lineBuf);
	return GWEN_ERROR_READ;
      }

      c=(unsigned int)i;
      if (c>32 || c==10 || c==13)
        break;

      /* skip blank */
      GWEN_BufferedIO_ReadChar(bio);
      GWEN_Buffer_AppendByte(lineBuf, 10);
    }

    if (GWEN_Buffer_GetUsedBytes(lineBuf)==0)
      break;

    /* get name */
    name=GWEN_Buffer_GetStart(lineBuf);
    p=strchr(name, ':');
    if (!p) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Error in line %d: Missing header name",
               GWEN_BufferedIO_GetLines(bio));
      GWEN_Buffer_free(lineBuf);
      return GWEN_ERROR_BAD_DATA;
    }
    *p=0;
    p++;
    while(*p && isspace(*p))
      p++;
    values=p;

    if (checkOnly)
      break;

    GWEN_DB_SetCharValue(data, flags, name, values);

    GWEN_Buffer_Reset(lineBuf);
  }

  GWEN_Buffer_free(lineBuf);
  return 0;
}



int GWEN_DBIO_Rfc822_Import(GWEN_DBIO *dbio,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_TYPE_UINT32 flags,
                            GWEN_DB_NODE *data,
			    GWEN_DB_NODE *cfg) {
  return GWEN_DBIO_Rfc822__Import(dbio, bio, flags, data, 0);
}



GWEN_DBIO_CHECKFILE_RESULT GWEN_DBIO_Rfc822_CheckFile(GWEN_DBIO *dbio,
						      const char *fname) {
  int fd;
  GWEN_BUFFEREDIO *bio;
  int rv;

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "open(%s): %s", fname, strerror(errno));
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  rv=GWEN_DBIO_Rfc822__Import(dbio, bio, 0, 0, 1);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  if (rv)
    return GWEN_DBIO_CheckFileResultNotOk;

  /* don't be too sure about whether the file can be read completely */
  return GWEN_DBIO_CheckFileResultUnknown;
}



GWEN_DBIO *GWEN_DBIO_Rfc822_Factory(GWEN_PLUGIN *pl) {
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("Rfc822", "Imports and exports RFC822 data");
  GWEN_DBIO_SetImportFn(dbio, GWEN_DBIO_Rfc822_Import);
  GWEN_DBIO_SetExportFn(dbio, GWEN_DBIO_Rfc822_Export);
  GWEN_DBIO_SetCheckFileFn(dbio, GWEN_DBIO_Rfc822_CheckFile);
  return dbio;
}



GWEN_PLUGIN *dbio_rfc822_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *modName,
				 const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_DBIO_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_DBIO_Plugin_SetFactoryFn(pl, GWEN_DBIO_Rfc822_Factory);

  return pl;
}


