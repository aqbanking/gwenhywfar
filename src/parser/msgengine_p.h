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

#ifndef CHAMELEON_MSGENGINE_P_H
#define CHAMELEON_MSGENGINE_P_H

#include <chameleon/chameleon.h>
#include <chameleon/msgengine.h>
#include <chameleon/conf.h>
#include <chameleon/stringlist.h>


#define MSGENGINE_CHARSTOESCAPE ":+\"\'"
#define MSGENGINE_DEFAULT_GROUPDELIM ":"
#define MSGENGINE_DEFAULT_DELIMITERS ":+\"\'"
#define MSGENGINE_VARNAME_WIDTH  30
#define MSGENGINE_TYPENAME_WIDTH 10

#define MSGENGINE_SHOW_FLAGS_OPTIONAL 0x00010000


struct _MSGENGINE {
  XMLNODE *defs;
  char escapeChar;
  char *charsToEscape;
  char *delimiters;
  char *secMode;
  unsigned int cfgMode;
  CONFIGGROUP *globalValues;
  MSGENGINE_FORMAT msgFormat;
  MSGENGINE_TYPECHECK_PTR typeCheckPtr;
  MSGENGINE_TYPEREAD_PTR typeReadPtr;
  MSGENGINE_TYPEWRITE_PTR typeWritePtr;

  MSGENGINE_BINTYPEREAD_PTR binTypeReadPtr;
  MSGENGINE_BINTYPEWRITE_PTR binTypeWritePtr;

  void *inheritorData;
};


int MsgEngine__CheckValue(MSGENGINE *e,
			  const char *value,
			  XMLNODE *node,
			  char escapeChar);

int MsgEngine__WriteValue(MSGENGINE *e,
			  char *buffer,
			  unsigned int size,
			  unsigned int *pos,
			  const char *value,
			  XMLNODE *node);

int MsgEngine__WriteElement(MSGENGINE *e,
			    char *buffer,
			    unsigned int size,
			    unsigned int *pos,
			    XMLNODE *node,
			    XMLNODE *rnode,
			    CONFIGGROUP *gr,
			    int loopNr,
			    int isOptional);

int MsgEngine__WriteGroup(MSGENGINE *e,
			  char *buffer,
			  unsigned int size,
			  unsigned int *pos,
			  XMLNODE *node,  /** format node */
			  XMLNODE *rnode, /** reference node */
			  CONFIGGROUP *gr,
			  int groupIsOptional);

int MsgEngine__ShowElement(MSGENGINE *e,
			   const char *path,
			   XMLNODE *node,
			   STRINGLIST *sl,
			   unsigned int flags);

int MsgEngine__ShowGroup(MSGENGINE *e,
			 const char *path,
			 XMLNODE *node,
			 XMLNODE *rnode,
			 STRINGLIST *sl,
			 unsigned int flags);

int MsgEngine__ReadValue(MSGENGINE *e,
			 const char *msg,
			 unsigned int msgSize,
			 unsigned int *pos,
			 XMLNODE *node,
			 char *buffer,
			 int bufsize,
			 const char *delimiters);

int MsgEngine__ReadGroup(MSGENGINE *e,
			 const char *msg,
			 unsigned int msgSize,
			 unsigned int *pos,
			 XMLNODE *node,
			 XMLNODE *rnode,
			 CONFIGGROUP *gr,
			 const char *delimiters);
const char *MsgEngine__findInValues(MSGENGINE *e,
				    XMLNODE *node,
				    const char *name);

const char  *MsgEngine__TransformValue(MSGENGINE *e,
				       const char *pvalue,
				       XMLNODE *node);
const char *MsgEngine__SearchForValue(MSGENGINE *e,
				      XMLNODE *node,
				      XMLNODE *refnode,
				      const char *name);


#endif

