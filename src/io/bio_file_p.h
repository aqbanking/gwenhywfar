/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Feb 07 2003
 copyright   : (C) 2003 by Martin Preuss
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


#ifndef GWENHYWFAR_BIO_FILE_P_H
#define GWENHYWFAR_BIO_FILE_P_H

#include <gwenhywfar/bio_file.h>

#define GWEN_BUFFEREDIO_FILE_TIMEOUT   20000


struct GWEN_BUFFEREDIO_FILE {
  int fd;
};
typedef struct GWEN_BUFFEREDIO_FILE GWEN_BUFFEREDIO_FILE;


GWEN_BUFFEREDIO_FILE *GWEN_BufferedIO_File_Table__new();
void GWEN_BufferedIO_File_Table__free(GWEN_BUFFEREDIO_FILE *bft);
GWEN_ERRORCODE GWEN_BufferedIO_File__Read(GWEN_BUFFEREDIO *dm,
                                          char *buffer,
                                          int *size,
                                          int timeout);
GWEN_ERRORCODE GWEN_BufferedIO_File__Write(GWEN_BUFFEREDIO *dm,
                                           const char *buffer,
                                           int *size,
                                           int timeout);


GWEN_ERRORCODE GWEN_BufferedIO_File__Close(GWEN_BUFFEREDIO *dm);

void GWEN_BufferedIO_File_FreeData(void *bp, void *p);





#endif /* GWENHYWFAR_BIO_FILE_P_H */





