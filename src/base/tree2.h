/***************************************************************************
 begin       : Thu Jul 04 2019
 copyright   : (C) 2019 by Martin Preuss
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


#ifndef GWEN_TREE2_H
#define GWEN_TREE2_H


#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup GWEN_MACRO_TREE2 Macros For Typesafe Tree Handling
 *
 * The macros of this group facilitates typesafe use of trees.
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
 * GWEN_TREE2_FUNCTION_DEFS(MYSTRUCT, MyStruct);
 *
 * struct MYSTRUCT {
 *   GWEN_TREE2_ELEMENT(MYSTRUCT);
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
 * GWEN_TREE2_FUNCTIONS(MYSTRUCT, MyStruct)
 *
 * MYSTRUCT *MyStruct_new(int myData) {
 *   MYSTRUCT *pMyStruct;
 *
 *   pMyStruct=(MYSTRUCT*)malloc(sizeof(MYSTRUCT));
 *   memset(pMyStruct, 0, sizeof(MYSTRUCT));
 *
 *   GWEN_TREE2_INIT(MYSTRUCT, pMyStruct)
 *
 *   pMyStruct->myData=myData;
 *   return pMyStruct;
 * }
 *
 * void MyStruct_free(MYSTRUCT *pMyStruct) {
 *   if (pMyStruct) {
 *     pMyStruct->myData=0;
 *
 *     GWEN_TREE2_FINI(MYSTRUCT, pMyStruct)
 *
 *     free(pMyStruct);
 *   }
 * }
 *
 * @endcode
 * Please note the three macros used in the code file:
 * <ul>
 *   <li>@ref GWEN_TREE2_FUNCTIONS creates the functions for the list
 *       management</li>
 *   <li>@ref GWEN_TREE2_INIT initializes the list data inside your
 *       struct to defined values </li>
 *   <li>@ref GWEN_TREE2_FINI frees all ressources occupied by the list
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
 * The tree management code assumes that there is a function called
 * (in this example) @b MyStruct_free() (or generally: TYPEPREFIX_free).
 * This is used when destroying a list of MYSTRUCT elements. In this case
 * all elements still enlisted are destroyed upon destruction of the list.
 * </p>
 */
/*@{*/


/** @name Internal Functions
 *
 * All functions and structs within this group should be considered
 * internal. They just implement the functionality behind the typesafe list
 * macros (see @ref GWEN_TREE2_FUNCTION_LIB_DEFS and following).
 */
/*@{*/
typedef struct GWEN_TREE2_ELEMENT GWEN_TREE2_ELEMENT;


/** Unlinks (removes) a tree element from the tree it used to
 * belong to. The tree element is not free'd or anything, it is
 * only removed from the tree it used to belong to. (This
 * operation is also called "remove" elsewhere.) */
GWENHYWFAR_API
void GWEN_Tree2_Unlink(GWEN_TREE2_ELEMENT *el);


/** Replaces a tree element with another one, so the replacement takes the place of the given element.
 * The given element to replace is unlinked in the process and can be free'd.
 * The replacement MUST NOT be part of any tree.
 */
GWENHYWFAR_API
void GWEN_Tree2_Replace(GWEN_TREE2_ELEMENT *elToReplace, GWEN_TREE2_ELEMENT *elReplacement);

/** Add a child below the given element. */
GWENHYWFAR_API
void GWEN_Tree2_AddChild(GWEN_TREE2_ELEMENT *where, GWEN_TREE2_ELEMENT *el);

/** Insert a child below the given element. */
GWENHYWFAR_API
void GWEN_Tree2_InsertChild(GWEN_TREE2_ELEMENT *where, GWEN_TREE2_ELEMENT *el);


/** Returns the data pointer of the first list element. */
GWENHYWFAR_API
void *GWEN_Tree2_GetFirstChild(const GWEN_TREE2_ELEMENT *el);

/** Returns the data pointer of the last list element. */
GWENHYWFAR_API
void *GWEN_Tree2_GetLastChild(const GWEN_TREE2_ELEMENT *el);



/** Allocate (create) a new list element structure. */
GWENHYWFAR_API
GWEN_TREE2_ELEMENT *GWEN_Tree2Element_new(void *d);

/** Free (delete) a list element structure. */
GWENHYWFAR_API
void GWEN_Tree2Element_free(GWEN_TREE2_ELEMENT *el);

/** Returns the data pointer of the list element that is the
 * previous (predecessor) to the given one in its list. If there
 * is no such prepending list element, returns NULL. */
GWENHYWFAR_API
void *GWEN_Tree2Element_GetPrevious(const GWEN_TREE2_ELEMENT *el);

/** Returns the data pointer of the list element that is the next
 * one (successor) to the given one in its list. If there is no
 * such prepending list element, returns NULL. */
GWENHYWFAR_API
void *GWEN_Tree2Element_GetNext(const GWEN_TREE2_ELEMENT *el);

