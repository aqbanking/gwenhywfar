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

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_MSGENGINE_ALL Message Engine
 * @ingroup MOD_PARSER
 *
 * This group contains function providing creating and parsing messages
 * based on an XML-alike file.
 */
/*@{*/

/** @defgroup MOD_TRUSTDATA Trust Data Handling
 *
 */
/*@{*/
typedef struct GWEN_MSGENGINE_TRUSTEDDATA GWEN_MSGENGINE_TRUSTEDDATA;
typedef enum {
  GWEN_MsgEngineTrustLevelNone=0,
  GWEN_MsgEngineTrustLevelLow,
  GWEN_MsgEngineTrustLevelMedium,
  GWEN_MsgEngineTrustLevelHigh,
  GWEN_MsgEngineTrustLevelFull
} GWEN_MSGENGINE_TRUSTLEVEL;


GWEN_MSGENGINE_TRUSTEDDATA*
  GWEN_MsgEngine_TrustedData_new(const char *data,
                                 unsigned int size,
                                 const char *description,
                                 GWEN_MSGENGINE_TRUSTLEVEL trustLevel);
void GWEN_MsgEngine_TrustedData_free(GWEN_MSGENGINE_TRUSTEDDATA *td);

GWEN_MSGENGINE_TRUSTEDDATA*
  GWEN_MsgEngine_TrustedData_GetNext(GWEN_MSGENGINE_TRUSTEDDATA *td);

const char*
  GWEN_MsgEngine_TrustedData_GetData(GWEN_MSGENGINE_TRUSTEDDATA *td);

unsigned int
  GWEN_MsgEngine_TrustedData_GetSize(GWEN_MSGENGINE_TRUSTEDDATA *td);

const char*
  GWEN_MsgEngine_TrustedData_GetDescription(GWEN_MSGENGINE_TRUSTEDDATA *td);

GWEN_MSGENGINE_TRUSTLEVEL
  GWEN_MsgEngine_TrustedData_GetTrustLevel(GWEN_MSGENGINE_TRUSTEDDATA *td);

const char*
  GWEN_MsgEngine_TrustedData_GetReplacement(GWEN_MSGENGINE_TRUSTEDDATA *td);


int GWEN_MsgEngine_TrustedData_AddPos(GWEN_MSGENGINE_TRUSTEDDATA *td,
                                      unsigned int pos);

int GWEN_MsgEngine_TrustedData_GetFirstPos(GWEN_MSGENGINE_TRUSTEDDATA *td);

int GWEN_MsgEngine_TrustedData_GetNextPos(GWEN_MSGENGINE_TRUSTEDDATA *td);

int
  GWEN_MsgEngine_TrustedData_CreateReplacements(GWEN_MSGENGINE_TRUSTEDDATA
                                                *td);
/*@}*/ /* defgroup */



#define GWEN_MSGENGINE_SHOW_FLAGS_NOSET 0x0001
#define GWEN_MSGENGINE_MAX_VALUE_LEN    8192

/** @name Read Flags
 */
/*@{*/
#define GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO 0x0001
#define GWEN_MSGENGINE_READ_FLAGS_DEFAULT 0
/*@}*/


/** @defgroup MOD_MSGENGINE Message Engine
 *
 */
/*@{*/

typedef struct GWEN__MSGENGINE GWEN_MSGENGINE;

/** @name Virtual Functions
 *
 * A message engine contains some pointers to functions which allow
 * extending the functionality of a message engine (like virtual functions
 * in C++)
 */
/*@{*/
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
                                               GWEN_BUFFER *dbuf);
/*@}*/


/** @name Constructor And Destructor
 *
 */
/*@{*/
GWEN_MSGENGINE *GWEN_MsgEngine_new();
void GWEN_MsgEngine_free(GWEN_MSGENGINE *e);
/*@}*/

/** @name Setters And Getters
 *
 */
