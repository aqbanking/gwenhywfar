/***************************************************************************
    begin       : Mon Jun 28 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_VIRTUALFN_H
#define TYPEMAKER2_VIRTUALFN_H


#include <gwenhywfar/list1.h>
#include <gwenhywfar/xml.h>

#include <stdio.h>

typedef struct TYPEMAKER2_VIRTUALFN TYPEMAKER2_VIRTUALFN;
GWEN_LIST_FUNCTION_DEFS(TYPEMAKER2_VIRTUALFN, Typemaker2_VirtualFn);


#define TYPEMAKER2_VIRTUALFN_FLAGS_FROMSLOT 0x00000001



TYPEMAKER2_VIRTUALFN *Typemaker2_VirtualFn_new();
void Typemaker2_VirtualFn_free(TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_Attach(TYPEMAKER2_VIRTUALFN *vf);

int Typemaker2_VirtualFn_readXml(TYPEMAKER2_VIRTUALFN *vf, GWEN_XMLNODE *node);


const char *Typemaker2_VirtualFn_GetName(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetName(TYPEMAKER2_VIRTUALFN *vf, const char *s);

const char *Typemaker2_VirtualFn_GetLocation(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetLocation(TYPEMAKER2_VIRTUALFN *vf, const char *s);

uint32_t Typemaker2_VirtualFn_GetFlags(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl);
void Typemaker2_VirtualFn_AddFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl);
void Typemaker2_VirtualFn_SubFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl);

int Typemaker2_VirtualFn_GetAccess(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetAccess(TYPEMAKER2_VIRTUALFN *vf, int i);

const char *Typemaker2_VirtualFn_GetReturnType(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetReturnType(TYPEMAKER2_VIRTUALFN *vf, const char *s);

const char *Typemaker2_VirtualFn_GetPreset(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetPreset(TYPEMAKER2_VIRTUALFN *vf, const char *s);

const char *Typemaker2_VirtualFn_GetDefaultReturnValue(const TYPEMAKER2_VIRTUALFN *vf);
void Typemaker2_VirtualFn_SetDefaultReturnValue(TYPEMAKER2_VIRTUALFN *vf, const char *s);

GWEN_STRINGLIST *Typemaker2_VirtualFn_GetParamTypes(const TYPEMAKER2_VIRTUALFN *vf);





#endif



