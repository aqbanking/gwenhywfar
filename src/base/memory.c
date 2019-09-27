/***************************************************************************
 begin       : Sat Jun 28 2003
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/memory.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>




void *GWEN_Memory_malloc(size_t wsize)
{
  void *p;

  if (GWEN_UNLIKELY(wsize==0)) {
    fprintf(stderr, "GWEN error: allocating 0 bytes, maybe a program error\n");
    abort();
  }

  p=malloc(wsize);
  if (p==NULL) {
    fprintf(stderr, "GWEN error: Not allocated %lu bytes (memory full?)\n",
            (unsigned long int) wsize);
  }
  assert(p);
  return p;
}



void *GWEN_Memory_realloc(void *oldp, size_t nsize)
{
  assert(oldp);
  assert(nsize);

  return realloc(oldp, nsize);
}



void GWEN_Memory_dealloc(void *p)
{
  free(p);
}



char *GWEN_Memory_strdup(const char *s)
{
  assert(s);
  return strdup(s);
}



