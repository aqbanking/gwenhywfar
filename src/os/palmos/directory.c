/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Dec 03 2002
    copyright   : (C) 2002 by Martin Preuss
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

#include "directory_p.h"

#include <stdlib.h>
#include <assert.h>
#include <gwenhywfar/debug.h>



GWEN_DIRECTORYDATA *GWEN_Directory_new(){
  GWEN_DIRECTORYDATA *d;

  d=(GWEN_DIRECTORYDATA *)malloc(sizeof(GWEN_DIRECTORYDATA));
  assert(d);
  memset(d,0,sizeof(GWEN_DIRECTORYDATA));
  return d;
}


void GWEN_Directory_free(GWEN_DIRECTORYDATA *d){
  if (d) {

    free(d);
  }
}


int GWEN_Directory_Open(GWEN_DIRECTORYDATA *d, const char *n){
  return -1;
}


int GWEN_Directory_Close(GWEN_DIRECTORYDATA *d){
  return -1;
}


int GWEN_Directory_Read(GWEN_DIRECTORYDATA *d,
                        char *buffer,
                        unsigned int len){
  return -1;
}


int GWEN_Directory_Rewind(GWEN_DIRECTORYDATA *d){
  return -1;
}



int GWEN_Directory_GetHomeDirectory(char *buffer, unsigned int size){
  return -1;
}




int GWEN_Directory_Create(const char *path){
  return -1;
}









