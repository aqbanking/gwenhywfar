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


#ifndef GWENHYWFAR_SYNCIO_P_H
#define GWENHYWFAR_SYNCIO_P_H

#include <gwenhywfar/syncio.h>



struct GWEN_SYNCIO {
  GWEN_INHERIT_ELEMENT(GWEN_SYNCIO)
  GWEN_LIST_ELEMENT(GWEN_SYNCIO)

  uint32_t refCount;

  GWEN_SYNCIO *baseIo;

  char *typeName;
  uint32_t flags;
  GWEN_SYNCIO_STATUS status;

  GWEN_SYNCIO_CONNECT_FN connectFn;
  GWEN_SYNCIO_DISCONNECT_FN disconnectFn;

  GWEN_SYNCIO_FLUSH_FN flushFn;
  GWEN_SYNCIO_READ_FN readFn;

  GWEN_SYNCIO_WRITE_FN writeFn;

};






#endif


