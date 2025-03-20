/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWENHYWFAR_PARSER_TAG16_H
#define GWENHYWFAR_PARSER_TAG16_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/misc.h>


/** @defgroup MOD_TAG16 TLV Elements with 16-Bit Size Field
 * @ingroup MOD_PARSER
 *
 * @brief This file contains the definition of a GWEN_TAG16, a simple TLV (Tag-Length-Value) object.
 *
 * Such an object starts with a one byte tag identifier, followed by a two byte (little endian) data length
 * field and an optional data file. Thus such an object takes at three bytes of header and optional data.
 *
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_TAG16 GWEN_TAG16;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_TAG16, GWEN_Tag16, GWENHYWFAR_API)


GWENHYWFAR_API GWEN_TAG16 *GWEN_Tag16_new(void);
GWENHYWFAR_API GWEN_TAG16 *GWEN_Tag16_newNoCopy(unsigned int tagType, unsigned int tagLength, const uint8_t *tagData);
GWENHYWFAR_API GWEN_TAG16 *GWEN_Tag16_newCopy(unsigned int tagType, unsigned int tagLength, const uint8_t *tagData);
GWENHYWFAR_API void GWEN_Tag16_free(GWEN_TAG16 *tag);


GWENHYWFAR_API void GWEN_Tag16_WriteTagToBuffer(unsigned int tagType, const uint8_t *s, int size, GWEN_BUFFER *buf);
GWENHYWFAR_API void GWEN_Tag16_WriteStringTagToBuffer(unsigned int tagType, const char *s, GWEN_BUFFER *buf);
GWENHYWFAR_API void GWEN_Tag16_WriteUint8TagToBuffer(unsigned int tagType, uint8_t data, GWEN_BUFFER *buf);
GWENHYWFAR_API void GWEN_Tag16_WriteUint16TagToBuffer(unsigned int tagType, uint16_t data, GWEN_BUFFER *buf);
GWENHYWFAR_API void GWEN_Tag16_WriteUint32TagToBuffer(unsigned int tagType, uint32_t data, GWEN_BUFFER *buf);
GWENHYWFAR_API void GWEN_Tag16_WriteUint64TagToBuffer(unsigned int tagType, uint64_t data, GWEN_BUFFER *buf);

GWENHYWFAR_API void GWEN_Tag16_DirectlyToBuffer(unsigned int tagType,
                                                const char *p,
                                                int size,
                                                GWEN_BUFFER *buf);


GWENHYWFAR_API GWEN_TAG16 *GWEN_Tag16_fromBuffer(GWEN_BUFFER *mbuf, int isBerTlv);
GWENHYWFAR_API GWEN_TAG16 *GWEN_Tag16_fromBuffer2(const uint8_t *p, uint32_t l, int doCopy);

GWENHYWFAR_API unsigned int GWEN_Tag16_GetTagType(const GWEN_TAG16 *tag);
GWENHYWFAR_API unsigned int GWEN_Tag16_GetTagLength(const GWEN_TAG16 *tag);

GWENHYWFAR_API const void *GWEN_Tag16_GetTagData(const GWEN_TAG16 *tag);
GWENHYWFAR_API uint8_t GWEN_Tag16_GetTagDataAsUint8(const GWEN_TAG16 *tag, uint8_t defaultValue);
GWENHYWFAR_API uint16_t GWEN_Tag16_GetTagDataAsUint16(const GWEN_TAG16 *tag, uint16_t defaultValue);
GWENHYWFAR_API uint32_t GWEN_Tag16_GetTagDataAsUint32(const GWEN_TAG16 *tag, uint32_t defaultValue);
GWENHYWFAR_API uint64_t GWEN_Tag16_GetTagDataAsUint64(const GWEN_TAG16 *tag, uint64_t defaultValue);
GWENHYWFAR_API char *GWEN_Tag16_GetTagDataAsNewString(const GWEN_TAG16 *tag, const char *defaultValue);

/**
 * Return the full size of the complete tag including type, length and data fields.
 */
GWENHYWFAR_API unsigned int GWEN_Tag16_GetTagSize(const GWEN_TAG16 *tag);


GWENHYWFAR_API GWEN_TAG16_LIST *GWEN_Tag16_List_fromBuffer(const uint8_t *p, uint32_t l, int doCopy);
GWENHYWFAR_API const GWEN_TAG16 *GWEN_Tag16_List_FindFirstByTagType(const GWEN_TAG16_LIST *tagList, unsigned int tagType);
GWENHYWFAR_API const GWEN_TAG16 *GWEN_Tag16_List_FindNextByTagType(const GWEN_TAG16 *tag, unsigned int tagType);

/**
 * Use this function together with @ref GWEN_Tag16_EndTagInBuffer to create complex tags in a buffer.
 * This can be used to cascade tags into a buffer.
 *
 * Internally this function writes the begin of a TAG16 structure into buffer. After that you can add as many bytes
 * as you like. Finally call @ref GWEN_Tag16_EndTagInBuffer to finish the tag by correcting the size element of the
 * tag16 in the buffer.
 *
 * @return start position of the newly created tag in buffer (<0 on error)
 * @param tagType 8-bit type code/name/id of the tag (meaning of this code is up to the caller)
 * @param buf buffer to write into
 */
GWENHYWFAR_API int GWEN_Tag16_StartTagInBuffer(unsigned int tagType, GWEN_BUFFER *buf);

/**
 * Use this to finish a tag16 structure in buffer you started with @ref GWEN_Tag16_StartTagInBuffer.
 *
 * Internally this function adjusts the size element of the tag16 object in the buffer.
 *
 * @return 0 if okay, <0 on error
 * @param startPos value returned by @ref GWEN_Tag16_StartTagInBuffer (i.e. start of the tag in buffer)
 * @param buf buffer to write into
 */
GWENHYWFAR_API int GWEN_Tag16_EndTagInBuffer(int startPos, GWEN_BUFFER *buf);



/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_PARSER_TAG16_H */

