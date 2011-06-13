/***************************************************************************
 begin       : Fri Jul 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "htmlimage_p.h"



GWEN_INHERIT_FUNCTIONS(HTML_IMAGE)
GWEN_LIST_FUNCTIONS(HTML_IMAGE, HtmlImage)


HTML_IMAGE *HtmlImage_new(void) {
  HTML_IMAGE *img;

  GWEN_NEW_OBJECT(HTML_IMAGE, img);
  img->refCount=1;
  GWEN_INHERIT_INIT(HTML_IMAGE, img);
  GWEN_LIST_INIT(HTML_IMAGE, img);

  return img;
}



void HtmlImage_free(HTML_IMAGE *img) {
  if (img) {
    assert(img->refCount);
    if (img->refCount>1)
      img->refCount--;
    else {
      GWEN_LIST_FINI(HTML_IMAGE, img);
      GWEN_INHERIT_FINI(HTML_IMAGE, img);
      free(img->imageName);

      img->refCount=0;
      GWEN_FREE_OBJECT(img);
    }
  }
}



void HtmlImage_Attach(HTML_IMAGE *img) {
  assert(img);
  assert(img->refCount);
  img->refCount++;
}



const char *HtmlImage_GetImageName(const HTML_IMAGE *img) {
  assert(img);
  assert(img->refCount);
  return img->imageName;
}



void HtmlImage_SetImageName(HTML_IMAGE *img, const char *s) {
  assert(img);
  assert(img->refCount);
  free(img->imageName);
  if (s) img->imageName=strdup(s);
  else img->imageName=NULL;
}



int HtmlImage_GetWidth(const HTML_IMAGE *img) {
  assert(img);
  assert(img->refCount);
  return img->width;
}



void HtmlImage_SetWidth(HTML_IMAGE *img, int i) {
  assert(img);
  assert(img->refCount);
  img->width=i;
}



int HtmlImage_GetHeight(const HTML_IMAGE *img) {
  assert(img);
  assert(img->refCount);
  return img->height;
}



void HtmlImage_SetHeight(HTML_IMAGE *img, int i) {
  assert(img);
  assert(img->refCount);
  img->height=i;
}



