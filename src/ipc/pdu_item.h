/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_PDU_ITEM_H
#define GWEN_PDU_ITEM_H


#include <gwenhywfar/tree.h>
#include <gwenhywfar/testframework.h>

#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


/** Program Data Unit
 *
 * A program data unit as it is called in DICOM specs is a message-like structure containing data
 * to be exchanged by applications.
 * They are generally used in DICOM messages but are also usefull for other interprocess communication
 * purposes, thats why we use them in Gwenhywfar.
 */
typedef struct GWEN_PDU_ITEM GWEN_PDU_ITEM;
GWEN_TREE_FUNCTION_LIB_DEFS(GWEN_PDU_ITEM, GWEN_PduItem, GWENHYWFAR_API)


GWENHYWFAR_API GWEN_PDU_ITEM *GWEN_PduItem_new(void);
GWENHYWFAR_API void GWEN_PduItem_free(GWEN_PDU_ITEM *item);


GWENHYWFAR_API int8_t GWEN_PduItem_GetType(const GWEN_PDU_ITEM *item);
GWENHYWFAR_API void GWEN_PduItem_SetType(GWEN_PDU_ITEM *item, int8_t i);

GWENHYWFAR_API uint32_t GWEN_PduItem_GetLength(const GWEN_PDU_ITEM *item);
GWENHYWFAR_API const uint8_t *GWEN_PduItem_GetDataPointer(const GWEN_PDU_ITEM *item);

GWENHYWFAR_API void GWEN_PduItem_SetData(GWEN_PDU_ITEM *item, const uint8_t *ptr, uint32_t len, int takeOver);


/**
 * Read a PDU item from the given buffer.
 *
 * This version assumes the size spec is in 32 bits (e.g. PDUs themselves).
 *
 * @return number of bytes read from buffer (negative value on error)
 * @param item pointer to item to read data for
 * @param bufferPointer pointer to the buffer to read from
 * @param bufferLength size of the given buffer
 */
GWENHYWFAR_API int GWEN_PduItem_ReadBuffer32(GWEN_PDU_ITEM *item, const uint8_t *bufferPointer, uint32_t bufferLength);


/**
 * Read a PDU item header from the given buffer and return the total number of bytes the PDU uses.
 * This can be used when reading only the first few bytes of the PDU (e.g. when reading from network)
 * to determine how many bytes have to be read to fully parse the given PDU.
 *
 * This version assumes the size spec is in 32 bits (e.g. PDUs themselves).
 *
 * @return number of bytes read from buffer (negative value on error)
 * @param bufferPointer pointer to the buffer to read from
 * @param bufferLength size of the given buffer (must at least be 6 bytes)
 */
GWENHYWFAR_API int GWEN_PduItem_ReadHeader32(const uint8_t *bufferPointer, uint32_t bufferLength);

/**
 * Read a PDU item from the given buffer.
 *
 * This version assumes the size spec is in 16 bits (e.g. PDUs sub-items).
 *
 * @return number of bytes read from buffer (negative value on error)
 * @param item pointer to item to read data for
 * @param bufferPointer pointer to the buffer to read from
 * @param bufferLength size of the given buffer
 */
GWENHYWFAR_API int GWEN_PduItem_ReadBuffer16(GWEN_PDU_ITEM *item, const uint8_t *bufferPointer, uint32_t bufferLength);



/**
 * Read a PDU item header from the given buffer and return the total number of bytes the PDU uses.
 * This can be used when reading only the first few bytes of the PDU (e.g. when reading from network)
 * to determine how many bytes have to be read to fully parse the given PDU.
 *
 * This version assumes the size spec is in 16 bits (e.g. PDUs themselves).
 *
 * @return number of bytes read from buffer (negative value on error)
 * @param bufferPointer pointer to the buffer to read from
 * @param bufferLength size of the given buffer (must at least be 4 bytes)
 */
GWENHYWFAR_API int GWEN_PduItem_ReadHeader16(const uint8_t *bufferPointer, uint32_t bufferLength);


/**
 * Write a PDU item to the given buffer.
 *
 * This version assumes the size spec is in 32 bits (e.g. PDUs themselves).
 *
 * @return number of bytes written to the buffer (negative value on error)
 * @param item pointer to item to write data for
 * @param destBuffer buffer to write to
 */
GWENHYWFAR_API int GWEN_PduItem_WriteBuffer32(const GWEN_PDU_ITEM *item, GWEN_BUFFER *destBuffer);


/**
 * Write a PDU item to the given buffer.
 *
 * This version assumes the size spec is in 16 bits (e.g. PDUs sub-items).
 *
 * @return number of bytes written to the buffer (negative value on error)
 * @param item pointer to item to write data for
 * @param destBuffer buffer to write to
 */
