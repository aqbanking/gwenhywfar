/***************************************************************************
    begin       : Wed Feb 15 2022
    copyright   : (C) 2022 by Ralf Habacker
    email       : ralf.habacker@freenet.de

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

#ifndef GWENHYWFAR_Funcs_H
#define GWENHYWFAR_Funcs_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This is one of the very few structs inside Gwenhywfar whose
 * contents are available for direct access to the the program.
 * Developer's note: Please note that any change within this struct will
 * make it necessary to increment the SO_VERSION of the library !
 */
typedef struct {
  const char *name;
  int (*func1)(void);
  int (*func2)(int, char**);
  int (*func3)(GWEN_DB_NODE*, int, char**);
  const char *description;
} GWEN_FUNCS;

/* Defines a command without parameters but with description  */
#define GWEN_Funcs_Entry_Help(a,b,c) { a, b, NULL, NULL, c }

/* Defines a command with argc/argv parameter and description */
#define GWEN_Funcs_Entry_Args_Help(a,b,c) { a, 0, b, NULL, c }

/* Defines a command with DB_NODE type and argc/argv parameter and description */
#define GWEN_Funcs_Entry_DB_NODE_Args_Help(a,b,c) { a, NULL, NULL, b, c }

/* Defines the end of the command table */
#define GWEN_Funcs_Entry_End() { NULL, NULL, NULL, NULL, NULL }

/* Defines one of the above commands without description */
#define GWEN_Funcs_Entry(a,b) { a, b, NULL, NULL, NULL }
#define GWEN_Funcs_Entry_Args(a,b) { a, 0, b, NULL, NULL }
#define GWEN_Funcs_Entry_DB_NODE_Args(a,b) { a, NULL, NULL, b, NULL }

/* Shortcuts */
#define GWEN_FE_DAH GWEN_Funcs_Entry_DB_NODE_Args_Help
#define GWEN_FE_DA GWEN_Funcs_Entry_DB_NODE_Args
#define GWEN_FE_D GWEN_Funcs_Entry_DB_NODE
#define GWEN_FE_END GWEN_Funcs_Entry_End

/* Checks if a command variant exists */
GWENHYWFAR_API
int GWEN_Funcs_Has_Call(const GWEN_FUNCS *func);
GWENHYWFAR_API
int GWEN_Funcs_Has_Call_Args(const GWEN_FUNCS *func);
GWENHYWFAR_API
int GWEN_Funcs_Has_Call_DB_NODE_Args(const GWEN_FUNCS *func);

/* Functions to call a specified command */
GWENHYWFAR_API
int GWEN_Funcs_Call(const GWEN_FUNCS *func);
GWENHYWFAR_API
int GWEN_Funcs_Call_Args(const GWEN_FUNCS *func, int argc, char **argv);
GWENHYWFAR_API
int GWEN_Funcs_Call_DB_NODE_Args(const GWEN_FUNCS *func, GWEN_DB_NODE *node, int argc, char **argv);

/**
 * This function prints out a space separated list of all defined functions
 * without description.
 */
GWENHYWFAR_API
void GWEN_Funcs_Usage(const GWEN_FUNCS* funcs);

/**
 * This function outputs a list of all defined functions including the description
 */
GWENHYWFAR_API
void GWEN_Funcs_Usage_With_Help(const GWEN_FUNCS* funcs);

/**
 * This function returns a pointer to the function described by \p name
 * or NULL if the function was not found.
 */
GWENHYWFAR_API
const GWEN_FUNCS* GWEN_Funcs_Find(const GWEN_FUNCS* funcs, const char *name);

#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_Funcs_H */



