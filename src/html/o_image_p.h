/***************************************************************************
 begin       : Fri Jul 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLOBJECT_IMAGE_P_H
#define HTMLOBJECT_IMAGE_P_H


#include "o_image_be.h"


typedef struct OBJECT_IMAGE OBJECT_IMAGE;
struct OBJECT_IMAGE {
  int scaledWidth;
  int scaledHeight;
  HTML_IMAGE *image;
};
static void GWENHYWFAR_CB HtmlObject_Image_FreeData(void *bp, void *p);


#endif

