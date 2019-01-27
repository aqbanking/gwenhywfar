/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWENHYWFAR_TLV_H
#define GWENHYWFAR_TLV_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>


typedef struct GWEN_TLV GWEN_TLV;

GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_TLV, GWEN_TLV, GWENHYWFAR_API)


GWENHYWFAR_API
GWEN_TLV *GWEN_TLV_new(void);
GWENHYWFAR_API
void GWEN_TLV_free(GWEN_TLV *tlv);

GWENHYWFAR_API
GWEN_TLV *GWEN_TLV_create(unsigned int tagType,
                          unsigned int tagMode,
                          const void *p,
                          unsigned int dlen,
                          int isBerTlv);


GWENHYWFAR_API
GWEN_TLV *GWEN_TLV_fromBuffer(GWEN_BUFFER *mbuf, int isBerTlv);

GWENHYWFAR_API
int GWEN_TLV_toBuffer(GWEN_TLV *tlv, GWEN_BUFFER *mbuf);

GWENHYWFAR_API
int GWEN_TLV_DirectlyToBuffer(unsigned int tagType,
                              unsigned int tagMode,
                              const void *tagData,
                              int tagLength,
                              int isBerTlv,
                              GWEN_BUFFER *mbuf);


GWENHYWFAR_API
int GWEN_TLV_IsBerTlv(const GWEN_TLV *tlv);

GWENHYWFAR_API
unsigned int GWEN_TLV_GetTagType(const GWEN_TLV *tlv);
GWENHYWFAR_API
unsigned int GWEN_TLV_GetTagLength(const GWEN_TLV *tlv);
GWENHYWFAR_API
const void *GWEN_TLV_GetTagData(const GWEN_TLV *tlv);

GWENHYWFAR_API
int GWEN_TLV_IsContructed(const GWEN_TLV *tlv);
GWENHYWFAR_API
unsigned int GWEN_TLV_GetClass(const GWEN_TLV *tlv);
GWENHYWFAR_API
unsigned int GWEN_TLV_GetTagSize(const GWEN_TLV *tlv);


/**
 * Reads a TLV header from a buffer (tag and length) and returns the number of bytes
 * used by the header.
 * @return number of bytes used to specify tag and length (i.e. TLV size without the data part)
 * @param tlv TLV to read the header into
 * @param p pointer to a buffer containing at least the header
 * @param size siez of the buffer pointed to by @b p
 * @param isBerTlv if !=0 then the TLV is supposed to be a BER-TLV object
 */
GWENHYWFAR_API
int GWEN_TLV_ReadHeader(GWEN_TLV *tlv, const uint8_t *p, uint32_t size, int isBerTlv);


/**
 * Directly writes a TLV header (tag and length) to the given buffer.
 * @return 0 if ok, error code otherwise
 * @param tagType tag id
 * @param tagMode bits 5, 6 and 7 of the first TLV byte (bits 6/7: class, bit 5: constructed)
 * @param tagLength length of the data part introduced by the TLV header
 * @param isBerTlv if !=0 then the TLV is supposed to be a BER-TLV object
 * @param mbuf buffer to receive the TLV header
 */
GWENHYWFAR_API
int GWEN_TLV_WriteHeader(unsigned int tagType,
                         unsigned int tagMode,
                         uint64_t tagLength,
                         int isBerTlv,
                         GWEN_BUFFER *mbuf);


/**
 * Directly parses a BER-TLV structure do a DB.
 * @return number of bytes parsed
 * @param dbRecord GWEN_DB database with the parsed TLV structure
 * @param mbuf buffer holding the TLV data
 * @param len length of the data in the buffer
 */

GWENHYWFAR_API
int GWEN_TLV_Buffer_To_DB(GWEN_DB_NODE *dbRecord, GWEN_BUFFER *mbuf, int len);

/**
 * Parses length of a BER-TLV structure.
 * @return number of bytes including the tag itself
 * @param mbuf buffer holding the TLV data
 * @param tag_len_len number of bytes used for tag identifier octets and length octets
 */

GWENHYWFAR_API
uint32_t GWEN_TLV_ParseLength(GWEN_BUFFER *mbuf, uint32_t *tag_len_len);

#endif /* GWENHYWFAR_TLV_H */

