/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Nov 25 2003
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

/** @file gwenhywfar/ipcxmlcmd.h
 * @brief This file contains some often needed generic IPC commands
 */

#ifndef GWENHYWFAR_IPCXMLCMD_H
#define GWENHYWFAR_IPCXMLCMD_H


#include <gwenhywfar/ipcxml.h>
#include <gwenhywfar/keyspec.h>


/** @defgroup MOD_IPCXMLCMD Generic IPC XML Request Creators And Handlers
 * @ingroup MOD_IPC_HIGHLEVEL
 * @brief This group contains some often needed generic IPC commands.
 *
 * There are 3 functions for most request types:
 * <ul>
 *   <li><strong>Request</strong>: Creates a request and enqueues it</li>
 *   <li><strong>Handle</strong>: Handles an incoming request of that type
 *       and returns a response</li>
 *   <li><strong>Result</strong>: Checks whether there is a response for a
 *       request of that type (called after the request function, if a
 *       response is expected)</li>
 * </ul>
 */
/*@{*/


/** @name Segment Result Handling
 * @brief These functions handle segment results.
 *
 */
/*@{*/

/**
 * This function returns the segment result code. The following groups
 * are defined so far (subset of HBCI protocol):
 * <ul>
 *  <li><strong>0000-0999</strong>: Ok</li>
 *  <li><strong>1000-1999</strong>: Information</li>
 *  <li><strong>3000-3999</strong>: Warning</li>
 *  <li><strong>9000-9999</strong>: Error</li>
 * </ul>
 * @param xs Pointer to the service to use
 * @param db DB node which contains a "SegResult" group
 * @param result pointer to a variable to receive the result code
 */
GWEN_ERRORCODE GWEN_IPCXMLCmd_Result_SegResult(GWEN_IPCXMLSERVICE *xs,
                                               GWEN_DB_NODE *db,
                                               unsigned int *result);
/**
 * This function sends a segment result in reply to a request.
 * For the sanity of the IPC protocol you should return a segment result
 * for every request which does not respond with any data. In such a case
 * a segment result reply would at least inform the requestor about
 * the result of his request.
 * @param xs Pointer to the service to use
 * @param rqid id of the incoming request to which a response is
 * to be created (retrieved via @ref GWEN_IPCXMLService_GetNextRequest).
 * @ref GWEN_IPCXMLService_AddClient or @ref GWEN_IPCXMLService_AddServer
 * @param flags see @ref GWEN_IPCXML_REQUESTFLAGS_FLUSH
 * @param result result code (see @ref GWEN_IPCXMLCmd_Result_SegResult)
 * @param text result text
 * @param param RFU (use 0 for now)
 */
GWEN_ERRORCODE GWEN_IPCXMLCmd_Response_SegResult(GWEN_IPCXMLSERVICE *xs,
                                                 unsigned int rqid,
                                                 unsigned int flags,
                                                 unsigned int result,
                                                 const char *text,
                                                 const char *param);
/*@}*/



/** @name Getting Public Keys
 * @brief Requests a public key or handles such a request
 *
 */
/*@{*/
unsigned int GWEN_IPCXMLCmd_Request_GetPubKey(GWEN_IPCXMLSERVICE *xs,
                                              unsigned int clid,
                                              unsigned int flags,
                                              const GWEN_KEYSPEC *ks);
GWEN_ERRORCODE GWEN_IPCXMLCmd_Handle_GetPubKey(GWEN_IPCXMLSERVICE *xs,
                                               unsigned int rqid,
                                               unsigned int flags,
                                               GWEN_DB_NODE *n);

GWEN_ERRORCODE GWEN_IPCXMLCmd_Result_GetPubKey(GWEN_IPCXMLSERVICE *xs,
                                               unsigned int rqid,
                                               GWEN_CRYPTKEY **pkey);
/*@}*/



/** @name Sending Public Keys
 * @brief Sends a public key or handles such a request
 *
 * <p>
 * This group contains no handler for this request, since only the application
 * can decide what to do with a received key. Some applications may decide
 * to temporary store the received key for review before using it
 * permanently, others may simply set the received as the new sign/crypt key.
 * </p>
 * <p>
 * However, the handler should create a "SegResult" for the request which
 * tells the requestor about the success of the key submission.
 * </p>
 */
/*@{*/
unsigned int GWEN_IPCXMLCmd_Request_SendPubKey(GWEN_IPCXMLSERVICE *xs,
                                               unsigned int connid,
                                               unsigned int flags,
                                               const GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_IPCXMLCmd_Result_SendPubKey(GWEN_IPCXMLSERVICE *xs,
                                                unsigned int rqid,
                                                unsigned int *result);

/*@}*/


/*@}*/


#endif
