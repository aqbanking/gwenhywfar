/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Jul 04 2003
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

#ifndef GWENHYFWAR_MSGENGINE_H
#define GWENHYFWAR_MSGENGINE_H

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/xml.h>
#include <gwenhyfwar/db.h>
#include <gwenhyfwar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GWEN__MSGENGINE GWEN_MSGENGINE;

#define GWEN_MSGENGINE_SHOW_FLAGS_NOSET 0x0001
#define GWEN_MSGENGINE_MAX_VALUE_LEN    8192


typedef int (*GWEN_MSGENGINE_TYPECHECK_PTR)(GWEN_MSGENGINE *e,
				       const char *value,
				       GWEN_XMLNODE *node,
				       char escapeChar);

typedef int (*GWEN_MSGENGINE_TYPEREAD_PTR)(GWEN_MSGENGINE *e,
                                           GWEN_BUFFER *msgbuf,
                                           GWEN_XMLNODE *node,
                                           GWEN_BUFFER *vbuf,
                                           char escapeChar,
                                           const char *delimiters);

/**
 * @return 0 on success, -1 on error, 1 if unknown type
 */
typedef int (*GWEN_MSGENGINE_TYPEWRITE_PTR)(GWEN_MSGENGINE *e,
                                            GWEN_BUFFER *gbuf,
                                            GWEN_BUFFER *data,
                                            GWEN_XMLNODE *node);


typedef int (*GWEN_MSGENGINE_BINTYPEREAD_PTR)(GWEN_MSGENGINE *e,
                                              GWEN_XMLNODE *node,
                                              GWEN_DB_NODE *gr,
                                              GWEN_BUFFER *vbuf);
typedef int (*GWEN_MSGENGINE_BINTYPEWRITE_PTR)(GWEN_MSGENGINE *e,
                                               GWEN_XMLNODE *node,
                                               GWEN_DB_NODE *gr,
                                               char *buffer,
                                               unsigned int bufsize);



typedef enum {
  GWEN_MsgEngineFormatText=0
} GWEN_MSGENGINE_FORMAT;


