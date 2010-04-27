/***************************************************************************
 begin       : Tue Apr 27 2010
 copyright   : (C) 2010 by Martin Preuss
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



#include "syncio_file_p.h"

#include <gwenhywfar/misc.h>

#include <assert.h>




GWEN_INHERIT(GWEN_SYNCIO, GWEN_SYNCIO_FILE)



GWEN_SYNCIO *GWEN_SyncIo_File_new(const char *path, GWEN_SYNCIO_FILE_CREATIONMODE cm) {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO_FILE *xio;

  assert(path);
  sio=GWEN_SyncIo_new(GWEN_SYNCIO_FILE_TYPE, NULL);
  GWEN_NEW_OBJECT(GWEN_SYNCIO_FILE, xio);
  GWEN_INHERIT_SETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio, xio, GWEN_SyncIo_File_FreeData);

  xio->creationMode=cm;
  xio->path=strdup(path);

  GWEN_SyncIo_SetConnectFn(sio, GWEN_SyncIo_File_Connect);
  GWEN_SyncIo_SetDisconnectFn(sio, GWEN_SyncIo_File_Disconnect);
  GWEN_SyncIo_SetReadFn(sio, GWEN_SyncIo_File_Read);
  GWEN_SyncIo_SetWriteFn(sio GWEN_SyncIo_File_Write);

  return sio;
}



const char *GWEN_SyncIo_File_GetPath(const GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(sio);

  return xio->path;
}



void GWENHYWFAR_CB GWEN_SyncIo_File_FreeData(void *bp, void *p) {
  GWEN_SYNCIO_FILE *xio;

  xio=(GWEN_SYNCIO_FILE*) p;
  free(xio->path);
  GWEN_FREE_OBJECT(xio);
}



int GWENHYWFAR_CB GWEN_SyncIo_File_Connect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(sio);

}



int GWENHYWFAR_CB GWEN_SyncIo_File_Disconnect(GWEN_SYNCIO *sio) {
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(sio);

}



int GWENHYWFAR_CB GWEN_SyncIo_File_Read(GWEN_SYNCIO *sio,
					uint8_t *buffer,
					uint32_t size) {
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(sio);

}



int GWENHYWFAR_CB GWEN_SyncIo_File_Write(GWEN_SYNCIO *sio,
					 const uint8_t *buffer,
					 uint32_t size) {
  GWEN_SYNCIO_FILE *xio;

  assert(sio);
  xio=GWEN_INHERIT_GETDATA(GWEN_SYNCIO, GWEN_SYNCIO_FILE, sio);
  assert(sio);

}


