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


#ifndef GWEN_CRYPT_CRYPTDEFS_P_H
#define GWEN_CRYPT_CRYPTDEFS_P_H


#include "cryptdefs.h"



static int GWEN_Crypt__TransformFromBCD(unsigned char *buffer,
                                        unsigned int bufLength,
                                        unsigned int *pinLength);

static int GWEN_Crypt__TransformFromFPIN2(unsigned char *buffer,
    unsigned int bufLength,
    unsigned int *pinLength);

static int GWEN_Crypt__TransformFromBin(unsigned char *buffer,
                                        unsigned int bufLength,
                                        unsigned int *pinLength);

static int GWEN_Crypt__TransformToBCD(unsigned char *buffer,
                                      unsigned int bufLength,
                                      unsigned int *pinLength);

static int GWEN_Crypt__TransformToFPIN2(unsigned char *buffer,
                                        unsigned int bufLength,
                                        unsigned int *pinLength);

static int GWEN_Crypt__TransformToBin(unsigned char *buffer,
                                      unsigned int bufLength,
                                      unsigned int *pinLength);




#endif



