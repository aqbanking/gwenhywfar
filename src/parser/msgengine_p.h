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

#ifndef GWENHYFWAR_MSGENGINE_P_H
#define GWENHYFWAR_MSGENGINE_P_H

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/msgengine.h>
#include <gwenhyfwar/db.h>
#include <gwenhyfwar/stringlist.h>
#include <gwenhyfwar/buffer.h>


#define GWEN_MSGENGINE_CHARSTOESCAPE ":+\"\'"
#define GWEN_MSGENGINE_DEFAULT_GROUPDELIM ":"
#define GWEN_MSGENGINE_DEFAULT_DELIMITERS ":+\"\'"
#define GWEN_MSGENGINE_VARNAME_WIDTH  30
#define GWEN_MSGENGINE_TYPENAME_WIDTH 10

#define GWEN_MSGENGINE_SHOW_FLAGS_OPTIONAL 0x00010000
#define GWEN_MSGENGINE_TRUSTEDDATA_MAXPOS  32


struct GWEN__MSGENGINE {
  GWEN_XMLNODE *defs;
  char escapeChar;
  char *charsToEscape;
  char *delimiters;
  char *secMode;
  unsigned int cfgMode;
  GWEN_DB_NODE *globalValues;
  GWEN_MSGENGINE_FORMAT msgFormat;
  GWEN_MSGENGINE_TYPECHECK_PTR typeCheckPtr;
  GWEN_MSGENGINE_TYPEREAD_PTR typeReadPtr;
  GWEN_MSGENGINE_TYPEWRITE_PTR typeWritePtr;

  GWEN_MSGENGINE_BINTYPEREAD_PTR binTypeReadPtr;
  GWEN_MSGENGINE_BINTYPEWRITE_PTR binTypeWritePtr;

  GWEN_MSGENGINE_TRUSTEDDATA *trustInfos;
  void *inheritorData;
};


int GWEN_MsgEngine__CheckValue(GWEN_MSGENGINE *e,
                               const char *value,
                               GWEN_XMLNODE *node,
                               char escapeChar);

int GWEN_MsgEngine__WriteValue(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_BUFFER *data,
                               GWEN_XMLNODE *node);

int GWEN_MsgEngine__WriteElement(GWEN_MSGENGINE *e,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_XMLNODE *node,
                                 GWEN_XMLNODE *rnode,
                                 GWEN_DB_NODE *gr,
                                 int loopNr,
                                 int isOptional);

int GWEN_MsgEngine__WriteGroup(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_XMLNODE *node,  /** format node */
                               GWEN_XMLNODE *rnode, /** reference node */
                               GWEN_DB_NODE *gr,
                               int groupIsOptional);

int GWEN_MsgEngine__ShowElement(GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                unsigned int flags);

int GWEN_MsgEngine__ShowGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_STRINGLIST *sl,
                              unsigned int flags);

int GWEN_MsgEngine__ListElement(GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                GWEN_XMLNODE *listNode,
                                unsigned int flags);

int GWEN_MsgEngine__ListGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_STRINGLIST *sl,
                              GWEN_XMLNODE *listNode,
                              unsigned int flags);

int GWEN_MsgEngine__ReadValue(GWEN_MSGENGINE *e,
                              GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_BUFFER *vbuf,
                              const char *delimiters,
                              unsigned int flags);

int GWEN_MsgEngine__ReadGroup(GWEN_MSGENGINE *e,
                              GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_DB_NODE *gr,
                              const char *delimiters,
                              unsigned int flags);

/**
 * @param dnode node of the element to transform
 */
const char *GWEN_MsgEngine__findInValues(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         GWEN_XMLNODE *dnode,
                                         const char *name,
                                         unsigned int *datasize);

const char  *GWEN_MsgEngine__TransformValue(GWEN_MSGENGINE *e,
                                            const char *pvalue,
                                            GWEN_XMLNODE *node,
                                            GWEN_XMLNODE *dnode,
                                            unsigned int *datasize);
const char *GWEN_MsgEngine__SearchForValue(GWEN_MSGENGINE *e,
                                           GWEN_XMLNODE *node,
                                           GWEN_XMLNODE *refnode,
                                           const char *name,
                                           unsigned int *datasize);
int GWEN_MsgEngine__IsCharTyp(const char *type);
int GWEN_MsgEngine__IsIntTyp(const char *type);
int GWEN_MsgEngine__IsBinTyp(const char *type);

int GWEN_MsgEngine_GetHighestTrustLevel(GWEN_XMLNODE *node,
                                        GWEN_XMLNODE *refnode);


struct GWEN_MSGENGINE_TRUSTEDDATA {
  GWEN_MSGENGINE_TRUSTEDDATA *next;
  char *data;
  unsigned int size;
  char *description;
  unsigned int trustLevel;
  char *replacement;
  unsigned int positions[GWEN_MSGENGINE_TRUSTEDDATA_MAXPOS];
  unsigned int posCount;
  unsigned int posPointer;
};



#endif

