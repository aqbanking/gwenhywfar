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
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Parses the given string and returns an URL object (if the string is ok).
 */
GWEN_HTTP_URL *GWEN_HttpUrl_fromString(const char *str);
int GWEN_HttpUrl_toString(const GWEN_HTTP_URL *url, GWEN_BUFFER *buf);

int GWEN_HttpUrl_toCommandString(const GWEN_HTTP_URL *url,
                                 GWEN_BUFFER *buf);

#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* HTTPURLFNS_H */
