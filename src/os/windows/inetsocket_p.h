/***************************************************************************
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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

/**
 * @file chameleon/socket.h
 * @short This file contains sockets and socket sets.
 */

#ifndef GWEN_SOCKET_P_H
#define GWEN_SOCKET_P_H

#include <winsock2.h>
#include <winerror.h>

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/types.h>
#include <sys/types.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API struct GWEN_SOCKET {
  GWEN_LIST_ELEMENT(GWEN_SOCKET)
  int socket;
  GWEN_SOCKETTYPE type;
  uint32_t flags;
};


GWENHYWFAR_API struct GWEN_SOCKETSETSTRUCT {
  fd_set set;
  int highest;
  uint32_t count;
};


/**
 * Initializes this module.
 */
GWENHYWFAR_API int GWEN_Socket_ModuleInit(void);

/**
 * Deinitializes this module.
 */
GWENHYWFAR_API int GWEN_Socket_ModuleFini(void);



#ifdef __cplusplus
}
#endif

#endif /* GWEN_SOCKET_P_H */



