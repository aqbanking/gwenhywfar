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


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <assert.h>


#ifndef GWEN_DUMMY_EMPTY_ARG
/** Necessary for MSVC compiler because it does not accept a left-out
    macro argument. */
# define GWEN_DUMMY_EMPTY_ARG
#endif


#ifndef GWEN_LIST1_H
#define GWEN_LIST1_H


#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup GWEN_MACRO_LIST Macros For Typesafe List Handling
 *
 * The macros of this group facilitates typesafe use of lists.
 *
 * <p>
 * Let's assume you have a structure type called MYSTRUCT and you want
 * to manage lists of them. Let's further assume that you want the
 * functions dealing with that struct have prefixes like MyStruct (as in
 * @b MyStruct_new)
 * </p>
 * The header file would look like this:
 *
 * @code
 *
 * / * mystruct.h * /
 *
 * #ifndef MYSTRUCT_H
 * #define MYSTRUCT_H
 *
 * typedef struct MYSTRUCT MYSTRUCT;
 *
 * GWEN_LIST_FUNCTION_DEFS(MYSTRUCT, MyStruct);
 *
 * struct {
 *   GWEN_LIST_ELEMENT(MYSTRUCT);
 *   int myData;
 * }
 *
 *
 * MYSTRUCT *MyStruct_new(int myData);
 * void MyStruct_free(MYSTRUCT *myStruct);
 *
 * #endif
 * @endcode
 * <p>
 * This defines all necessary data and function prototypes needed for
 * list management.
 * </p>
 *
 * <p>
 * The code file would look quite similar to the following:
 * </p>
 *
 * @code
 *
 * / * mystruct.c * /
 *
 * GWEN_LIST_FUNCTIONS(MYSTRUCT, MyStruct)
 *
 * MYSTRUCT *MyStruct_new(int myData) {
 *   MYSTRUCT *pMyStruct;
 *
 *   pMyStruct=(MYSTRUCT*)malloc(sizeof(MYSTRUCT));
 *   memset(pMyStruct, 0, sizeof(MYSTRUCT));
 *
 *   GWEN_LIST_INIT(MYSTRUCT, pMyStruct)
 *
 *   pMyStruct->myData=myData;
 *   return pMyStruct;
 * }
 *
 * void MyStruct_free(MYSTRUCT *pMyStruct) {
 *   if (pMyStruct) {
 *     pMyStruct->myData=0;
 *     GWEN_LIST_FINI(MYSTRUCT, pMyStruct)
 *     free(pMyStruct);
 *   }
 * }
 *
 * @endcode
 * Please note the three macros used in the code file:
 * <ul>
 *   <li>@ref GWEN_LIST_FUNCTIONS creates the functions for the list
 *       management</li>
 *   <li>@ref GWEN_LIST_INIT initializes the list data inside your
 *       struct to defined values </li>
 *   <li>@ref GWEN_LIST_FINI frees all ressources occupied by the list
 *       management code. Please note that this macro should be the last
 *       statement inside the destructor function before @b free()</li>
 * </ul>
 *
 * <p>Note: When writing these macro code lines, the original ISO
 * C89 standard for the C language does not allow terminating the
 * macro statement with a semicolon ';'. Any recent compiler will
 * probably silently ignore such an extra ';', but you should be
 * aware that this can cause problems once one of your users tries
 * to compile this with a different compiler. Therefore these code
 * lines should end directly with the closing parentheses.</p>
 * 
 * <p>
 * The list management code assumes that there is a function called
 * (in this example) @b MyStruct_free() (or generally: TYPEPREFIX_free).
 * This is used when destroying a list of MYSTRUCT elements. In this case
 * all elements still enlisted are destroyed upon destruction of the list.
 * </p>
 */
/*@{*/


typedef struct GWEN_LIST1 GWEN_LIST1;
typedef struct GWEN_LIST1_ELEMENT GWEN_LIST1_ELEMENT;


GWENHYWFAR_API
GWEN_LIST1 *GWEN_List1_new();

GWENHYWFAR_API
void GWEN_List1_free(GWEN_LIST1 *l);

GWENHYWFAR_API
int GWEN_List1_GetCount(const GWEN_LIST1 *l);

GWENHYWFAR_API
int GWEN_List1_Add(GWEN_LIST1 *l, GWEN_LIST1_ELEMENT *el);

GWENHYWFAR_API
int GWEN_List1_Insert(GWEN_LIST1 *l, GWEN_LIST1_ELEMENT *el);

GWENHYWFAR_API
int GWEN_List1_Del(GWEN_LIST1_ELEMENT *el);

GWENHYWFAR_API
int GWEN_List1_AddList(GWEN_LIST1 *dest, GWEN_LIST1 *l);

GWENHYWFAR_API
void *GWEN_List1_GetFirst(const GWEN_LIST1 *l);

GWENHYWFAR_API
void *GWEN_List1_GetLast(const GWEN_LIST1 *l);



GWENHYWFAR_API
GWEN_LIST1_ELEMENT *GWEN_List1Element_new(void *d);

GWENHYWFAR_API
void GWEN_List1Element_free(GWEN_LIST1_ELEMENT *el);

