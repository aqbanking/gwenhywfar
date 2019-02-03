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

/*
 Changes

 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <gwenhywfar/gwenhywfar.h>
#include <windows.h>
#include <stdio.h>

/**
 * This is a simple initialisation function. It really does nothing (for
 * now).
 */
BOOL APIENTRY DllMain(HINSTANCE hInst,
                      DWORD reason,
                      LPVOID reserved)
{
  int err;

  switch (reason) {
  case DLL_PROCESS_ATTACH:
    err=GWEN_Init();
    if (err) {
      fprintf(stderr, "Could not initialize Gwenhywfar, aborting\n");
      return FALSE;
    }
    /* DEBUG */
    /* fprintf(stderr, "Gwenhywfar initialized.\n"); */
    break;

  case DLL_PROCESS_DETACH:
    err=GWEN_Fini();
    if (err) {
      fprintf(stderr, "Could not deinitialize Gwenhywfar\n");
    }
    /* DEBUG */
    /* fprintf(stderr, "Gwenhywfar deinitialized.\n"); */
    break;

  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}



