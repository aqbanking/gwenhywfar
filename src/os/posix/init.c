/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 27 2002
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


#include <gwenhywfar/gwenhywfar.h>
#include <stdio.h>
#include <stdlib.h>


static void GWEN_LibInit() __attribute__((constructor));
static void GWEN_LibFini() __attribute__((destructor));


void GWEN_LibInit() {
  GWEN_ERRORCODE err;

  err=GWEN_Init();
  if (!GWEN_Error_IsOk(err)) {
    fprintf(stderr, "Could not initialize Gwenhywfar, aborting\n");
    abort();
  }
  /* fprintf(stderr, "Gwenhywfar initialized.\n"); */
}


void GWEN_LibFini() {
  GWEN_ERRORCODE err;

  err=GWEN_Fini_Forced();
  if (!GWEN_Error_IsOk(err)) {
    fprintf(stderr, "Could not deinitialize Gwenhywfar\n");
  }
  /* fprintf(stderr, "Gwenhywfar deinitialized.\n"); */
}

