/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
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

#ifndef GWENHYWFAR_HBCIDLG_H
#define GWENHYWFAR_HBCIDLG_H


#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/seccontext.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MOD_HBCIDIALOG HBCI Dialog
 * @ingroup MOD_HBCI
 */
/*@{*/


/** @name Dialog Flags
 *
 */
/*@{*/
#define GWEN_HBCIDIALOG_FLAGS_INITIATOR 0x0001
/*@}*/


typedef struct GWEN_HBCIDIALOG GWEN_HBCIDIALOG;


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
typedef void
  (*GWEN_HBCIDLG_FREEDATA_FN)(GWEN_HBCIDIALOG *hdlg);

typedef void
  (*GWEN_HBCIDLG_RESET_FN)(GWEN_HBCIDIALOG *hdlg);
/*@}*/


/** @name Constructors And Destructors And More
 *
 */
/*@{*/
GWEN_HBCIDIALOG *GWEN_HBCIDialog_new(GWEN_MSGENGINE *e,
                                     GWEN_SECCTX_MANAGER *scm);
void GWEN_HBCIDialog_free(GWEN_HBCIDIALOG *hdlg);

/**
 * Increments the internal usage counter for this dialog. This is used
 * by @ref GWEN_HBCIMsg_new. This makes sure that a dialog object is not
 * freed as long as any message is attached to it.
 * The function @ref GWEN_HBCIDialog_new initializes this counter to 1.
 * If you don't want to use a dialog anymore but there could still be some
 * HBCI messages (or maybe other objects using this mechanism) referring to
 * this dialog you can call @ref GWEN_HBCIDialog_Detach instead of
 * @ref GWEN_HBCIDialog_free.
 */
void GWEN_HBCIDialog_Attach(GWEN_HBCIDIALOG *hdlg);

/**
 * Decrements the internal usage counter for this dialog. This is used
 * by @ref GWEN_HBCIMsg_free. This makes sure that a dialog object is not
 * freed as long as any message is attached to it.
 * If the usage counter reaches zero upon calling of this function then the
 * dialog will automatically be freed.
 */
void GWEN_HBCIDialog_Detach(GWEN_HBCIDIALOG *hdlg);

/*@}*/


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
void GWEN_HBCIDialog_SetFreeDataFn(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCIDLG_FREEDATA_FN fn);
void GWEN_HBCIDialog_SetResetFn(GWEN_HBCIDIALOG *hdlg,
                                GWEN_HBCIDLG_RESET_FN fn);
void GWEN_HBCIDialog_SetInheritorData(GWEN_HBCIDIALOG *hdlg,
                                      void *data);
void *GWEN_HBCIDialog_GetInheritorData(GWEN_HBCIDIALOG *hdlg);
/*@}*/


/** @name Virtual Functions
 *
 */
/*@{*/
/**
 * Resets the dialog for the next connection. It performs - among others -
 * the following actions:
 * <ul>
 *  <li>initialize the dialog id to "0"</li>
 *  <li>resets the number of the next message to 1</li>
 *  <li>resets the number of the last received message to 0</li>
 *  <li>clears the remote name</li>
 * </ul>
 */
void GWEN_HBCIDialog_Reset(GWEN_HBCIDIALOG *hdlg);
/*@}*/


/** @name Getters And Setters
 *
 */
/*@{*/
GWEN_MSGENGINE *GWEN_HBCIDialog_GetMsgEngine(GWEN_HBCIDIALOG *hdlg);

const char *GWEN_HBCIDialog_GetDialogId(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetDialogId(GWEN_HBCIDIALOG *hdlg,
                                 const char *s);

unsigned int GWEN_HBCIDialog_GetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg,
                                           unsigned int i);

unsigned int GWEN_HBCIDialog_GetNextMsgNum(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetNextMsgNum(GWEN_HBCIDIALOG *hdlg,
                                   unsigned int i);
unsigned int GWEN_HBCIDialog_GetFlags(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetFlags(GWEN_HBCIDIALOG *hdlg,
                              unsigned int f);

GWEN_SECCTX_MANAGER*
  GWEN_HBCIDialog_GetSecurityManager(GWEN_HBCIDIALOG *hdlg);
const char *GWEN_HBCIDialog_GetLocalName(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetLocalName(GWEN_HBCIDIALOG *hdlg,
                                  const char *s);

const char *GWEN_HBCIDialog_GetRemoteName(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetRemoteName(GWEN_HBCIDIALOG *hdlg,
                                   const char *s);
/*@}*/



/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif


#endif



