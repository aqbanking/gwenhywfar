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

int GWEN_MsgEngine__ReadValue(GWEN_MSGENGINE *e,
                              const char *msg,
                              unsigned int msgSize,
                              unsigned int *pos,
                              GWEN_XMLNODE *node,
                              char *buffer,
                              int *bufsize,
                              const char *delimiters);

int GWEN_MsgEngine__ReadGroup(GWEN_MSGENGINE *e,
                              const char *msg,
                              unsigned int msgSize,
                              unsigned int *pos,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_DB_NODE *gr,
                              const char *delimiters);
const char *GWEN_MsgEngine__findInValues(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         const char *name);

const char  *GWEN_MsgEngine__TransformValue(GWEN_MSGENGINE *e,
                                            const char *pvalue,
                                            GWEN_XMLNODE *node);
const char *GWEN_MsgEngine__SearchForValue(GWEN_MSGENGINE *e,
                                           GWEN_XMLNODE *node,
                                           GWEN_XMLNODE *refnode,
                                           const char *name,
                                           unsigned int *datasize);
int GWEN_MsgEngine__IsCharTyp(const char *type);
int GWEN_MsgEngine__IsIntTyp(const char *type);
int GWEN_MsgEngine__IsBinTyp(const char *type);


#endif

