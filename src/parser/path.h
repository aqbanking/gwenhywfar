/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue Sep 09 2003
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


#ifndef GWENHYFWAR_PATH_H
#define GWENHYFWAR_PATH_H

#include <gwenhyfwar/gwenhyfwarapi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Path Flags
 *
 * The path flags only use the lower word of the integer. The high word
 * may be used/interpreted by the called function.
 */
/*@{*/
/**
 * if this is set then all elements of the path except the last one
 * must exist.
 */
#define GWEN_PATH_FLAGS_PATHMUSTEXIST          0x00000001
/**
 * if this is set then none of the elements of the path except the last
 * one must exist.
 */
#define GWEN_PATH_FLAGS_PATHMUSTNOTEXIST       0x00000002

/**
 * if this bit is set then the whole path will be created. This may lead
 * to double entries at any part of the path.
 */
#define GWEN_PATH_FLAGS_PATHCREATE             0x00000004

/**
 * if this bit is set then the last element of the path MUST exist.
 * This implies @ref GWEN_PATH_FLAGS_PATHMUSTEXIST
 */
#define GWEN_PATH_FLAGS_NAMEMUSTEXIST          0x00000008

/**
 * if this bit is set then the last element of the path MUST NOT exist.
 */
#define GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST       0x00000010

/**
 * if this bit is set then the last element of the path is created in any
 * case (this is for groups).
 * This may lead to double entries of the last element.
 */
#define GWEN_PATH_FLAGS_CREATE_GROUP            0x00000020

/**
 * if this bit is set then the last element of the path is created in any
 * case (this is for variables).
 * This may lead to double entries of the last element.
 */
#define GWEN_PATH_FLAGS_CREATE_VAR              0x00000040


/**
 * a variable is wanted (if this bit is 0 then a group is wanted).
 * This flag is used internally, too. When the path handler function
 * is called by @ref GWEN_Path_Handle then this flag is modified
 * to reflect the type of the current path element.
 */
#define GWEN_PATH_FLAGS_VARIABLE                0x00000080

/**
 * @internal
 */
#define GWEN_PATH_FLAGS_INTERNAL               0x0000f000

/**
 * @internal
 * this is flagged for the path function. If this is set then the
 * element given is the last one, otherwise it is not.
 */
#define GWEN_PATH_FLAGS_LAST                   0x00001000

/*@}*/



GWENHYFWAR_API
  typedef void* (*GWEN_PATHHANDLERPTR)(const char *entry,
				       void *data,
				       unsigned int flags);


GWENHYFWAR_API
  void *GWEN_Path_Handle(const char *path,
			 void *data,
			 unsigned int flags,
			 GWEN_PATHHANDLERPTR elementFunction);



#ifdef __cplusplus
}
#endif


#endif /* GWENHYFWAR_PATH_H */

