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


/** @file db.h
 *
 * @brief This file contains the definition of a GWEN_DB database.
 *
 * A GWEN_DB database consists of a tree of @ref GWEN_DB_NODE
 * entries.
 * Such a @ref GWEN_DB_NODE node can either be a group of nodes, or a
 * variable, or a variable's value. Usually an application programmer
 * will only get in touch with group nodes. Nevertheless first we
 * explain the difference of the three cases. Depending on either of
 * these cases, you can
 *
 * <ol>
 * <li> Iterate through groups or get a variable with
 *  e.g. GWEN_DB_GetNextGroup(), GWEN_DB_GetNextVar() 
 * <li> Get the type of a variable with e.g. GWEN_DB_GetVariableType() 
 *  -- the value of a variable is retrieved by the shortcut functions 
 *  explained below.
 * <li> Get the type of a value with GWEN_DB_GetValueType(). Again the
 * value itself is retrieved with the shortcut functions below.
 * </ol>
 * 
 * To retrieve or set the value of such a variable, the following
 * "shortcut" functions for all three supported typed exist:
 * GWEN_DB_GetIntValue(), GWEN_DB_GetCharValue(),
 * GWEN_DB_GetBinValue(). These functions only accept a group  and a "path"
 * to the desired variable.
 */

#ifndef GWENHYFWAR_DB_H
#define GWENHYFWAR_DB_H

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/path.h>
#include <gwenhyfwar/bufferedio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** maximum size of a text line when reading/writing from/to streams */
#define GWEN_DB_LINE_MAXSIZE  1024

/** @name DB Flags
 *
 * Please note that the setter functions also take the flags from
 * the module GWEN_PATH into account. So you most likely need to specify
 * them, too.
 */
/*@{*/
/** Overwrite existing values when assigning a new value to a variable */
#define GWEN_DB_FLAGS_OVERWRITE_VARS         0x00010000
/** Overwrite existing groups when calling @ref GWEN_DB_GetGroup() */
#define GWEN_DB_FLAGS_OVERWRITE_GROUPS       0x00020000
/** quote variable names when writing them to a stream */
#define GWEN_DB_FLAGS_QUOTE_VARNAMES         0x00040000
/** quote values when writing them to a stream */
#define GWEN_DB_FLAGS_QUOTE_VALUES           0x00080000
/** allows writing of subgroups when writing to a stream */
#define GWEN_DB_FLAGS_WRITE_SUBGROUPS        0x00100000
/** adds some comments when writing to a stream */
#define GWEN_DB_FLAGS_DETAILED_GROUPS        0x00200000
/** indents text according to the current path depth when writing to a
 * stream to improve the readability of the created file */
#define GWEN_DB_FLAGS_INDEND                 0x00400000
/** writes a newline to the stream after writing a group to improve
 * the readability of the created file */
#define GWEN_DB_FLAGS_ADD_GROUP_NEWLINES     0x00800000
/** uses a colon (":") instead of an equation mark when reading/writing
 * variable definitions */
#define GWEN_DB_FLAGS_USE_COLON              0x01000000
/** stops reading from a stream at empty lines */
#define GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE     0x02000000
/** normally the type of a variable is written to the stream, too.
 * This flag changes this behaviour */
#define GWEN_DB_FLAGS_OMIT_TYPES             0x04000000
/** appends data to an existing file instead of overwriting it */
#define GWEN_DB_FLAGS_APPEND_FILE            0x08000000
/*@}*/


/** These are the default flags which you use in most cases */
#define GWEN_DB_FLAGS_DEFAULT \
  GWEN_DB_FLAGS_QUOTE_VALUES | \
  GWEN_DB_FLAGS_WRITE_SUBGROUPS | \
  GWEN_DB_FLAGS_DETAILED_GROUPS | \
  GWEN_DB_FLAGS_INDEND | \
  GWEN_DB_FLAGS_ADD_GROUP_NEWLINES


/** same like @ref GWEN_DB_FLAGS_DEFAULT except that the produced file
 * (when writing to a stream) is more compact (but less readable)*/
#define GWEN_DB_FLAGS_COMPACT \
  GWEN_DB_FLAGS_QUOTE_VALUES | \
  GWEN_DB_FLAGS_WRITE_SUBGROUPS

/** These flags can be used to read a DB from a HTTP header. It uses a
 * colon instead of the equation mark with variable definitions and stops
 * when encountering an empty line.*/
#define GWEN_DB_FLAGS_HTTP \
  GWEN_DB_FLAGS_USE_COLON |\
  GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE


/**
 * This is the type used to store a DB. Its contents are explicitly NOT
 * part of the API. 
 *
 * A description of what can be done with this type can be found in
 * @ref db.h
 */
typedef union GWEN_DB_NODE GWEN_DB_NODE;

/**
 * This specifies the type of a value stored in the DB.
 */