/** Returns the element which is logically below the given one.
 * The order of search is this:
 * <ul>
 *  <li>first child of the given element </li>
 *  <li>next neighbour of the given element </li>
 *  <li>loop for every parent: check next neighbour of the given element's parent (if any) </li>
 * </ul>
 */
GWENHYWFAR_API
void *GWEN_Tree2Element_GetBelow(const GWEN_TREE2_ELEMENT *el);

/** Returns the first child of the given element. */
GWENHYWFAR_API
void *GWEN_Tree2Element_GetFirstChild(const GWEN_TREE2_ELEMENT *el);

/** Returns the last child of the given element. */
GWENHYWFAR_API
void *GWEN_Tree2Element_GetLastChild(const GWEN_TREE2_ELEMENT *el);

GWENHYWFAR_API
void *GWEN_Tree2Element_GetParent(const GWEN_TREE2_ELEMENT *el);

/** Returns the number of children of the given element */
GWENHYWFAR_API
uint32_t GWEN_Tree2Element_GetChildrenCount(const GWEN_TREE2_ELEMENT *el);

/*@}*/



/** @name Typesafe Macros
 *
 */
/*@{*/

/**
 * Use this inside the declaration of a struct for which you want to create
 * lists.
 */
#define GWEN_TREE2_ELEMENT(t) \
GWEN_TREE2_ELEMENT *_tree2_element;

/**
 * Use this macro in your public header files to export only list functions
 * which do not modify a list. This allows your code to return lists which can
 * not be modified by callers. It also prevents callers from creating their
 * own lists (this is sometimes needed).
 */
#define GWEN_TREE2_FUNCTION_LIB_DEFS_CONST(t, pr, decl) \
  decl t* pr##_Tree2_GetNext(const t *element); \
  decl t* pr##_Tree2_GetPrevious(const t *element); \
  decl t* pr##_Tree2_GetBelow(const t *element); \
  decl t* pr##_Tree2_GetFirstChild(const t *element); \
  decl t* pr##_Tree2_GetLastChild(const t *element); \
  decl t* pr##_Tree2_GetParent(const t *element);


#define GWEN_TREE2_FUNCTION_LIB_DEFS_NOCONST(t, pr, decl) \
  typedef GWEN_TREE2_ELEMENT t##_TREE2_ELEMENT; \
  \
  decl void pr##_Tree2_Unlink(t *element); \
  decl void pr##_Tree2_Replace(t *elToReplace, t *elReplacement); \
  \
  decl void pr##_Tree2_AddChild(t *where, t *element); \
  decl void pr##_Tree2_InsertChild(t *where, t *element); \
  \
  decl void pr##_Tree2_ClearChildren(t *element); \
  decl void pr##_Tree2_free(t *element);


#define GWEN_TREE2_FUNCTION_DEFS_CONST(t, pr) \
  GWEN_TREE2_FUNCTION_LIB_DEFS_CONST(t, pr, GWEN_DUMMY_EMPTY_ARG)

#define GWEN_TREE2_FUNCTION_DEFS_NOCONST(t, pr) \
  GWEN_TREE2_FUNCTION_LIB_DEFS_NOCONST(t, pr, GWEN_DUMMY_EMPTY_ARG)


/**
 * Use this in public header files to define some prototypes for list
 * functions.
 * Let's assume the type of your list elements is "MYTYPE" and you want to
 * use the prefix "MyType_" for the list functions.
 * The following function prototypes will then be created:
 * <ul>
 *  <li>
 *    void MyType_Tree2_Add(MYTYPE *element, MYTYPE_TREE *list);<br>
 *    Adds (appends) a MYTYPE struct at the end of the given
 *    list. (We apologize for the unusual argument order here.)
 *  </li>
 *  <li>
 *    void MyType_Tree2_Unlink(MYTYPE *element);<br>
 *    Removes a MyType struct from the list it is enlisted to.
 *  </li>
 *  <li>
 *    MYTYPE *MyType_Tree2_FirstChild(MYTYPE *element); <br>
 *    Returns the first element of the given list.
 *  </li>
 *  <li>
 *    MYTYPE* MyType_Tree2_Next(const MYTYPE *element);<br>
 *    Returns the next list element to the given one (the
 *    successor) in its list.
 *  </li>
 *  <li>
 *    MYTYPE* MyType_Tree2_Previous(const MYTYPE *element);<br>
 *    Returns the previous list element to the given one (the
 *    predecessor) in its list.
 *  </li>
 *  <li>
 *    void MyType_Tree2_Clear(MYTYPE *element); <br>
 *    Frees all entries of the given list.
 *    This function assumes that there is a function Mytype_free().
 *  </li>
 *  <li>
 *    MYTYPE_TREE *MyType_Tree2_new(); <br>
 *    Creates a new list of elements of MYTYPE type.
 *  </li>
 *  <li>
 *    void MyType_Tree2_free(MYTYPE_TREE *l); <br>
 *    Clears and frees a list of elements of MYTYPE type.
 *    All objects inside the list are freed.
 *  </li>
 * </ul>
 *
 */
