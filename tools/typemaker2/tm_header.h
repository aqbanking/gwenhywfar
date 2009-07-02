/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_HEADER_H
#define TYPEMAKER2_HEADER_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/xml.h>


typedef struct TYPEMAKER2_HEADER TYPEMAKER2_HEADER;
GWEN_LIST_FUNCTION_DEFS(TYPEMAKER2_HEADER, Typemaker2_Header)


enum {
  Typemaker2_HeaderType_Unknown=-1,
  Typemaker2_HeaderType_System=0,
  Typemaker2_HeaderType_Local
};

enum {
  Typemaker2_HeaderLocation_Unknown=-1,
  Typemaker2_HeaderLocation_Pre=0,
  Typemaker2_HeaderLocation_Post,
  Typemaker2_HeaderLocation_Code
};


TYPEMAKER2_HEADER *Typemaker2_Header_new();
void Typemaker2_Header_free(TYPEMAKER2_HEADER *th);
void Typemaker2_Header_Attach(TYPEMAKER2_HEADER *th);


const char *Typemaker2_Header_GetFileName(const TYPEMAKER2_HEADER *th);
void Typemaker2_Header_SetFileName(TYPEMAKER2_HEADER *th, const char *s);

int Typemaker2_Header_GetType(const TYPEMAKER2_HEADER *th);
void Typemaker2_Header_SetType(TYPEMAKER2_HEADER *th, int i);

int Typemaker2_Header_GetLocation(const TYPEMAKER2_HEADER *th);
void Typemaker2_Header_SetLocation(TYPEMAKER2_HEADER *th, int i);

int Typemaker2_Header_readXml(TYPEMAKER2_HEADER *th, GWEN_XMLNODE *node);



#endif



