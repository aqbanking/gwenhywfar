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

/** @file misc.h
 *
 * @short This file contains some macros concerning lists and inheritance.
 *
 * <p>
 * FIRST: Yes, I DO know, macros are very, very bad.
 * When writing these macros I spent much time debugging them, because the
 * compiler is not much of a help here.
 * The validity of a macro is only checked upon invocation,
 * so if you never use a macro it will never be checked.
 * </p>
 * <p>
 * However, these macros do work just fine and they make some tasks
 * much easier to handle.
 * </p>
 * <p>
 * The reason for using macros is the lack of templates in C.
 * When writing Gwenhywfar I often faced the fact that some functions always
 * appear with many structs defined. The only difference is the name of those
 * functions and the type of the arguments.
 * </p>
 * <p>
 * The best example is the handling of lists of structs.
 * In most listable structs there was a variable called @b next which pointed
 * to the next object in the list. There were also functions like TYPE_next(),
 * TYPE_add(), TYPE_del() etc for list handling. Whenever I improved the list
 * mechanism I had to change ALL code files in order to improve them all.
 * </p>
 * <p>
 * These macros are now used to facilitate improvements in list or inheritance
 * handling code in C.
 * </p>
 * <p>
 * @b NOTE: Please do not change these macros unless you know exactly what you
 * are doing!
 * Bugs in the macros will most probably lead to nearly undebuggable results
 * in code files using them.<br>
 * You have been warned ;-)
 * </p>
 *
 */

#ifndef GWENHYWFAR_MISC_H
#define GWENHYWFAR_MISC_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifdef __cplusplus
extern "C" {
#endif

#define GWEN_NEW_OBJECT(typ, varname) \
  varname=(typ *)malloc(sizeof(typ)); \
  assert(varname); \
  memset(varname,0,sizeof(typ));


  /** @defgroup GWEN_MACRO_LIST Macros For Typesafe List Handling
   *
   */
  /*@{*/
#define GWEN_LIST_ADD(typ, sr, head) {\
  typ *curr;                \
                            \
  assert(sr);               \
  assert(head);             \
                            \
  curr=*head;               \
  if (!curr) {              \
    *head=sr;               \
  }                         \
  else {                    \
    while(curr->next) {     \
      curr=curr->next;      \
    }                       \
    curr->next=sr;          \
  }\
  }


#define GWEN_LIST_INSERT(typ, sr, head) {\
  typ *curr;                \
                            \
  assert(sr);               \
  assert(head);             \
                            \
  curr=*head;               \
  if (!curr) {              \
    *head=sr;               \
  }                         \
  else {                    \
    sr->next=curr;\
    *head=sr;\
  }\
  }


#define GWEN_LIST_DEL(typ, sr, head) {\
  typ *curr;                   \
                               \
  assert(sr);                  \
  assert(head);                \
  curr=*head;                  \
  if (curr) {                  \
    if (curr==sr) {            \
      *head=curr->next;        \
    }                          \
    else {                     \
      while(curr->next!=sr) {  \
	curr=curr->next;       \
      }                        \
      if (curr)                \
	curr->next=sr->next;   \
    }                          \
  }                            \
  sr->next=0;\
  }


  /**
   * Use this inside the declaration of a struct for which you want to create
   * lists.
   */
#define GWEN_LIST_ELEMENT(t) \
  t *next; \
  t##_LIST *listPtr;

  /**
   * Use this in public header files to define some prototypes for list
   * functions.
   * Let's assume the type of your list elements is "MYTYPE" and you want to
   * use the prefix "MyType_" for the list functions.
   * The following function prototypes will then be created:
   * <ul>
   *  <li>
   *    void MyType_List_Add(MYTYPE *element, MYTYPE_LIST *list);<br>
   *    Adds a MYTYPE struct to the given list.
   *  </li>
   *  <li>
   *    void MyType_List_Del(MYTYPE *element);<br>
   *    Removes a MyType struct from the list it is enlisted to.
   *  </li>
   *  <li>
   *    MYTYPE *MyType_List_First(MYTYPE *element); <br>
   *    Returns the first member of the given list.
   *  </li>
   *  <li>
   *    MYTYPE* MyType_List_Next(const MYTYPE *element);<br>
   *    Returns a pointer to the object followed by the given one.
   *  </li>
   *  <li>
   *    void MyType_List_Clear(MYTYPE *element); <br>
   *    Frees all entries of the given list.
   *    This function assumes that there is a function Mytype_free().
   *  </li>
   *  <li>
   *    MYTYPE_LIST *MyType_List_new(); <br>
   *    Creates a new list of elements of MYTYPE type.
   *  </li>
   *  <li>
   *    void MyType_List_free(MYTYPE_LIST *l); <br>
   *    Clears and frees a list of elements of MYTYPE type.
   *    All objects inside the list are freed.
   *  </li>
   * </ul>
   */
