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
#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"
#include "gwenhywfar/text.h"
#include <ctype.h>



void *GWEN_Path_Handle(const char *path,
                       void *data,
                       GWEN_TYPE_UINT32 flags,
                       GWEN_PATHHANDLERPTR elementFunction) {
  GWEN_BUFFER *buf1;
  int i;
  unsigned int origflags;
  int startAtRoot;

  origflags=flags;

  buf1=GWEN_Buffer_new(0, 128, 0, 1);

  /* skip leading blanks */
  while (*path && isspace((int)*path))
    path++;

  /* skip leading slashes */
  startAtRoot=0;
  while (*path && (*path=='/' || *path=='\\')) {
    if (origflags & GWEN_PATH_FLAGS_CHECKROOT)
      startAtRoot=1;
    path++;
  } /* while */

  while (*path) {
    GWEN_Buffer_Reset(buf1);

    flags=origflags &
      ~GWEN_PATH_FLAGS_INTERNAL &
      ~GWEN_PATH_FLAGS_VARIABLE;

    /* copy element into buffer */
    i=0;
    if (startAtRoot) {
      GWEN_Buffer_AppendByte(buf1, '/');
      flags|=GWEN_PATH_FLAGS_ROOT;
    }
    while (*path && !(*path=='/' || *path=='\\'))
      GWEN_Buffer_AppendByte(buf1, *(path++));

    /* check for group or entry */
    if (*path) {
      /* skip slashes */
      path++;
      while (*path && (*path=='/' || *path=='\\'))
        path++;

      /* check if delimiter is followed by #0 */
      if (!*path) {
        /* it is so do some more tests */
        if (origflags & GWEN_PATH_FLAGS_VARIABLE) {
          /* a trailing slash indicates that the current entry is
           * supposed to be a group. If the flags indicate that an entry
           * is to be found then this would be an error, because the path
           * ends in a group instead of an entry */
          DBG_DEBUG(GWEN_LOGDOMAIN, "Path ends with a group while an entry is wanted");
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

    /* escape or unescape if wanted */
    if (!(flags & GWEN_PATH_FLAGS_LAST) ||
        ((flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_CONVERT_LAST))) {
      if (flags & GWEN_PATH_FLAGS_ESCAPE) {
        GWEN_BUFFER *buf2;
        const char *p;
        int rv;

        buf2=GWEN_Buffer_new(0, 64, 0, 1);
        GWEN_Buffer_SetStep(buf2, 128);
        p=GWEN_Buffer_GetStart(buf1);
        if (startAtRoot) {
          p++;
          GWEN_Buffer_AppendByte(buf2, '/');
        }
        if (flags & GWEN_PATH_FLAGS_TOLERANT_ESCAPE)
          rv=GWEN_Text_EscapeToBufferTolerant(p, buf2);
        else
          rv=GWEN_Text_EscapeToBuffer(p, buf2);
        if (rv) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Could not escape path element");
          GWEN_Buffer_free(buf2);
          GWEN_Buffer_free(buf1);
          return 0;
        }
        GWEN_Buffer_free(buf1);
        buf1=buf2;
      }
      else if (flags & GWEN_PATH_FLAGS_UNESCAPE) {
        GWEN_BUFFER *buf2;
        const char *p;
        int rv;

        buf2=GWEN_Buffer_new(0, 64, 0, 1);
        GWEN_Buffer_SetStep(buf2, 128);
        p=GWEN_Buffer_GetStart(buf1);
        if (startAtRoot) {
          p++;
          GWEN_Buffer_AppendByte(buf2, '/');
        }
        if (flags & GWEN_PATH_FLAGS_TOLERANT_ESCAPE)
          rv=GWEN_Text_UnescapeToBufferTolerant(p, buf2);
        else
          rv=GWEN_Text_UnescapeToBuffer(p, buf2);
        if (rv) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Could not unescape path element");
          GWEN_Buffer_free(buf2);
          GWEN_Buffer_free(buf1);
          return 0;
        }
        GWEN_Buffer_free(buf1);
        buf1=buf2;
      }
    }

    /* call function */
    if (elementFunction) {
      data=(elementFunction)(GWEN_Buffer_GetStart(buf1), data, flags);
      if (!data) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Error on path element \"%s\"",
                  GWEN_Buffer_GetStart(buf1));
        GWEN_Buffer_free(buf1);
        return 0;
      }
    }
    DBG_DEBUG(GWEN_LOGDOMAIN, "Successfully handled element \"%s\"",
              GWEN_Buffer_GetStart(buf1));
    if (startAtRoot)
      startAtRoot=0;
  } /* while (*path) */

  GWEN_Buffer_free(buf1);
  return data;
}



void *GWEN_Path_AppendPathElement(const char *entry,
                                  void *data,
                                  unsigned int flags){
  GWEN_BUFFER *ebuf;

  ebuf=(GWEN_BUFFER*)data;

  GWEN_Buffer_AppendString(ebuf, entry);
  if (!(flags & GWEN_PATH_FLAGS_LAST) ||
      !(flags & GWEN_PATH_FLAGS_VARIABLE))
    GWEN_Buffer_AppendByte(ebuf, '/');
  GWEN_Buffer_AllocRoom(ebuf, 1);
  GWEN_Buffer_GetPosPointer(ebuf)[0]=0;
  return data;
}



int GWEN_Path_Convert(const char *path,
                      GWEN_BUFFER *buffer,
                      GWEN_TYPE_UINT32 flags) {
  void *p;

  p=GWEN_Path_Handle(path,
                     buffer,
                     flags,
                     GWEN_Path_AppendPathElement);
  if (!p) {
    return -1;
  }
  return 0;
}