typedef enum {
  /** type unknown */
  GWEN_DB_VALUETYPE_UNKNOWN=0,
  /** simple, null terminated C-string */
  GWEN_DB_VALUETYPE_CHAR,
  /** integer */
  GWEN_DB_VALUETYPE_INT,
  /** binary, user defined data */
  GWEN_DB_VALUETYPE_BIN
} GWEN_DB_VALUETYPE;



/** @name Constructing, Destructing, Copying
 *
 */
/*@{*/

/**
 * Creates a new (empty) group with the given name. I.e. this is the
 * constructor.  When finished using this group, you should free it
 * using @ref GWEN_DB_Group_free() in order to avoid memory leaks.
 */
GWEN_DB_NODE *GWEN_DB_Group_new(const char *name);

/**
 * Frees a DB group. I.e. this is the destructor. This is needed to
 * avoid memory leaks.
 */
void GWEN_DB_Group_free(GWEN_DB_NODE *n);

/**
 * Creates a deep copy of the given node. This copy will then be owned
 * by the caller and MUST be freed after using it by calling @ref
 * GWEN_DB_Group_free().
 */
GWEN_DB_NODE *GWEN_DB_Group_dup(const GWEN_DB_NODE *n);
/*@}*/



/** @name Iterating Through Groups and Variables
 *
 */
/*@{*/
/**
 * Returns the first group below the given one.
 * If there is no group then NULL is returned.
 */
GWEN_DB_NODE *GWEN_DB_GetFirstGroup(GWEN_DB_NODE *n);

/**
 * Returns the first group following the given one.
 * If there is no group then NULL is returned.
 */
GWEN_DB_NODE *GWEN_DB_GetNextGroup(GWEN_DB_NODE *n);

/**
 * Returns the first variable below the given group.
 * If there is no variable then NULL is returned.
 */
GWEN_DB_NODE *GWEN_DB_GetFirstVar(GWEN_DB_NODE *n);


/**
 * Returns the next variable following the given one.
 * If there is no variable then NULL is returned.
 */
GWEN_DB_NODE *GWEN_DB_GetNextVar(GWEN_DB_NODE *n);

/**
 * Returns the type of the first value of the given variable
 * @param n root node of the DB
 * @param p path of the variable to inspect
 */
GWEN_DB_VALUETYPE GWEN_DB_GetVariableType(GWEN_DB_NODE *n,
                                          const char *p);

/**
 * Returns the first value below the given variable.
 * If there is no value then NULL is returned.
 */
GWEN_DB_NODE *GWEN_DB_GetFirstValue(GWEN_DB_NODE *n);

/**
 * Returns the next value following the given one.
 * If there is no value then NULL is returned.
 */
GWEN_DB_NODE *GWEN_DB_GetNextValue(GWEN_DB_NODE *n);

/**
 * Returns the type of the given value.
 */
GWEN_DB_VALUETYPE GWEN_DB_GetValueType(GWEN_DB_NODE *n);

/*@}*/




/** @name Special Variable Treatment
 *
 */
/*@{*/
/**
 * Deletes the given variable by removing it and its values from the DB.
 * @param n root of the DB
 * @param path path to the variable to remove
 * @return Zero on success, nonzero on error
 */
int GWEN_DB_DeleteVar(GWEN_DB_NODE *n,
                      const char *path);

/**
 * Checks whether the given variable exists.
 * @return Zero if variable not found, nonzero otherwise
 * @param n root of the DB
 * @param path path to the variable to check for
 */
int GWEN_DB_VariableExists(GWEN_DB_NODE *n,
                           const char *path);
/*@}*/





/** @name Variable Getter and Setter
 *
 * These getter functions check for the existence of the given variable and
 * return the value specified by an index.
 * Under the following circumstances the also given default value will be
 * returned:
 * <ul>
 *  <li>the variable does not exist</li>
 *  <li>the variable exists but has no values (should not occur)</li>
 *  <li>the variable exists but the given value index is out of range (e.g.
 *  specifying index 1 with a variable that has only one value)</li>
 *  <li>a string value is expected but the variable is not of that type.
 *  However, if you want an integer value but the variable only has a char
 *  value then the getter functions try to convert the char to an integer.
 *  Other conversions do not take place.</li>
 * </ul>
 *
 * The setter functions either replace an existing variable, create a missing
 * variable, or return an error if the variable does not exist (see description
 * of the flags).
 * All setter functions make deep copies of the given values, so you may
 * free the params after calling the setter function.
 *
 * All getter functions return the variable's retrieved value. All
 * setter functions return Zero if ok and Nonzero on error.
 *
 * This module knows about the following types (see @ref GWEN_DB_VALUETYPE):
 * <ul>
 *  <li>char (simple null terminated C strings)</li>
 *  <li>int (integer values)</li>
 *  <li>bin (binary, user specified data)</li>
 * </ul>
 */
/*@{*/
/** Returns the variable's retrieved value.
 */
const char *GWEN_DB_GetCharValue(GWEN_DB_NODE *n,
                                 const char *path,
                                 int idx,
                                 const char *defVal);
/**
 * @param val The string value that is copied into the DB
 * @return 0 on success, nonzero on error
 */