#define GWEN_LIST_FUNCTION_DEFS(t, pr) \
  typedef struct t##_LIST {\
  t *first;\
  GWEN_TYPE_UINT32 count;\
  } t##_LIST; \
  void pr##_List_Add(t *element, t##_LIST *list); \
  void pr##_List_Insert(t *element, t##_LIST *list); \
  void pr##_List_Del(t *element); \
  t* pr##_List_First(const t##_LIST *l); \
  void pr##_List_Clear(t##_LIST *l); \
  t##_LIST* pr##_List_new(); \
  void pr##_List_free(t##_LIST *l); \
  t* pr##_List_Next(const t *element); \
  GWEN_TYPE_UINT32 pr##_List_GetCount(const t##_LIST *l);


  /**
   * Use this inside your code files (*.c).
   * Actually implements the functions for which the prototypes have been
   * defined via @ref GWEN_LIST_FUNCTION_DEFS.
   */
#define GWEN_LIST_FUNCTIONS(t, pr) \
  void pr##_List_Add(t *element, t##_LIST *l) { \
  assert(l); \
  GWEN_LIST_ADD(t, element, &(l->first)); \
  element->listPtr=l;\
  l->count++;\
  }; \
  \
  void pr##_List_Insert(t *element, t##_LIST *l) { \
  assert(l); \
  GWEN_LIST_INSERT(t, element, &(l->first)); \
  element->listPtr=l;\
  l->count++;\
  }; \
  \
  void pr##_List_Del(t *element){ \
  assert(element->listPtr);\
  assert(element->listPtr->first); \
  assert(element->listPtr->count);\
  GWEN_LIST_DEL(t, element, &(element->listPtr->first)); \
  element->listPtr->count--;\
  element->listPtr=0;\
  };\
  \
  t* pr##_List_First(const t##_LIST *l) { \
  if (l) return l->first;\
  else return 0; \
  }; \
  \
  void pr##_List_Clear(t##_LIST *l) { \
  t* el; \
  while(l->first) {\
  el=l->first;\
  pr##_List_Del(el);\
  pr##_free(el);\
  } /* while */ \
  }; \
  \
  t##_LIST* pr##_List_new(){\
  t##_LIST *l; \
  GWEN_NEW_OBJECT(t##_LIST, l);\
  return l;\
  }\
  \
  void pr##_List_free(t##_LIST *l) {\
  if (l) pr##_List_Clear(l);\
  free(l);\
  } \
  \
  t* pr##_List_Next(const t *element) { \
  assert(element);\
  return element->next;\
  }; \
  \
  GWEN_TYPE_UINT32 pr##_List_GetCount(const t##_LIST *l){\
  assert(l);\
  return l->count;\
  }



  /**
   * Use this in your code file (*.c) inside the init code for the struct
   * you want to use in lists (in GWEN these are the functions which end with
   * "_new".
   */
#define GWEN_LIST_INIT(t, element) element->listPtr=0;


  /**
   * Use this in your code file (*.c) inside the fini code for the struct
   * you want to use in lists (in GWEN these are the functions which end with
   * "_free".
   */
#define GWEN_LIST_FINI(t, element) \
  if (element) { \
  if (element->listPtr)\
  GWEN_LIST_DEL(t, element, &(element->listPtr->first)); \
  }
  /*@}*/ /* defgroup */




#ifdef __cplusplus
}
#endif

#endif



