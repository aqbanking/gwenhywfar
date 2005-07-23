/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTTOKEN_OHBCI_H
#define GWEN_CRYPTTOKEN_OHBCI_H

#include <aqhbci/medium.h>
#include <aqhbci/mediumrdh.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/keyspec.h>


/** @name Constructor And Destructor
 *
 */
/*@{*/
AH_MEDIUM *GWEN_CryptTokenOHBCI_new(AH_HBCI *hbci,
                              AH_MEDIUMPROVIDER *mp,
                              const char *mediumName);
/*@}*/




#endif /* GWEN_CRYPTTOKEN_OHBCI_H */

