/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: ohbci_l.h 794 2005-07-26 16:13:16Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTTOKEN_OHBCI_L_H
#define GWEN_CRYPTTOKEN_OHBCI_L_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/ctplugin_be.h>


/** @name Constructor And Destructor
 *
 */
/*@{*/
GWEN_CRYPT_TOKEN *GWEN_Crypt_TokenOHBCI_new(GWEN_PLUGIN_MANAGER *pm,
					    const char *name);
/*@}*/




#endif /* GWEN_CRYPTTOKEN_OHBCI_L_H */