GWENHYWFAR_API
void *GWEN_List1Element_GetData(const GWEN_LIST1_ELEMENT *el);

GWENHYWFAR_API
void *GWEN_List1Element_GetPrevious(const GWEN_LIST1_ELEMENT *el);

GWENHYWFAR_API
void *GWEN_List1Element_GetNext(const GWEN_LIST1_ELEMENT *el);




/**
 * Use this inside the declaration of a struct for which you want to create
 * lists.
 */
#define GWEN_LIST_ELEMENT(t) \
GWEN_LIST1_ELEMENT *_list1_element;


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
 *
 * This macro should be used in applications, not in libraries. In
 * libraries please use the macro @ref GWEN_LIST_FUNCTION_LIB_DEFS.
 */
#define GWEN_LIST_FUNCTION_LIB_DEFS(t, pr, decl) \
  typedef GWEN_LIST1_ELEMENT t##_LIST_ELEMENT; \
  typedef GWEN_LIST1 t##_LIST; \
  \
  decl int pr##_List_AddList(t##_LIST *dst, t##_LIST *l); \
  decl int pr##_List_Add(t *element, t##_LIST *list); \
  decl int pr##_List_Insert(t *element, t##_LIST *list); \
  decl int pr##_List_Del(t *element); \
  decl t* pr##_List_First(const t##_LIST *l); \
  decl t* pr##_List_Last(const t##_LIST *l); \
  decl void pr##_List_Clear(t##_LIST *l); \
  decl t##_LIST* pr##_List_new(); \
  decl void pr##_List_free(t##_LIST *l); \
  decl t* pr##_List_Next(const t *element); \
  decl t* pr##_List_Previous(const t *element); \
  decl GWEN_TYPE_UINT32 pr##_List_GetCount(const t##_LIST *l);



#define GWEN_LIST_FUNCTION_DEFS(t, pr) \
  GWEN_LIST_FUNCTION_LIB_DEFS(t, pr, GWEN_DUMMY_EMPTY_ARG)


  /**
   * Use this inside your code files (*.c).
   * Actually implements the functions for which the prototypes have been
   * defined via @ref GWEN_LIST_FUNCTION_DEFS.
   */
#define GWEN_LIST_FUNCTIONS(t, pr) \
  \
  int pr##_List_Add(t *element, t##_LIST *l) { \
    assert(element); \
    assert(element->_list1_element);\
    return GWEN_List1_Add(l, element->_list1_element); \
  } \
  \
  int pr##_List_AddList(t##_LIST *dst, t##_LIST *l) { \
    return GWEN_List1_AddList(dst, l); \
  } \
  \
  int pr##_List_Insert(t *element, t##_LIST *l) { \
    assert(element); \
    assert(element->_list1_element);\
    return GWEN_List1_Insert(l, element->_list1_element); \
  } \
  \
  int pr##_List_Del(t *element){ \
    assert(element); \
    assert(element->_list1_element);\
    return GWEN_List1_Del(element->_list1_element); \
  }\
  \
  t* pr##_List_First(const t##_LIST *l) { \
    if (l) return (t*)GWEN_List1_GetFirst(l);\
    else return 0; \
  } \
  \
  t* pr##_List_Last(const t##_LIST *l) { \
    if (l) return (t*) GWEN_List1_GetLast(l);\
    else return 0; \
  } \
  \
  void pr##_List_Clear(t##_LIST *l) { \
    t* el; \
    while( (el=GWEN_List1_GetFirst(l)) ) {\
      pr##_List_Del(el);\
      pr##_free(el);\
    } /* while */ \
  } \
  \
  t##_LIST* pr##_List_new(){\
    return (t##_LIST*)GWEN_List1_new(); \
  }\
  \
  void pr##_List_free(t##_LIST *l) {\
    if (l) { \
      pr##_List_Clear(l);\
      GWEN_List1_free(l); \
    }\
  } \
  \
  t* pr##_List_Next(const t *element) { \
    assert(element); \
    assert(element->_list1_element);\
    return (t*)GWEN_List1Element_GetNext(element->_list1_element);\
  } \
  \
  t* pr##_List_Previous(const t *element) { \
    assert(element); \
    assert(element->_list1_element);\
    return (t*)GWEN_List1Element_GetPrevious(element->_list1_element);\
  } \
  \
  GWEN_TYPE_UINT32 pr##_List_GetCount(const t##_LIST *l){\
    return GWEN_List1_GetCount(l);\
  }



/**
 * Use this in your code file (*.c) inside the init code for the struct
 * you want to use in lists (in GWEN these are the functions which end with
 * "_new".
 */
#define GWEN_LIST_INIT(t, element) \
  element->_list1_element=GWEN_List1Element_new(element);


/**
 * Use this in your code file (*.c) inside the fini code for the struct
 * you want to use in lists (in GWEN these are the functions which end with
 * "_free".
 */
#define GWEN_LIST_FINI(t, element) \
  if (element && element->_list1_element) { \
    GWEN_List1Element_free(element->_list1_element); \
    element->_list1_element=0; \
  }

/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif


