/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Sep 16 2003
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


#ifndef GWENHYFWAR_CONNLAYER_H
#define GWENHYFWAR_CONNLAYER_H

#include <gwenhywfar/msglayer.h>
#include <gwenhywfar/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup MOD_IPC_CONNLAYER IPC Connection Layer
 * The connection layer works on top of the @ref MOD_IPC_MSGLAYER
 */
/*@{*/

#define GWEN_IPCCONNLAYER_FLAGS_PERSISTENT 0x0001
#define GWEN_IPCCONNLAYER_FLAGS_PASSIVE    0x0002


typedef struct GWEN_IPCCONNLAYER GWEN_IPCCONNLAYER;

typedef void (*GWEN_IPCCONNLAYER_FREE)(GWEN_IPCCONNLAYER *cl);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_WORK)(GWEN_IPCCONNLAYER *cl,
                                                 int rd);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_ACCEPT)(GWEN_IPCCONNLAYER *cl,
                                                   GWEN_IPCMSGLAYER *ml,
                                                   GWEN_IPCCONNLAYER **c);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_OPEN)(GWEN_IPCCONNLAYER *cl);
typedef GWEN_ERRORCODE (*GWEN_IPCCONNLAYER_CLOSE)(GWEN_IPCCONNLAYER *cl,
                                                  int force);

typedef void (*GWEN_IPCCONNLAYER_UP)(GWEN_IPCCONNLAYER *cl);
typedef void (*GWEN_IPCCONNLAYER_DOWN)(GWEN_IPCCONNLAYER *cl);



GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_GetNext(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_Work(GWEN_IPCCONNLAYER *cl,
                                         int rd);
GWEN_ERRORCODE GWEN_ConnectionLayer_Open(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_Close(GWEN_IPCCONNLAYER *cl,
                                          int force);
void GWEN_ConnectionLayer_Up(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_Down(GWEN_IPCCONNLAYER *cl);

unsigned int GWEN_ConnectionLayer_GetId(GWEN_IPCCONNLAYER *cl);
unsigned int GWEN_ConnectionLayer_GetFlags(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetFlags(GWEN_IPCCONNLAYER *cl,
                                   unsigned int flags);

const char *GWEN_ConnectionLayer_GetInfo(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetInfo(GWEN_IPCCONNLAYER *cl, const char *s);

unsigned int GWEN_ConnectionLayer_GetUserMark(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetUserMark(GWEN_IPCCONNLAYER *cl,
                                      unsigned int i);
unsigned int GWEN_ConnectionLayer_GetLibMark(GWEN_IPCCONNLAYER *cl);



GWEN_IPCCONNLAYER *GWEN_ConnectionLayer_new(GWEN_IPCMSGLAYER *ml);
void GWEN_ConnectionLayer_free(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_Append(GWEN_IPCCONNLAYER *cl,
                                 GWEN_IPCCONNLAYER *newcl);
void *GWEN_ConnectionLayer_GetData(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetData(GWEN_IPCCONNLAYER *cl, void *d);

GWEN_SOCKET *GWEN_ConnectionLayer_GetReadSocket(GWEN_IPCCONNLAYER *cl);
GWEN_SOCKET *GWEN_ConnectionLayer_GetWriteSocket(GWEN_IPCCONNLAYER *cl);
GWEN_ERRORCODE GWEN_ConnectionLayer_Accept(GWEN_IPCCONNLAYER *cl,
                                           GWEN_IPCCONNLAYER **c);

void GWEN_ConnectionLayer_SetFreeDataFn(GWEN_IPCCONNLAYER *cl,
                                        GWEN_IPCCONNLAYER_FREE f);
void GWEN_ConnectionLayer_SetWorkFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_WORK f);
void GWEN_ConnectionLayer_SetAcceptFn(GWEN_IPCCONNLAYER *cl,
                                      GWEN_IPCCONNLAYER_ACCEPT f);
void GWEN_ConnectionLayer_SetOpenFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_OPEN f);
void GWEN_ConnectionLayer_SetCloseFn(GWEN_IPCCONNLAYER *cl,
                                     GWEN_IPCCONNLAYER_CLOSE f);
void GWEN_ConnectionLayer_SetUpFn(GWEN_IPCCONNLAYER *cl,
                                  GWEN_IPCCONNLAYER_UP f);
void GWEN_ConnectionLayer_SetDownFn(GWEN_IPCCONNLAYER *cl,
                                    GWEN_IPCCONNLAYER_DOWN f);


GWEN_IPCMSGLAYER *GWEN_ConnectionLayer_GetMsgLayer(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetLibMark(GWEN_IPCCONNLAYER *cl,
                                     unsigned int i);

unsigned int GWEN_ConnectionLayer_GetType(GWEN_IPCCONNLAYER *cl);
void GWEN_ConnectionLayer_SetType(GWEN_IPCCONNLAYER *cl,
                                  unsigned int i);

/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif /* GWENHYFWAR_CONNLAYER_H */

