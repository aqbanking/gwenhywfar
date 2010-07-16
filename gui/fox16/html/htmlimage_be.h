/***************************************************************************
 begin       : Fri Jul 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLIMAGE_BE_H
#define HTMLIMAGE_BE_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/inherit.h>

#include <gwen-gui-fox16/api.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct HTML_IMAGE HTML_IMAGE;
GWEN_INHERIT_FUNCTION_LIB_DEFS(HTML_IMAGE, FOX16GUI_API)
GWEN_LIST_FUNCTION_LIB_DEFS(HTML_IMAGE, HtmlImage, FOX16GUI_API)


FOX16GUI_API 
HTML_IMAGE *HtmlImage_new();

FOX16GUI_API 
void HtmlImage_free(HTML_IMAGE *img);

FOX16GUI_API 
void HtmlImage_Attach(HTML_IMAGE *img);


FOX16GUI_API 
const char *HtmlImage_GetImageName(const HTML_IMAGE *img);

FOX16GUI_API 
void HtmlImage_SetImageName(HTML_IMAGE *img, const char *s);

FOX16GUI_API 
int HtmlImage_GetWidth(const HTML_IMAGE *img);

FOX16GUI_API 
void HtmlImage_SetWidth(HTML_IMAGE *img, int i);


FOX16GUI_API 
int HtmlImage_GetHeight(const HTML_IMAGE *img);

FOX16GUI_API 
void HtmlImage_SetHeight(HTML_IMAGE *img, int i);


#ifdef __cplusplus
}
#endif


#endif

