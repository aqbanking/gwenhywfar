/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Apr 24 2004
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

#ifndef GWENHYWFAR_ARGS_H
#define GWENHYWFAR_ARGS_H

#include <gwenhywfar/gwenhywfarapi.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct GWEN_ARGS GWEN_ARGS;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/types.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif


#define GWEN_ARGS_FLAGS_HAS_ARGUMENT     0x00000001
#define GWEN_ARGS_FLAGS_LAST             0x00000002
#define GWEN_ARGS_FLAGS_HELP             0x00000004

#define GWEN_ARGS_MODE_ALLOW_FREEPARAM   0x00000001
#define GWEN_ARGS_MODE_STOP_AT_FREEPARAM 0x00000002

#define GWEN_ARGS_RESULT_ERROR (-1)
#define GWEN_ARGS_RESULT_HELP  (-2)


typedef enum {
  GWEN_ArgsTypeChar=0,
  GWEN_ArgsTypeInt
} GWEN_ARGS_TYPE;

typedef enum {
  GWEN_ArgsOutTypeTXT=0,
  GWEN_ArgsOutTypeHTML
} GWEN_ARGS_OUTTYPE;


/**
 * This is one of the very few structs inside Gwenhywfar whose
 * contents are available for direct access to the the program.
 * Developer's note: Please note that any change within this struct will
 * make it necessary to increment the SO_VERSION of the library !
 */
GWENHYWFAR_API
struct GWEN_ARGS {
  GWEN_TYPE_UINT32 flags;
  GWEN_ARGS_TYPE type;
  const char *name;
  unsigned int minNum;
  unsigned int maxNum;
  const char *shortOption;
  const char *longOption;
  const char *shortDescription;
  const char *longDescription;
};


/**
 * This function parses the given argument list.
 * Known options are stored within the given DB under their respective name.
 * Free parameters (which are arguments without leading "-"'s) are stored in
 * the variable "params" of the given db.
 */
GWENHYWFAR_API
int GWEN_Args_Check(int argc, char **argv,
		    int startAt,
		    GWEN_TYPE_UINT32 mode,
		    const GWEN_ARGS *args,
		    GWEN_DB_NODE *db);

GWENHYWFAR_API
int GWEN_Args_Usage(const GWEN_ARGS *args, GWEN_BUFFER *ubuf,
                    GWEN_ARGS_OUTTYPE ot);

GWENHYWFAR_API
int GWEN_Args_ShortUsage(const GWEN_ARGS *args, GWEN_BUFFER *ubuf,
                         GWEN_ARGS_OUTTYPE ot);

#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_ARGS_H */