int GWEN_DB_SetCharValue(GWEN_DB_NODE *n,
                         unsigned int flags,
                         const char *path,
                         const char *val);


/** Returns the variable's retrieved value.
 */
int GWEN_DB_GetIntValue(GWEN_DB_NODE *n,
                        const char *path,
                        int idx,
                        int defVal);

/**
 * @return 0 on success, nonzero on error
 */
int GWEN_DB_SetIntValue(GWEN_DB_NODE *n,
                        unsigned int flags,
                        const char *path,
                        int val);


/** Returns the variable's retrieved value. The size of the binary
 * data is written into the int pointer argument returnValueSize.
 */
const void *GWEN_DB_GetBinValue(GWEN_DB_NODE *n,
                                const char *path,
                                int idx,
                                const void *defVal,
                                unsigned int defValSize,
                                unsigned int *returnValueSize);

/**
 * @param val The binary data that is copied into the DB.
 * @param valSize The number of bytes in the binary data value.
 *
 * @return 0 on success, nonzero on error
 */
int GWEN_DB_SetBinValue(GWEN_DB_NODE *n,
			unsigned int flags,
			const char *path,
                        const void *val,
			unsigned int valSize);
/*@}*/



/** @name Group Handling
 *
 */
/*@{*/

/**
 * This function either creates a new group, returns an existing one or
 * returns an error if there is no group but the caller wanted one (depending
 * on the flags given).
 */
GWEN_DB_NODE *GWEN_DB_GetGroup(GWEN_DB_NODE *n,
                               unsigned int flags,
                               const char *path);

/**
 * Returns the name of the given group.
 */
const char *GWEN_DB_GroupName(GWEN_DB_NODE *n);

/**
 * Renames the given group.
 */
void GWEN_DB_GroupRename(GWEN_DB_NODE *n, const char *newname);

/**
 * Adds the given group as a new child of the first given one.
 *
 * @note This function takes over the ownership of the new group, so
 * you MUST NOT free it afterwards.
 */
int GWEN_DB_AddGroup(GWEN_DB_NODE *n, GWEN_DB_NODE *nn);

/**
 * This function adds all children of the second node as new children to
 * the first given one.
 *
 * @note This function does NOT take over ownership of the new
 * group. The caller is still the owner of the given group.
 */
int GWEN_DB_AddGroupChildren(GWEN_DB_NODE *n, GWEN_DB_NODE *nn);

/**
 * Unlinks a group (and thereby all its children) from its parent and
 * brothers.
 *
 * This function DOES NOT free the group, it just unlinks it. You can then use
 * it with e.g. @ref GWEN_DB_AddGroup or other functions to relink it at any
 * other position of even in other DBs.
 */
void GWEN_DB_UnlinkGroup(GWEN_DB_NODE *n);

/**
 * Locates and removes the group of the given name.
 * @return 0 on success, nonzero on error
 */
int GWEN_DB_DeleteGroup(GWEN_DB_NODE *n,
                        const char *path);
/**
 * Frees all children of the given node thereby clearing it.
 * @return 0 on success, nonzero on error
 * @param path path to the group under the given node to clear
 * (if 0 then clear the given node)
 */
int GWEN_DB_ClearGroup(GWEN_DB_NODE *n,
                       const char *path);
/*@}*/



/** @name Reading and Writing From/To Streams
 *
 * These functions read or write a DB from/to GWEN_BUFFEREDIO.
 * This allows to use any source or target supported by GWEN_BUFFEREDIO
 * for data storage (these are currently sockets, files and memory buffers).
 * The flags determine how to read/write the data (e.g. if sub-groups are
 * to be written etc).
 */
/*@{*/

/** Read a DB from GWEN_BUFFEREDIO.
 */
int GWEN_DB_ReadFromStream(GWEN_DB_NODE *n,
                           GWEN_BUFFEREDIO *bio,
                           unsigned int dbflags);

/** Read a DB from a file.
 */
int GWEN_DB_ReadFile(GWEN_DB_NODE *n,
                     const char *fname,
                     unsigned int dbflags);

/** Write a DB to a GWEN_BUFFEREDIO.
 */
int GWEN_DB_WriteToStream(GWEN_DB_NODE *node,
                          GWEN_BUFFEREDIO *bio,
                          unsigned int dbflags);

/** Write a DB to a file.
 */
int GWEN_DB_WriteFile(GWEN_DB_NODE *n,
                      const char *fname,
                      unsigned int dbflags);
/*@}*/


/** @name Debugging
 *
 * These functions are for debugging purposes only. You should NOT consider
 * them part of the API.
 */
/*@{*/

/**
 * Dumps the content of the given DB to the given file (e.g. stderr).
 * @param n node to dump
 * @param f destination file
 * @param insert number of blanks to insert at every line
 */
void GWEN_DB_Dump(GWEN_DB_NODE *n, FILE *f, int insert);
/*@}*/


#ifdef __cplusplus
}
#endif


#endif



