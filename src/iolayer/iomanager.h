/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IOMANAGER_H
#define GWEN_IOMANAGER_H

#include <gwenhywfar/iolayer.h>


#ifdef __cplusplus
extern "C" {
#endif



GWENHYWFAR_API DEPRECATED int GWEN_Io_Manager_RegisterLayer(GWEN_IO_LAYER *io);
GWENHYWFAR_API int GWEN_Io_Manager_UnregisterLayer(GWEN_IO_LAYER *io);


GWENHYWFAR_API GWEN_IO_LAYER_WORKRESULT GWEN_Io_Manager_Work(void);

GWENHYWFAR_API int GWEN_Io_Manager_Wait(int msecs, uint32_t guiid);

GWENHYWFAR_API int GWEN_Io_Manager_WaitForRequest(GWEN_IO_REQUEST *r, int msecs);


#ifdef __cplusplus
}
#endif


#endif

