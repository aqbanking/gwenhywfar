/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed May 05 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GWEN_BIO_NETLAYER_P_H
#define GWEN_BIO_NETLAYER_P_H

#include "bio_netlayer_l.h"


int BIO_netlayer_write(BIO *bio, const char *out, int outl);
int BIO_netlayer_read(BIO *bio, char *in, int inl);
int BIO_netlayer_puts(BIO *bio, const char *str);
int BIO_netlayer_create(BIO *bio);
int BIO_netlayer_destroy(BIO *bio);
long BIO_netlayer_ctrl(BIO *bio, int cmd, long num, void *ptr);



#endif /* GWEN_BIO_NETLAYER_P_H */


