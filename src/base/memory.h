/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
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


#ifndef GWENHYWFAR_MEMORY_H
#define GWENHYWFAR_MEMORY_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



#ifdef __cplusplus
extern "C" {
#endif


  /* this is taken from the system header file assert.h and
   * and modified by me (Martin Preuss).
   */
# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define GWEN_LOCATION_FUNCTION	__PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define GWEN_LOCATION_FUNCTION	__func__
#  else
#   define GWEN_LOCATION_FUNCTION	((__const char *) "unknown function")
#  endif
# endif


  GWENHYWFAR_API 
  GWEN_ERRORCODE GWEN_Memory_ModuleInit();

  GWENHYWFAR_API 
  GWEN_ERRORCODE GWEN_Memory_ModuleFini();

  GWENHYWFAR_API 
  void GWEN_Memory_Report();


  GWENHYWFAR_API 
  void *GWEN_Memory_NewObject(void *p,
                              const char *typeName,
                              const char *function,
                              const char *file,
                              int line);

  GWENHYWFAR_API 
  void GWEN_Memory_FreeObject(void *object,
                              const char *function,
                              const char *file,
                              int line);

  GWENHYWFAR_API 
  void GWEN_Memory_AttachObject(void *object,
                                const char *function,
                                const char *file,
                                int line);



#define GWEN_NEW_OBJECT(typ, varname) \
  varname=(typ*)malloc(sizeof(typ));\
  assert(varname); \
  memset((void*)varname, 0, sizeof(typ));

#define GWEN_FREE_OBJECT(varname) \
  free(varname)


#define GWEN_NEW(typ, function) \
  ((typ*)GWEN_Memory_NewObject(function,\
  __STRING(typ),\
  GWEN_LOCATION_FUNCTION, \
  __FILE__,\
  __LINE__))

#define GWEN_FREE(varname) \
  GWEN_Memory_FreeObject((void*)varname,\
  GWEN_LOCATION_FUNCTION,\
  __FILE__,\
  __LINE__)

#define GWEN_ATTACH(varname) \
  GWEN_Memory_AttachObject((void*)varname,\
  GWEN_LOCATION_FUNCTION,\
  __FILE__,\
  __LINE__)


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_MEMORY_H */

