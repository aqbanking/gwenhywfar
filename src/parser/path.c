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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG

#include "path.h"
#include "gwenhyfwar/debug.h"
#include "gwenhyfwar/misc.h"
#include <ctype.h>



void *GWEN_Path_Handle(const char *path,
                       void *data,
                       unsigned int flags,
                       GWEN_PATHHANDLERPTR elementFunction) {
  char buffer[256];
  int i;
  unsigned int origflags;

  origflags=flags;

  /* skip leading blanks */
  while (*path && isspace(*path))
    path++;

  /* skip leading slashes */
  while (*path && (*path=='/' || *path=='\\'))
    path++;

  while (*path) {
    /* copy element into buffer */
    i=0;
    while (*path && !(*path=='/' || *path=='\\')) {
      if (i>(sizeof(buffer)-1)) {
        DBG_ERROR(0, "Path element too long");
        return 0;
      }
      buffer[i++]=*(path++);
    }
    /* end element buffer */
    buffer[i]=0;

    /* check for the element type */
    flags=origflags &
      ~GWEN_PATH_FLAGS_INTERNAL &
      ~GWEN_PATH_FLAGS_VARIABLE;

    /* check for group or entry */
    if (*path) {
      /* skip slashes */
      path++;
      while (*path && *path=='/')
        path++;

      /* check if delimiter is followed by #0 */
      if (!*path) {
        /* it is so do some more tests */
        if (origflags & GWEN_PATH_FLAGS_VARIABLE) {
          /* a trailing slash indicates that the current entry is
           * supposed to be a group. If the flags indicate that an entry
           * is to be found then this would be an error, because the path
           * ends in a group instead of an entry */
          DBG_DEBUG(0, "Path ends with a group while an entry is wanted");
          return 0;
        }
        /* other wise simply mark this element as the last one */
        flags|=GWEN_PATH_FLAGS_LAST;
      }
    } /* if *path */
    else {
      /* path ends here with #0 */
      flags|=GWEN_PATH_FLAGS_LAST;
      if (origflags & GWEN_PATH_FLAGS_VARIABLE) {
        /* path ends with #0, caller wants a variable so this
         * last element is one */
        flags|=GWEN_PATH_FLAGS_VARIABLE;
      }
    }

    /* call function */
    if (elementFunction) {
      data=(elementFunction)(buffer, data, flags);
      if (!data) {
        DBG_DEBUG(0, "Error on path element \"%s\"",
                  buffer);
        return 0;
      }
    }
    DBG_DEBUG(0, "Successfully handled element \"%s\"",
              buffer);
  } /* while (*path) */

  return data;
}