#define GWEN_TREE2_FUNCTION_LIB_DEFS(t, pr, decl) \
  GWEN_TREE2_FUNCTION_LIB_DEFS_CONST(t, pr, decl) \
  GWEN_TREE2_FUNCTION_LIB_DEFS_NOCONST(t, pr, decl)


/**
 * This macro should be used in applications, not in libraries. In
 * libraries please use the macro @ref GWEN_TREE2_FUNCTION_LIB_DEFS.
 */
#define GWEN_TREE2_FUNCTION_DEFS(t, pr) \
  GWEN_TREE2_FUNCTION_LIB_DEFS(t, pr, GWEN_DUMMY_EMPTY_ARG)


/**
 * Use this inside your code files (*.c).
 * Actually implements the functions for which the prototypes have been
 * defined via @ref GWEN_TREE2_FUNCTION_DEFS.
 */
#define GWEN_TREE2_FUNCTIONS(t, pr) \
  \
  void pr##_Tree2_Unlink(t *element){ \
    assert(element); \
    assert(element->_tree2_element);\
    GWEN_Tree2_Unlink(element->_tree2_element); \
  }\
  \
  void pr##_Tree2_Replace(t *elToReplace, t *elReplacement) { \
    assert(elToReplace); \
    assert(elToReplace->_tree2_element);\
    assert(elReplacement); \
    assert(elReplacement->_tree2_element);\
    GWEN_Tree2_Replace(elToReplace->_tree2_element, elReplacement->_tree2_element); \
  } \
  \
  \
  t* pr##_Tree2_GetNext(const t *element) { \
    assert(element); \
    assert(element->_tree2_element);\
    return (t*)GWEN_Tree2Element_GetNext(element->_tree2_element);\
  } \
  \
  t* pr##_Tree2_GetPrevious(const t *element) { \
    assert(element); \
    assert(element->_tree2_element);\
    return (t*)GWEN_Tree2Element_GetPrevious(element->_tree2_element);\
  } \
  \
  t* pr##_Tree2_GetBelow(const t *element) { \
    assert(element); \
    assert(element->_tree2_element);\
    return (t*)GWEN_Tree2Element_GetBelow(element->_tree2_element);\
  } \
  \
  void pr##_Tree2_AddChild(t *where, t *element) { \
    assert(where); \
    assert(where->_tree2_element);\
    assert(element); \
    assert(element->_tree2_element);\
    GWEN_Tree2_AddChild(where->_tree2_element, element->_tree2_element); \
  } \
  \
  void pr##_Tree2_InsertChild(t *where, t *element) { \
    assert(where); \
    assert(where->_tree2_element);\
    assert(element); \
    assert(element->_tree2_element);\
    GWEN_Tree2_InsertChild(where->_tree2_element, element->_tree2_element); \
  } \
  \
  void pr##_Tree2_ClearChildren(t *element) { \
    t* c; \
    while( (c=GWEN_Tree2Element_GetFirstChild(element->_tree2_element)) ) {\
      pr##_Tree2_ClearChildren(c);\
      pr##_Tree2_Unlink(c);\
      pr##_free(c);\
    } /* while */ \
  } \
  \
  void pr##_Tree2_free(t *element) { \
    pr##_Tree2_ClearChildren(element);\
    pr##_Tree2_Unlink(element);\
    pr##_free(element);\
  } \
  \
  t* pr##_Tree2_GetFirstChild(const t *element) { \
    assert(element); \
    assert(element->_tree2_element);\
    return (t*)GWEN_Tree2Element_GetFirstChild(element->_tree2_element);\
  } \
  \
  t* pr##_Tree2_GetLastChild(const t *element) { \
    assert(element); \
    assert(element->_tree2_element);\
    return (t*)GWEN_Tree2Element_GetLastChild(element->_tree2_element);\
  } \
  \
  t* pr##_Tree2_GetParent(const t *element) { \
    assert(element); \
    assert(element->_tree2_element);\
    return (t*)GWEN_Tree2Element_GetParent(element->_tree2_element);\
  } \
  \


/**
 * Use this in your code file (*.c) inside the init code for the struct
 * you want to use in lists (in GWEN these are the functions which end with
 * "_new".
 */
#define GWEN_TREE2_INIT(t, element, pr) \
  element->_tree2_element=GWEN_Tree2Element_new(element);


/**
 * Use this in your code file (*.c) inside the fini code for the struct
 * you want to use in lists (in GWEN these are the functions which end with
 * "_free".
 * t is the base type id (e.g. "AF_ACCOUNT")
 * element is the pointer to the element to fini
 * pr ist the prefix of functions for this type (e.g. "AF_Account")
 */
#define GWEN_TREE2_FINI(t, element, pr) \
  if (element && element->_tree2_element) { \
    pr##_Tree2_ClearChildren(element);\
    pr##_Tree2_Unlink(element);\
    GWEN_Tree2Element_free(element->_tree2_element); \
    element->_tree2_element=0; \
  }

/*@}*/


/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif


