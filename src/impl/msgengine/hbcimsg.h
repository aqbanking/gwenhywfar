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

#ifndef GWENHYWFAR_HBCIMSG_H
#define GWENHYWFAR_HBCIMSG_H

#include <gwenhywfar/error.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/hbcicryptocontext.h>
#include <gwenhywfar/hbcidialog.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MOD_HBCIMSG HBCI Message Parser And Creator
 * @ingroup MOD_HBCI
 *
 * A HBCI message is usually used this way:
 * <ul>
 *  <li>Encoding:
 *   <ol>
 *    <li>create a message using @ref GWEN_HBCIMsg_new</li>
 *    <li>set security flags using @ref GWEN_HBCIMsg_SetFlags</li>
 *    <li>set keyspec for encryption using @ref GWEN_HBCIMsg_SetCrypter</li>
 *    <li>add keyspecs for signers using @ref GWEN_HBCIMsg_AddSigner</li>
 *    <li>add nodes using @ref GWEN_HBCIMsg_AddNode</li>
 *    <li>encode the message using @ref GWEN_HBCIMsg_EncodeMsg</li>
 *    <li>retrieve the resulting message buffer using
 *      @ref GWEN_HBCIMsg_TakeBuffer</li>
 *   </ol>
 *  </li>
 *  <li>Decoding:
 *   <ol>
 *    <li>create a message using @ref GWEN_HBCIMsg_new</li>
 *    <li>hand over the data for the message to be decoded using
 *       @ref GWEN_HBCIMsg_SetBuffer (Please note that this function
 *       takes over ownership for the buffer!).</li>
 *    <li>decode the message using @ref GWEN_HBCIMsg_DecodeMsg</li>
 *   </ol>
 *  </li>
 * </ul>
 */
/*@{*/

typedef struct GWEN_HBCIMSG GWEN_HBCIMSG;


/** @name Message Flags
 * These flags define the behaviour upon encoding a message.
 */
/*@{*/
/**
 * If set then the message is to be signed upon encoding. In this case
 * there you MUST add signers by calling @ref GWEN_HBCIMsg_AddSigner.
 */
#define GWEN_HBCIMSG_FLAGS_SIGN  0x0001
/** if set then the message is to be encrypted upon encoding */
#define GWEN_HBCIMSG_FLAGS_CRYPT 0x0002
/*@}*/



/** @name Constructors And Destructors
 *
 */
/*@{*/
/**
 * Creates a new HBCI message. You MUST calll @ref GWEN_HBCIMsg_free if
 * you don't need the created object anymore in order to prevent memory
 * leaks.
 * @param hdlg HBCI dialog for which this message is to be created.
 */
GWEN_HBCIMSG *GWEN_HBCIMsg_new(GWEN_HBCIDIALOG *hdlg);

/**
 * Frees the given message and all ressources associated with it.
 */
void GWEN_HBCIMsg_free(GWEN_HBCIMSG *hmsg);
/*@}*/


/** @name Security
 *
 */
/*@{*/


/**
 * Defines the behaviour upon encoding a message. Please note that this
 * function MUST be called <strong>before</strong> adding nodes !
 */
void GWEN_HBCIMsg_SetFlags(GWEN_HBCIMSG *hmsg,
                           unsigned int f);

/**
 * Returns the flags which have been set so far.
 */
unsigned int GWEN_HBCIMsg_GetFlags(GWEN_HBCIMSG *hmsg);

/**
 * Returns the first signer out of a list of signers.
 * You cann call @ref GWEN_KeySpec_Next to iterate through the list.
 * You MUST NOT free the list returned, nor should you modify it !
 */
GWEN_KEYSPEC *GWEN_HBCIMsg_GetSigners(GWEN_HBCIMSG *hmsg);

/**
 * Adds a signer to the list of signers. Please note that you may not add
 * signers <strong>after</strong> adding the first data node by calling
 * @ref GWEN_HBCIMsg_AddNode !
 */
int GWEN_HBCIMsg_AddSigner(GWEN_HBCIMSG *hmsg,
                           const GWEN_KEYSPEC *ks);
/**
 * Returns the number of signers for this message. This number is updated
 * when you call @ref GWEN_HBCIMsg_AddSigner. It is also updated upon
 * @ref GWEN_HBCIMsg_DecodeMsg to reflect the number of signers of the
 * decoded message.
 */
unsigned int GWEN_HBCIMsg_GetSignerCount(GWEN_HBCIMSG *hmsg);

/**
 * Returns the keyspec of the key to be used (upon encoding) or which has been
 * used (upon decoding) for encryption/decryption of the message.
 */
const GWEN_KEYSPEC *GWEN_HBCIMsg_GetCrypter(const GWEN_HBCIMSG *hmsg);

/**
 * Sets the keyspec of the key to be used for encryption.
 */
void GWEN_HBCIMsg_SetCrypter(GWEN_HBCIMSG *hmsg,
                             const GWEN_KEYSPEC *ks);
/*@}*/


/** @name Getters And Setters
 *
 */
/*@{*/
/**
 * Returns a pointer to the buffer used internally.
 * You MUST NOT free the pointer, nor should you modify the object referred to
 * by the pointer !
 */
GWEN_BUFFER *GWEN_HBCIMsg_GetBuffer(GWEN_HBCIMSG *hmsg);

/**
 * Takes the buffer away from the HBCIMsg.
 * This leaves this object without a buffer, so you can not call
 * @ref GWEN_HBCIMsg_AddNode after calling this function.
 * The caller is responsible for freeing the returned buffer !
 */
GWEN_BUFFER *GWEN_HBCIMsg_TakeBuffer(GWEN_HBCIMSG *hmsg);

/**
 * Sets the buffer to be used internally. you only have to call this function
 * if you want to <i>decode</i> a message.
 * Takes over ownership of the given buffer.
 */
void GWEN_HBCIMsg_SetBuffer(GWEN_HBCIMSG *hmsg,
                            GWEN_BUFFER *bf);

/**
 * Returns the number of the message this one refers to. This is only used
 * for messages which are replies to previoulsy received messages.
 */
unsigned int GWEN_HBCIMsg_GetMsgRef(GWEN_HBCIMSG *hmsg);

/**
 * Sets the number of the message this one refers to. This is only used
 * for messages which are replies to previoulsy received messages.
 */
void GWEN_HBCIMsg_SetMsgRef(GWEN_HBCIMSG *hmsg,
                            unsigned int i);

/**
 * Returns the number of this message.
 */
unsigned int GWEN_HBCIMsg_GetMsgNumber(GWEN_HBCIMSG *hmsg);

/**
 * Sets the number of this message.
 * Each message number should be unique in context of the current dialog.
 */
void GWEN_HBCIMsg_SetMsgNumber(GWEN_HBCIMSG *hmsg,
                               unsigned int i);

/**
 * Returns the id of the message layer via which this message has been
 * received. This is only used when exchanging messages via Gwens IPC
 * mechanism (see @ref MOD_IPCXMLSERVICE for details).
 * An application should not use this function directly.
 * The message layer id is not used by any function within this group.
 */
unsigned int GWEN_HBCIMsg_GetMsgLayerId(GWEN_HBCIMSG *hmsg);

/**
 * An application should not use this function directly.
 * The message layer id is not used by any function within this group.
 */
void GWEN_HBCIMsg_SetMsgLayerId(GWEN_HBCIMSG *hmsg,
                                unsigned int i);

/**
 * Returns the number of the dialog within which this message has been
 * received. This is only used when exchanging messages via Gwens IPC
 * mechanism (see @ref MOD_IPCXMLSERVICE for details).
 * An application should not use this function directly.
 * The dialog number is not used by any function within this group.
 */
unsigned int GWEN_HBCIMsg_GetDialogNumber(GWEN_HBCIMSG *hmsg);

/**
 * Sets the number of the dialog within which this message has been
 * received. This is only used when exchanging messages via Gwens IPC
 * mechanism (see @ref MOD_IPCXMLSERVICE for details).
 * An application should not use this function directly.
 * The dialog number is not used by any function within this group.
 */
void GWEN_HBCIMsg_SetDialogNumber(GWEN_HBCIMSG *hmsg,
                                  unsigned int did);

/**
 * Returns the id of the dialog within which a message has been received.
 * The dialog id is part of the message header, it is defined by the HBCI
 * server.
 */
const char *GWEN_HBCIMsg_GetDialogId(GWEN_HBCIMSG *hmsg);
/*@}*/


/** @name Handling Message Lists
 *
 */
/*@{*/

/**
 * Returns the next HBCI message from a list.
 */
GWEN_HBCIMSG *GWEN_HBCIMsg_Next(GWEN_HBCIMSG *hmsg);

/**
 * Adds a HBCI message to a list.
 */
void GWEN_HBCIMsg_Add(GWEN_HBCIMSG *hmsg, GWEN_HBCIMSG **head);

/**
 * Unlinks a HBCI message from a list.
 */
void GWEN_HBCIMsg_Del(GWEN_HBCIMSG *hmsg, GWEN_HBCIMSG **head);
/*@}*/


/** @name Adding Nodes To A Message
 *
 */
/*@{*/
/**
 * Adds a node to the message. It creates a segment defined by the
 * given node and data.
 * @return number of the segment used by this node
 */
unsigned int GWEN_HBCIMsg_AddNode(GWEN_HBCIMSG *hmsg,
                                  GWEN_XMLNODE *node,
                                  GWEN_DB_NODE *data);

/**
 * Returns the number of the number which would be returned by a call
 * to @ref GWEN_HBCIMsg_AddNode.
 */
unsigned int GWEN_HBCIMsg_GetCurrentSegmentNumber(GWEN_HBCIMSG *hmsg);

/**
 * Returns the number of nodes already added to this message (not counting
 * message head and tail, crypt head and data or signature heads and tails,
 * if any. Just the number of nodes added via @ref GWEN_HBCIMsg_AddNode).
 */
unsigned int GWEN_HBCIMsg_GetNodes(GWEN_HBCIMSG *hmsg);
/*@}*/


/** @name Encoding And Decoding Messages
 *
 */
/*@{*/
/**
 * Encodes the message according to the message flags
 * (see @ref GWEN_HBCIMSG_FLAGS_SIGN and others).
 * It adds all administrative segments needed to build a valid HBCI protocol
 * message. After calling this method you can use @ref GWEN_HBCIMsg_TakeBuffer
 * to get the resulting message buffer.
 */
int GWEN_HBCIMsg_EncodeMsg(GWEN_HBCIMSG *hmsg);


/**
 * This function decrypts the message (if it is encrypted) and verifies all
 * signatures found.
 * The DB will be filled with the data of all parsed segments, so it will
 * even contain the data of some administrative segments like:
 * <ul>
 *  <li>message head</li>
 *  <li>message tail</li>
 *  <li>signature heads (if message has been signed)</li>
 *  <li>signature tails (if message has been signed)</li>
 * </ul>
 * If a signature can not be verified (because there is no remote sign key
 * or our remote sign key differs from that used signing the data) then a
 * "?" is prepended to the name of the signer.
 * If the signature could be verified but turned out to be invalid a "!" is
 * prepended to the signer's name.
 * Thus you can later check for correct signatures by checking for the first
 * character of the signer keyspec's owner.
 */
int GWEN_HBCIMsg_DecodeMsg(GWEN_HBCIMSG *hmsg,
                           GWEN_DB_NODE *gr,
                           unsigned int flags);
/*@}*/

/** @name Debugging
 *
 */
/*@{*/
void GWEN_HBCIMsg_Dump(const GWEN_HBCIMSG *hmsg, FILE *f, unsigned int indent);
/*@}*/


/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif



#endif



