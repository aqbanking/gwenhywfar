/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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
 * @file chameleon/inetaddr_p.h
 * @short This file contains the internet address handling module
 */

#ifndef GWEN_INETADDR_P_H
#define GWEN_INETADDR_P_H


#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/error.h>
#include <gwenhyfwar/inetaddr.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @internal
 */
GWENHYFWAR_API struct GWEN_INETADDRESSSTRUCT {
  GWEN_AddressFamily af;
  int size;
  struct sockaddr *address;
};


#ifdef __cplusplus
}
#endif

/*@} defgroup */


#endif /* MOD_INETADDR_H */