GWENHYWFAR_API int GWEN_PduItem_WriteBuffer16(const GWEN_PDU_ITEM *item, GWEN_BUFFER *destBuffer);


/**
 * Directly write a PDU item to the given buffer without the need for an actual PDU object.
 */
GWENHYWFAR_API int GWEN_PduItem_DirectlyWritePdu16(int8_t t,
                                                   const uint8_t *dataPointer, uint32_t length,
                                                   GWEN_BUFFER *destBuffer);

/**
 * Directly write an integer PDU item to the given buffer without the need for an actual PDU object.
 */
GWENHYWFAR_API int GWEN_PduItem_DirectlyWritePdu16AsUint32(int8_t t, uint32_t i, GWEN_BUFFER *destBuffer);



/**
 * Write a PDU header (type, size) to the given buffer.
 *
 * This version assumes the size spec is in 32 bits (as in real PDUs items).
 *
 * @return number of bytes written to the buffer (negative value on error)
 * @param destBuffer buffer to write to
 * @param t type of the PDU/PDU item
 * @param length size of the PDU (can be 0 here if you call @ref GWEN_PduItem_AdjustItemSizeInBuffer32 later)
 */
GWENHYWFAR_API void GWEN_PduItem_AppendItemHeader32(GWEN_BUFFER *destBuffer, uint8_t t, uint32_t length);


/**
 * Write a PDU header (type, size) to the given buffer.
 *
 * This version assumes the size spec is in 16 bits (e.g. PDUs sub-items).
 *
 * @return number of bytes written to the buffer (negative value on error)
 * @param destBuffer buffer to write to
 * @param t type of the PDU/PDU item
 * @param length size of the PDU (can be 0 here if you call @ref GWEN_PduItem_AdjustItemSizeInBuffer16 later)
 */
GWENHYWFAR_API void GWEN_PduItem_AppendItemHeader16(GWEN_BUFFER *destBuffer, uint8_t t, uint32_t length);


/**
 * Adjust the size field in a PDU stored in the given buffer starting at the given position.
 *
 * The item whose size is to be adjusted is expected to start at the given position (i.e. first byte if the complete
 * PDU/PDU item) and to end at then current buffer pos (@ref GWEN_Buffer_GetPos).
 * The item/PDU size will be adjusted for the size of the item-type, the reserved byte at the 2nd byte and the size field so
 * that after calling this function the buffer contains a correct PDU/PDU item.
 *
 * This version is for 32 bit sizes (as in real APU objects).
 *
 * @param destBuffer buffer in which the APU size is to adjusted
 * @param startPos position of the first byte of the PDU/PDU item in the buffer
 */
GWENHYWFAR_API void GWEN_PduItem_AdjustItemSizeInBuffer32(GWEN_BUFFER *destBuffer, uint32_t startPos);

/**
 * Adjust the size field in a PDU stored in the given buffer starting at the given position.
 *
 * The item whose size is to be adjusted is expected to start at the given position (i.e. first byte if the complete
 * PDU/PDU item) and to end at then current buffer pos (@ref GWEN_Buffer_GetPos).
 * The item/PDU size will be adjusted for the size of the item-type, the reserved byte at the 2nd byte and the size field so
 * that after calling this function the buffer contains a correct PDU/PDU item.
 *
 * This version is for 16 bit sizes (as in APU sub-items).
 *
 * @param destBuffer buffer in which the APU size is to adjusted
 * @param startPos position of the first byte of the PDU/PDU item in the buffer
 */
GWENHYWFAR_API void GWEN_PduItem_AdjustItemSizeInBuffer16(GWEN_BUFFER *destBuffer, uint32_t startPos);



/**
 * Copy the value of the given PDU item to a string and return that string.
 *
 * @return string representing the given data (caller becomes responsible for freeing any data returned via @ref free)
 * @param item pointer to item to read data from
 */
GWENHYWFAR_API char *GWEN_PduItem_GetDataAsString(const GWEN_PDU_ITEM *item);



/**
 * Read the data of the given item and convert it to an uint32_t.
 *
 * @return uint32_t representing the item value
 * @param item pointer to item to read data from
 */
GWENHYWFAR_API uint32_t GWEN_PduItem_GetDataAsUint32(const GWEN_PDU_ITEM *item);


GWENHYWFAR_API void GWEN_PduItem_SetDataAsUint32(GWEN_PDU_ITEM *item, uint32_t i);


GWENHYWFAR_API void GWEN_PduItem_SetDataAsString(GWEN_PDU_ITEM *item, const char *s);


GWENHYWFAR_API void GWEN_PduItem_Dump(const GWEN_PDU_ITEM *item, int indent);

GWENHYWFAR_API int GWEN_PduItem_AddTests(GWEN_TEST_MODULE *mod);



#ifdef __cplusplus
}
#endif




#endif
