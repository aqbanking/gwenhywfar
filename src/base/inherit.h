/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Dec 05 2003
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

#ifndef GWENHYWFAR_INHERIT_H
#define GWENHYWFAR_INHERIT_H

typedef struct GWEN_INHERITDATA GWEN_INHERITDATA;


#include <gwenhywfar/misc.h>
#include <gwenhywfar/gwenhywfarapi.h>


#ifdef __cplusplus
extern "C" {
#endif


  /** @defgroup GWEN_MACRO_INHERIT Macros For Typesafe Inheritance
   *
   */
  /*@{*/
  GWENHYWFAR_API 
  typedef void (*GWEN_INHERIT_FREEDATAFN)(void *baseData,
                                          void *data);

  GWEN_LIST_FUNCTION_DEFS(GWEN_INHERITDATA, GWEN_InheritData);

  GWENHYWFAR_API 
  GWEN_INHERITDATA *GWEN_InheritData_new(const char *t,
                                         GWEN_TYPE_UINT32 id,
                                         void *data,
                                         void *baseData,
                                         GWEN_INHERIT_FREEDATAFN fn);
  GWENHYWFAR_API 
  void GWEN_InheritData_free(GWEN_INHERITDATA *d);

  GWENHYWFAR_API 
  const char *GWEN_InheritData_GetTypeName(const GWEN_INHERITDATA *d);

  GWENHYWFAR_API 
  GWEN_TYPE_UINT32 GWEN_InheritData_GetId(const GWEN_INHERITDATA *d);

  GWENHYWFAR_API 
  void *GWEN_InheritData_GetData(const GWEN_INHERITDATA *d);

  GWENHYWFAR_API 
  void *GWEN_InheritData_GetFreeDataFn(const GWEN_INHERITDATA *d);

  GWENHYWFAR_API 
  GWEN_TYPE_UINT32 GWEN_Inherit_MakeId(const char *typeName);

  GWENHYWFAR_API 
  void* GWEN_Inherit_FindData(GWEN_INHERITDATA_LIST *l,
                              GWEN_TYPE_UINT32 id,
                              int wantCreate);

  /** @name Macros To Be Used In Inherited Classes - Header Files
   *
   */
  /*@{*/
  /**
   * Use this macro inside the struct which you want to make inheritable.
   * This macro defines some new elements for the struct for administration
   * of inheritance.
   */
#define GWEN_INHERIT_ELEMENT(t) \
  GWEN_INHERITDATA_LIST *INHERIT__list;

  /**
   * Use this macro in the header file of the inherited class. This defines
   * the prototypes of some inheritance functions.
   * You should not care about these functions here, since you should not use
   * them directly. Please use @ref GWEN_INHERIT_GETDATA and
   * @ref GWEN_INHERIT_SETDATA instead.
   */
#define GWEN_INHERIT_FUNCTION_DEFS(t) \
  void t##__INHERIT_SETDATA(t *element, \
                            const char *typeName,\
                            GWEN_TYPE_UINT32 id,\
                            void *data,\
                            GWEN_INHERIT_FREEDATAFN f);\
  int t##__INHERIT_ISOFTYPE(t *element, GWEN_TYPE_UINT32 id);\
  GWEN_INHERITDATA_LIST *t##__INHERIT_GETLIST(const t *element);

  /*@}*/


  /** @name Macros To Be Used In Inherited Classes - C Files
   *
   */
  /*@{*/
  /**
   * Use this macro in the C file of the inherited class. It defines the
   * implementations of the inheritance functions. This macro MUST be
   * placed after the include statement which includes the classes header
   * file.
   */
