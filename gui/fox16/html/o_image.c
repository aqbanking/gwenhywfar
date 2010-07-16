/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "o_image_p.h"



GWEN_INHERIT(HTML_OBJECT, OBJECT_IMAGE);


static int HtmlObject_Image_Layout(HTML_OBJECT *o) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  HtmlObject_SetWidth(o, xo->scaledWidth);
  HtmlObject_SetHeight(o, xo->scaledHeight);
  return 0;
}




HTML_OBJECT *HtmlObject_Image_new(GWEN_XML_CONTEXT *ctx) {
  HTML_OBJECT *o;
  OBJECT_IMAGE *xo;

  o=HtmlObject_new(ctx, HtmlObjectType_Image);
  GWEN_NEW_OBJECT(OBJECT_IMAGE, xo);
  GWEN_INHERIT_SETDATA(HTML_OBJECT, OBJECT_IMAGE, o, xo, HtmlObject_Image_FreeData);
  HtmlObject_SetLayoutFn(o, HtmlObject_Image_Layout);

  return o;
}



void GWENHYWFAR_CB HtmlObject_Image_FreeData(void *bp, void *p) {
  OBJECT_IMAGE *xo;

  xo=(OBJECT_IMAGE*) p;
  HtmlImage_free(xo->image);

  GWEN_FREE_OBJECT(xo);
}



int HtmlObject_Image_GetScaledWidth(const HTML_OBJECT *o) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  return xo->scaledWidth;
}



void HtmlObject_Image_SetScaledWidth(HTML_OBJECT *o, int i) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  xo->scaledWidth=i;
}



int HtmlObject_Image_GetScaledHeight(const HTML_OBJECT *o) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  return xo->scaledHeight;
}



void HtmlObject_Image_SetScaledHeight(HTML_OBJECT *o, int i) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  xo->scaledHeight=i;
}



HTML_IMAGE *HtmlObject_Image_GetImage(const HTML_OBJECT *o) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  return xo->image;
}



void HtmlObject_Image_SetImage(HTML_OBJECT *o, HTML_IMAGE *img) {
  OBJECT_IMAGE *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_IMAGE, o);
  assert(xo);

  if (img)
    HtmlImage_Attach(img);
  HtmlImage_free(xo->image);
  xo->image=img;
}








