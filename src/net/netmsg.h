/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
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


#ifndef GWEN_NETMSG_H
#define GWEN_NETMSG_H

typedef struct GWEN_NETMSG GWEN_NETMSG;

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTION_DEFS(GWEN_NETMSG, GWEN_NetMsg);


/** @name Constructors And Destructors
 *
 */
/*@{*/
GWEN_NETMSG *GWEN_NetMsg_new(GWEN_TYPE_UINT32 bufferSize);
void GWEN_NetMsg_free(GWEN_NETMSG *msg);
void GWEN_NetMsg_Attach(GWEN_NETMSG *msg);

/*@}*/


/** @name Getters And Setters
 *
 */
/*@{*/
/**
 * Returns a pointer to the buffer belonging to a message.
 * This function does NOT relinquish ownership.
 */
GWEN_BUFFER *GWEN_NetMsg_GetBuffer(const GWEN_NETMSG *msg);

GWEN_BUFFER *GWEN_NetMsg_TakeBuffer(GWEN_NETMSG *msg);

/**
 * Replaces the internal buffer with the given one.
 * Takes over ownership of the buffer.
 */
void GWEN_NetMsg_SetBuffer(GWEN_NETMSG *msg,
                           GWEN_BUFFER *buf);

/**
 * Returns the value of the size variable. The meaning of this variable
 * depends on the protocol this message belongs to.
 */
GWEN_TYPE_UINT32 GWEN_NetMsg_GetSize(const GWEN_NETMSG *msg);
void GWEN_NetMsg_SetSize(GWEN_NETMSG *msg,
                         GWEN_TYPE_UINT32 size);

/**
 * Returns a pointer to the DB belonging to a message.
 * This function does NOT relinquish ownership.
 */
GWEN_DB_NODE *GWEN_NetMsg_GetDB(const GWEN_NETMSG *msg);

/*@}*/



#endif
