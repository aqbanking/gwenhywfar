/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri May 07 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef HTTPURLFNS_H
#define HTTPURLFNS_H

#include <gwenhywfar/httpurl.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Parses the given string and returns an URL object (if the string is ok).
 */
GWEN_HTTP_URL *GWEN_HttpUrl_fromString(const char *str);
const char *GWEN_HttpUrl_toString(const GWEN_HTTP_URL *url);

#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* HTTPURLFNS_H */