/*@{*/
void GWEN_MsgEngine_SetEscapeChar(GWEN_MSGENGINE *e, char c);
char GWEN_MsgEngine_GetEscapeChar(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetCharsToEscape(GWEN_MSGENGINE *e, const char *c);
const char *GWEN_MsgEngine_GetCharsToEscape(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetMode(GWEN_MSGENGINE *e, const char *mode);
const char *GWEN_MsgEngine_GetMode(GWEN_MSGENGINE *e);


unsigned int GWEN_MsgEngine_GetProtocolVersion(GWEN_MSGENGINE *e);
void GWEN_MsgEngine_SetProtocolVersion(GWEN_MSGENGINE *e,
                                       unsigned int p);


GWEN_XMLNODE *GWEN_MsgEngine_GetDefinitions(GWEN_MSGENGINE *e);
void GWEN_MsgEngine_SetDefinitions(GWEN_MSGENGINE *e,
                                   GWEN_XMLNODE *n,
                                   int take);

int GWEN_MsgEngine_AddDefinitions(GWEN_MSGENGINE *e,
                                  GWEN_XMLNODE *node);
/*@}*/


/** @name Setters For Virtual Functions: Extending Data Type Handling
 *
 */
/*@{*/
void GWEN_MsgEngine_SetTypeReadFunction(GWEN_MSGENGINE *e,
                                        GWEN_MSGENGINE_TYPEREAD_PTR p);
GWEN_MSGENGINE_TYPEREAD_PTR
  GWEN_MsgEngine_GetTypeReadFunction(GWEN_MSGENGINE *e);

void GWEN_MsgEngine_SetTypeWriteFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPEWRITE_PTR p);
GWEN_MSGENGINE_TYPEWRITE_PTR
  GWEN_MsgEngine_GetTypeWriteFunction(GWEN_MSGENGINE *e);
/*@}*/ /* Extending Data Type Handling */


/** @name Setters For Virtual Functions: Extending Binary Data Handling
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
/*@}*/  /* Extending Binary Data Handling */


/** @name Extending GWEN_MSGENGINE
 */
/*@{*/
void *GWEN_MsgEngine_GetInheritorData(GWEN_MSGENGINE *e);
void GWEN_MsgEngine_SetInheritorData(GWEN_MSGENGINE *e, void *d);
/*@}*/

/** @name Locating XML Nodes And Properties
 *
 */
/*@{*/
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
 * Searches for a property in "node" and in "refnode" and all its parents.
 * If topdown is 0 then the nearest value is used, otherwise the farest
 * one is used.
 */
const char *GWEN_MsgEngine_SearchForProperty(GWEN_XMLNODE *node,
                                             GWEN_XMLNODE *refnode,
                                             const char *name,
                                             int topDown);
/*@}*/

/** @name Getters And Setters for Global Variables
 *
 */
/*@{*/
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
/*@}*/


/** @name Parsing, Listing And Creating Messages
 *
 */
/*@{*/
int GWEN_MsgEngine_CreateMessage(GWEN_MSGENGINE *e,
                                 const char *msgName,
                                 int msgVersion,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_DB_NODE *msgData);

int GWEN_MsgEngine_CreateMessageFromNode(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         GWEN_BUFFER *gbuf,
                                         GWEN_DB_NODE *msgData);

/**
 * Deprecated, use @ref GWEN_MsgEngine_ListMessage instead.
 */
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
                                GWEN_DB_NODE *gr,
                                unsigned int flags);

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
 * @param e message engine
 * @param gtype typename for segments (most callers use "SEG")
 * @param mbuf GWEN_BUFFER containing the message. Parsing is started at
 * the current position within the buffer, so please make sure that this
 * pos is set to the beginning of the message before calling this function.
 * @param gr database to store information parsed from the given message
 * @param flags see @ref GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO and
 *  @ref GWEN_MSGENGINE_READ_FLAGS_DEFAULT
 */
int GWEN_MsgEngine_ReadMessage(GWEN_MSGENGINE *e,
                               const char *gtype,
                               GWEN_BUFFER *mbuf,
                               GWEN_DB_NODE *gr,
                               unsigned int flags);

/**
 * This function creates a full tree of all groups and elements
 * used by the given message.
 * The caller is responsible for freeing the data returned.
 */
GWEN_XMLNODE *GWEN_MsgEngine_ListMessage(GWEN_MSGENGINE *e,
                                         const char *typ,
                                         const char *msgName,
                                         int msgVersion,
                                         unsigned int flags);
/*@}*/


/** @name Handling Trust Information
 *
 */
/*@{*/
/**
 * This function returns trust info gathered while parsing a message.
 * The caller of this function takes over ownership of this list of
 * data, so it is his responsibility to free it.
 * @return list of trust data (0 if none)
 */
GWEN_MSGENGINE_TRUSTEDDATA *GWEN_MsgEngine_TakeTrustInfo(GWEN_MSGENGINE *e);


/**
 *
 */
int GWEN_MsgEngine_AddTrustInfo(GWEN_MSGENGINE *e,
                                const char *data,
                                unsigned int size,
                                const char *description,
                                GWEN_MSGENGINE_TRUSTLEVEL trustLevel,
                                unsigned int pos);

/*@}*/ /* Handling Trust Information */
/*@}*/ /* defgroup */
/*@}*/ /* defgroup (yes, twice) */


#ifdef __cplusplus
}
#endif

#endif

