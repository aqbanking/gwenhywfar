/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef TYPEMAKER2_UTIL_H
#define TYPEMAKER2_UTIL_H


#include <inttypes.h>



uint32_t Typemaker2_FlagsFromString(const char *s);

uint32_t Typemaker2_TypeFlagsFromString(const char *t);


int Typemaker2_AccessFromString(const char *s);



#endif



