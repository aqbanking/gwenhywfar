/***************************************************************************
 begin       : Wed Jun 22 2011
 copyright   : (C) 2011 by Martin Preuss
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


#ifndef GWEN_SAR_H
#define GWEN_SAR_H


#include <gwenhywfar/gwen_sar_fileheader.h>
#include <gwenhywfar/syncio_file.h>



typedef struct GWEN_SAR GWEN_SAR;

GWENHYWFAR_API 
GWEN_SAR *GWEN_Sar_new(void);

GWENHYWFAR_API 
void GWEN_Sar_Attach(GWEN_SAR *sr);

GWENHYWFAR_API 
void GWEN_Sar_free(GWEN_SAR *sr);


GWENHYWFAR_API 
int GWEN_Sar_CreateArchive(GWEN_SAR *sr, const char *aname);

GWENHYWFAR_API 
int GWEN_Sar_OpenArchive(GWEN_SAR *sr, const char *aname,
                         GWEN_SYNCIO_FILE_CREATIONMODE cm,
                         uint32_t acc);

GWENHYWFAR_API 
int GWEN_Sar_CloseArchive(GWEN_SAR *sr, int abandon);


GWENHYWFAR_API 
int GWEN_Sar_AddFile(GWEN_SAR *sr, const char *fname);


GWENHYWFAR_API 
const GWEN_SAR_FILEHEADER_LIST *GWEN_Sar_GetHeaders(GWEN_SAR *sr);


GWENHYWFAR_API 
int GWEN_Sar_ExtractFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh);

GWENHYWFAR_API 
int GWEN_Sar_CheckFile(GWEN_SAR *sr, const GWEN_SAR_FILEHEADER *fh);




#endif