#define GWEN_INHERIT_FUNCTIONS(t) \
  GWEN_INHERITDATA_LIST *t##__INHERIT_GETLIST(const t *element) {\
  assert(element);\
  return element->INHERIT__list;\
  }\
  \
  void t##__INHERIT_SETDATA(t *element, \
                            const char *typeName,\
                            GWEN_TYPE_UINT32 id,\
                            void *data,\
                            GWEN_INHERIT_FREEDATAFN f) {\
  GWEN_INHERITDATA *d;\
  \
  assert(element);\
  assert(element->INHERIT__list);\
  \
  d=GWEN_Inherit_FindData(element->INHERIT__list, id, 1);\
  if (d) {\
    fprintf(stderr,\
            "ERROR: Type \"%s\" already inherits base type\n",\
            typeName);\
    abort();\
  }\
  d=GWEN_InheritData_new(typeName, id, data, (void*)element, f);\
  GWEN_InheritData_List_Insert(d, element->INHERIT__list);\
  }\
  \
  int t##__INHERIT_ISOFTYPE(t *element, GWEN_TYPE_UINT32 id) {\
  assert(element);\
  assert(element->INHERIT__list);\
  \
  return (GWEN_Inherit_FindData(element->INHERIT__list, id, 1)!=0);\
  }

  /**
   * Use this macro in your C file in constructor functions for the inherited
   * class. This macro initializes the elements defined by the macro
   * @ref GWEN_INHERIT_ELEMENT.
   */
#define GWEN_INHERIT_INIT(t, element) \
  assert(element);\
  element->INHERIT__list=GWEN_InheritData_List_new();


  /**
   * Use this macro in your C file in destructor functions for the inherited
   * class. This macro deinitializes the elements defined by the macro
   * @ref GWEN_INHERIT_ELEMENT. This should be the first instruction in that
   * function, because it also gives inheriting classes the opportunity to
   * free their own data associated with the given element. It causes the
   * least problems if inheriting classes free their data before the base
   * class does.
   */
#define GWEN_INHERIT_FINI(t, element) \
  assert(element);\
  GWEN_InheritData_List_free(element->INHERIT__list);

  /*@}*/

  /** @name Macros To Be Used In Inheriting Classes
   *
   */
  /*@{*/
  /**
   * Use this in the C file of inheriting classes. It initializes a global
   * variable with a hash of the inheriting type name. This is used to speed
   * up inmheritance functions. This variable will be filled with a value
   * upon the first invocation of the macro @ref GWEN_INHERIT_SETDATA.
   */
#define GWEN_INHERIT(bt, t) \
  GWEN_TYPE_UINT32 t##__INHERIT_ID=0;

  /**
   * This macros returns the private data of an inheriting class associated
   * with an element of its base class.
   */
#define GWEN_INHERIT_GETDATA(bt, t, element) \
  ((t*)GWEN_Inherit_FindData(bt##__INHERIT_GETLIST(element),t##__INHERIT_ID,0));

  /**
   * This macro sets the private data of an inheriting class associated
   * with an element of its base class. The last argument is a pointer to a
   * function which frees the associated data. That function will be called
   * when the element of the base class given is freed or new data is to be
   * associated with the element.
   * The prototype of that function is this:
   * @code
   * typedef void (*function)(void *baseData, void *data);
   * @endcode
   * Please note that the argument to that function is a pointer to the
   * base type element. If you want to get the private data associated with
   * the base type element (and you probably do) you must call
   * @ref GWEN_INHERIT_GETDATA.
   * Every time the macro @ref GWEN_INHERIT_SETDATA is used the previously
   * associated data will be freed by calling the function whose prototype
   * you've just learned.
   */
#define GWEN_INHERIT_SETDATA(bt, t, element, data, fn) \
  if (!t##__INHERIT_ID)\
  t##__INHERIT_ID=GWEN_Inherit_MakeId(__STRING(t));\
  bt##__INHERIT_SETDATA(element, __STRING(t), t##__INHERIT_ID, data, fn);

  /**
   * This macro checks whether the given element is of the given type.
   * @return !=0 if the pointer is of the expected type, 0 otherwise
   * @param bt base type
   * @param t derived type
   * @param element pointer which is to be checked
   */
#define GWEN_INHERIT_ISOFTYPE(bt, t, element) \
  ((bt##__INHERIT_ISOFTYPE(element,\
                           ((t##__INHERIT_ID==0)?\
                            ((t##__INHERIT_ID=GWEN_Inherit_MakeId(__STRING(t)))):\
                            t##__INHERIT_ID)))?1:0)
  /*@}*/

  /*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif



#endif /* GWENHYWFAR_INHERIT_P_H */



