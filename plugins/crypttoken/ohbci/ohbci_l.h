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


#ifndef GWEN_CRYPTTOKEN_OHBCI_L_H
#define GWEN_CRYPTTOKEN_OHBCI_L_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypttoken.h>


/** @name Constructor And Destructor
 *
 */
/*@{*/
GWEN_CRYPTTOKEN *GWEN_CryptTokenOHBCI_new(GWEN_PLUGIN_MANAGER *pm,
                                          const char *subTypeName,
                                          const char *name);
/*@}*/




#endif /* GWEN_CRYPTTOKEN_OHBCI_L_H */