GWEN_MSGENGINE *GWEN_MsgEngine_new();
void GWEN_MsgEngine_free(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetFormat(GWEN_MSGENGINE *e, GWEN_MSGENGINE_FORMAT f);
GWEN_MSGENGINE_FORMAT GWEN_MsgEngine_GetFormat(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetEscapeChar(GWEN_MSGENGINE *e, char c);
char GWEN_MsgEngine_GetEscapeChar(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetCharsToEscape(GWEN_MSGENGINE *e, const char *c);
const char *GWEN_MsgEngine_GetCharsToEscape(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetMode(GWEN_MSGENGINE *e, const char *mode);
const char *GWEN_MsgEngine_GetMode(GWEN_MSGENGINE *e);

unsigned int GWEN_MsgEngine_GetConfigMode(GWEN_MSGENGINE *e);
void GWEN_MsgEngine_SetConfigMode(GWEN_MSGENGINE *e, unsigned int m);

GWEN_XMLNODE *GWEN_MsgEngine_GetDefinitions(GWEN_MSGENGINE *e);
void GWEN_MsgEngine_SetDefinitions(GWEN_MSGENGINE *e, GWEN_XMLNODE *n);

int GWEN_MsgEngine_AddDefinitions(GWEN_MSGENGINE *e,
                                  GWEN_XMLNODE *node);

void GWEN_MsgEngine_SetTypeCheckFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPECHECK_PTR p);
GWEN_MSGENGINE_TYPECHECK_PTR
  GWEN_MsgEngine_GetTypeCheckFunction(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetTypeReadFunction(GWEN_MSGENGINE *e,
                                        GWEN_MSGENGINE_TYPEREAD_PTR p);
GWEN_MSGENGINE_TYPEREAD_PTR
  GWEN_MsgEngine_GetTypeReadFunction(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetTypeWriteFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPEWRITE_PTR p);
GWEN_MSGENGINE_TYPEWRITE_PTR
  GWEN_MsgEngine_GetTypeWriteFunction(GWEN_MSGENGINE *e);


/** @name Handler for binary data
 *
 */
/*@{*/
void GWEN_MsgEngine_SetBinTypeReadFunction(GWEN_MSGENGINE *e,
                                           GWEN_MSGENGINE_BINTYPEREAD_PTR p);
GWEN_MSGENGINE_BINTYPEREAD_PTR
  GWEN_MsgEngine_GetBinTypeReadFunction(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetBinTypeWriteFunction(GWEN_MSGENGINE *e,
                                            GWEN_MSGENGINE_BINTYPEWRITE_PTR p);
GWEN_MSGENGINE_BINTYPEWRITE_PTR
  GWEN_MsgEngine_GetBinTypeWriteFunction(GWEN_MSGENGINE *e);
/*@}*/


void *GWEN_MsgEngine_GetInheritorData(GWEN_MSGENGINE *e);
void GWEN_MsgEngine_SetInheritorData(GWEN_MSGENGINE *e, void *d);

GWEN_XMLNODE *GWEN_MsgEngine_FindGroupByProperty(GWEN_MSGENGINE *e,
                                                 const char *pname,
                                                 int version,
                                                 const char *pvalue);

/**
 * Looks for a node of the given type.
 * Example: If type is "GROUP" then the node will be searched in
 * "<GROUPS>", and the tag name will be "<GROUPdef>".
 */
GWEN_XMLNODE *GWEN_MsgEngine_FindNodeByProperty(GWEN_MSGENGINE *e,
                                                const char *t,
                                                const char *pname,
                                                int version,
                                                const char *pvalue);

/**
 * Set a global variable which will be used for "$"-Variables in description
 * files.
 */
int GWEN_MsgEngine_SetValue(GWEN_MSGENGINE *e,
                            const char *path,
                            const char *value);
int GWEN_MsgEngine_SetIntValue(GWEN_MSGENGINE *e,
                               const char *path,
                               int value);
const char *GWEN_MsgEngine_GetValue(GWEN_MSGENGINE *e,
			       const char *path,
			       const char *defValue);
int GWEN_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                               const char *path,
                               int defValue);


int GWEN_MsgEngine_CreateMessage(GWEN_MSGENGINE *e,
                                 const char *msgName,
                                 int msgVersion,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_DB_NODE *msgData);

int GWEN_MsgEngine_CreateMessageFromNode(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         GWEN_BUFFER *gbuf,
                                         GWEN_DB_NODE *msgData);

int GWEN_MsgEngine_ShowMessage(GWEN_MSGENGINE *e,
                               const char *typ,
                               const char *msgName,
                               int msgVersion,
                               unsigned int flags);

/**
 * This function parses a single entity specified by a single
 * XML node (which may of course consist of multiple XML nodes).
 * This function makes no assumptions about the format of used variables
 * whatsoever. All data parsed from the given message is stored within the
 * given database.
 */
int GWEN_MsgEngine_ParseMessage(GWEN_MSGENGINE *e,
                                GWEN_XMLNODE *group,
                                GWEN_BUFFER *msgbuf,
                                GWEN_DB_NODE *gr);

/**
 * This function skips all bytes from the given buffer until the given
 * delimiter is found or the buffer ends. It also takes care of escape
 * characters (to not accidentally take an escaped delimiter for a real one)
 * and is able to identify and correctly skip binary data. For the latter
 * to work it takes into account that binary data is preceeded by a
 * "@123@" sequence, where "123" is the length of the binary data.
 * This sequence has been taken from the HBCI specs (German homebanking
 * protocol) and has proven to be very effective ;-)
 */
int GWEN_MsgEngine_SkipSegment(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *msgbuf,
                               unsigned char escapeChar,
                               unsigned char delimiter);

/**
 * This function reads all segments found within the given buffer.
 * This is used to read a full message containing multiple segments.
 * It assumes that each segment starts with a group with the id of
 * "SegHead" which defines the variable "code" to determine each segment
 * type. Please note that this function makes no further assumptions about
 * the format of a segment, group or element of a message. This is totally
 * based on the settings specified in the XML file.
 * Unknown segments are simply skipped.
 * For each segment found inside the message a group is created within the
 * given database. The name of that group is derived from the property
 * "id" within the XML description of each segment (or "code" if "id" does
 * not exist).
 * One special group is created below every segment group: "segment". This
 * group contains some variables:
 * <ul>
 *   <li>
 *     <i>pos</i> holding the start position of the segment inside the
 *     buffer
 *   </li>
 *   <li>
 *     <i>length</i> holding the length of the area occupied by the segment
 *   </li>
 * </ul>
 * The data of every segment is simply added to the database, so there may
 * be multiple groups with the same name if a given segment type occurs more
 * often than once.
 * @return 0 if ok, -1 on error and 1 if no segment was available
 * @param gtype typename for segments (most callers use "SEG")
 * @param mbuf GWEN_BUFFER containing the message. Parsing is started at
 * the current position within the buffer, so please make sure that this
 * pos is set to the beginning of the message before calling this function.
 * @param gr database to store information parsed from the given message
 */
int GWEN_MsgEngine_ReadMessage(GWEN_MSGENGINE *e,
                               const char *gtype,
                               GWEN_BUFFER *mbuf,
                               GWEN_DB_NODE *gr);


#ifdef __cplusplus
}
#endif

#endif

