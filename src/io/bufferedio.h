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


#ifndef GWENHYWFAR_BUFFEREDIO_H
#define GWENHYWFAR_BUFFEREDIO_H "$Id$"

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/inherit.h>

#ifdef __cplusplus
extern "C" {
#endif
  typedef struct GWEN_BUFFEREDIOSTRUCT GWEN_BUFFEREDIO;
  GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_BUFFEREDIO, GWENHYWFAR_API)
  /* No trailing semicolon because this is a macro call */
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/error.h>

/* for compatibility only */
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/bio_socket.h>
#include <gwenhywfar/bio_buffer.h>
#include <gwenhywfar/buffer.h>



#ifdef __cplusplus
extern "C" {
#endif

#define GWEN_BUFFEREDIO_ERROR_TYPE    "BufferedIO"
#define GWEN_BUFFEREDIO_ERROR_READ    1
#define GWEN_BUFFEREDIO_ERROR_WRITE   2
#define GWEN_BUFFEREDIO_ERROR_CLOSE   3
#define GWEN_BUFFEREDIO_ERROR_TIMEOUT 4
#define GWEN_BUFFEREDIO_ERROR_PARTIAL 5
#define GWEN_BUFFEREDIO_ERROR_EOF     6
#define GWEN_BUFFEREDIO_ERROR_NO_DATA 7


#define GWEN_BUFFEREDIO_CHAR_ERROR   (-1)
#define GWEN_BUFFEREDIO_CHAR_EOF     (-2)
#define GWEN_BUFFEREDIO_CHAR_NO_DATA (-3)


/** If set then @ref GWEN_BufferedIO_Close is enabled */
#define GWEN_BUFFEREDIO_FLAGS_CLOSE   0x00000001

#define GWEN_BUFFEREDIO_FLAGS_DEFAULT \
  (\
  GWEN_BUFFEREDIO_FLAGS_CLOSE\
  )


#define GWEN_BUFFEREDIO_CBID_IO "GWEN_BUFFEREDIO_CBID_IO"


typedef enum {
  GWEN_LineModeUnix=0,
  GWEN_LineModeDOS
} GWEN_BUFFEREDIOLINEMODE;

typedef GWEN_ERRORCODE (*GWEN_BUFFEREDIOREADFN)(GWEN_BUFFEREDIO *dm,
                                                 char *buffer,
                                                  int *size,
                                                  int timeout);
typedef GWEN_ERRORCODE (*GWEN_BUFFEREDIOWRITEFN)(GWEN_BUFFEREDIO *dm,
                                                   const char *buffer,
                                                   int *size,
                                                   int timeout);
typedef GWEN_ERRORCODE (*GWEN_BUFFEREDIOCLOSEFN)(GWEN_BUFFEREDIO *dm);



/**
 * This function should not be used directly. It is used by "inheriting"
 * code (e.g. @ref GWEN_BufferedIO_File_new).
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_BUFFEREDIO *GWEN_BufferedIO_new();

/**
 * Frees a buffered IO context no matter of what type it is.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API void GWEN_BufferedIO_free(GWEN_BUFFEREDIO *dm);

/**
 * Set the read buffer to be used. This is only needed if you want to read
 * from a stream.
 * @author Martin Preuss<martin@aquamaniac.de>
 * @param buffer pointer to the buffer to be used. If 0 then this function
 * will allocate a buffer of len size. This function takes over ownership
 * of this buffer in any case ! This means you should never free this
 * buffer, this will be done by this module itself.
 * @param len length of the buffer (in bytes)
 */
GWENHYWFAR_API void GWEN_BufferedIO_SetReadBuffer(GWEN_BUFFEREDIO *bt,
                                                  char *buffer,
                                                  int len);

/**
 * Set the write buffer to be used. This is only needed if you want to write
 * to a stream.
 * @author Martin Preuss<martin@aquamaniac.de>
 * @param buffer pointer to the buffer to be used. If 0 then this function
 * will allocate a buffer of len size. This function takes over ownership
 * of this buffer in any case !
 * @param len length of the buffer (in bytes)
 */
GWENHYWFAR_API void GWEN_BufferedIO_SetWriteBuffer(GWEN_BUFFEREDIO *bt,
                                                   char *buffer,
                                                   int len);

/**
 * Checks whether the end of stream is reached.
 * @author Martin Preuss<martin@aquamaniac.de>
 * @return 0 if not, !=0 otherwise
 */
GWENHYWFAR_API int GWEN_BufferedIO_CheckEOF(GWEN_BUFFEREDIO *dm);

/**
 * Peeks at the next character in the stream. This functions reads the
 * next character without advancing the internal pointers, so the next
 * peek or read will return the very same character.
 * @author Martin Preuss<martin@aquamaniac.de>
 * @return -1 on error, character otherwise
 */
GWENHYWFAR_API int GWEN_BufferedIO_PeekChar(GWEN_BUFFEREDIO *dm);

/**
 * Reads the next character from the stream.
 * @author Martin Preuss<martin@aquamaniac.de>
 * @return -1 on error, character otherwise
 */
GWENHYWFAR_API int GWEN_BufferedIO_ReadChar(GWEN_BUFFEREDIO *dm);

/**
 * Really writes the content of the write buffer to the stream. This is
 * automatically called upon @ref GWEN_BufferedIO_Close.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_Flush(GWEN_BUFFEREDIO *bt);

/**
 * Really writes the content of the write buffer to the stream.
 * This function only flushes as much bytes as possible. If after
 * calling this function some bytes still remain inside the write
 * buffer, a warning will be returned
 * (error code=GWEN_BUFFEREDIO_ERROR_PARTIAL,
 * severity=GWEN_ERROR_SEVERITY_WARN). You should then call this function as
 * long as it returns this warning in order to really save the data.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_ShortFlush(GWEN_BUFFEREDIO *bt);

/*
 * Returns !=0 if the read buffer is empty, 0 if it is not.
 */
GWENHYWFAR_API int GWEN_BufferedIO_ReadBufferEmpty(GWEN_BUFFEREDIO *bt);


/*
 * Returns !=0 if the write buffer is empty, 0 if it is not.
 */
GWENHYWFAR_API int GWEN_BufferedIO_WriteBufferEmpty(GWEN_BUFFEREDIO *bt);

/**
 * Reads a line until a CR (in Unix mode) or a CRLF (DOS mode) is found
 * or the buffer is filled, whichever comes first.
 * The trailing CR or CRLF is not copied into the buffer.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_ReadLine(GWEN_BUFFEREDIO *bt,
                                                       char *buffer,
                                                       unsigned int s);

/**
 * Reads a line until a CR (in Unix mode) or a CRLF (DOS mode) is found.
 * The trailing CR or CRLF is not copied into the buffer.
 * This function uses a GWEN_BUFFER to store the data.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API
GWEN_ERRORCODE GWEN_BufferedIO_ReadLine2Buffer(GWEN_BUFFEREDIO *bt,
                                               GWEN_BUFFER *buffer);

/**
 * Writes a character into the stream.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_WriteChar(GWEN_BUFFEREDIO *dm,
                                                        char c);

/**
 * Writes the content of the buffer to the stream until a null byte is
 * found.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_Write(GWEN_BUFFEREDIO *bt,
                                                    const char *buffer);

/**
 * Writes the content of the given buffer plus appropriate line end
 * character(s) (CR in Unix mode and CR/LF in DOS mode) into the stream.
 * Please note that the buffer should not contain CR or CR/LF characters.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_WriteLine(GWEN_BUFFEREDIO *bt,
                                                        const char *buffer);

/**
 * Closes the stream after flushing the output buffer (if needed).
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API GWEN_ERRORCODE GWEN_BufferedIO_Close(GWEN_BUFFEREDIO *dm);


/**
 * Set the line mode to be used. Currently there are:
 * <ul>
 *  <li>LineModeUnix (using CR as line terminator)</li>
 *  <li>LineModeDOS (using CR/LF as line terminator)</li>
 * </ul>
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API
void GWEN_BufferedIO_SetLineMode(GWEN_BUFFEREDIO *dm,
                                 GWEN_BUFFEREDIOLINEMODE lm);

/**
 * Returns the current flags for this bufferedIO.
 * See @ref GWEN_BUFFEREDIO_FLAGS_CLOSE and others for details.
 */
GWENHYWFAR_API
GWEN_TYPE_UINT32 GWEN_BufferedIO_GetFlags(const GWEN_BUFFEREDIO *bt);

/**
 * Sets the flags for this bufferedIO.
 * See @ref GWEN_BUFFEREDIO_FLAGS_CLOSE and others for details.
 */
GWENHYWFAR_API
void GWEN_BufferedIO_SetFlags(GWEN_BUFFEREDIO *bt, GWEN_TYPE_UINT32 f);

/**
 * Adds the given flags to the current flags for this bufferedIO.
 * See @ref GWEN_BUFFEREDIO_FLAGS_CLOSE and others for details.
 */
GWENHYWFAR_API
void GWEN_BufferedIO_AddFlags(GWEN_BUFFEREDIO *bt, GWEN_TYPE_UINT32 f);

/**
 * Removes the given flags from the current flags for this bufferedIO.
 * See @ref GWEN_BUFFEREDIO_FLAGS_CLOSE and others for details.
 */
GWENHYWFAR_API
void GWEN_BufferedIO_SubFlags(GWEN_BUFFEREDIO *bt, GWEN_TYPE_UINT32 f);


/**
 * Return the currently used line mode.
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API
GWEN_BUFFEREDIOLINEMODE GWEN_BufferedIO_GetLineMode(GWEN_BUFFEREDIO *dm);


/**
 * Set the timeout for read- and write operations in milliseconds.
 * Some values have special meanings:
 * <ul>
 *  <li>0: no timeout (not blocking)</li>
 *  <li>-1: wait forever (blocking)</li>
 * </ul>
 * @author Martin Preuss<martin@aquamaniac.de>
 */
GWENHYWFAR_API
void GWEN_BufferedIO_SetTimeout(GWEN_BUFFEREDIO *dm, int timeout);


/**
 * Returns the currently used timeout value in milliseconds.
 */
GWENHYWFAR_API
int GWEN_BufferedIO_GetTimeout(GWEN_BUFFEREDIO *dm);


GWENHYWFAR_API
int GWEN_BufferedIO_GetLines(const GWEN_BUFFEREDIO *dm);


GWENHYWFAR_API
int GWEN_BufferedIO_GetBytesRead(const GWEN_BUFFEREDIO *dm);

GWENHYWFAR_API
int GWEN_BufferedIO_GetBytesWritten(const GWEN_BUFFEREDIO *dm);


GWENHYWFAR_API
void GWEN_BufferedIO_SetReadFn(GWEN_BUFFEREDIO *dm,
                               GWEN_BUFFEREDIOREADFN fn);
GWENHYWFAR_API
void GWEN_BufferedIO_SetWriteFn(GWEN_BUFFEREDIO *dm,
                                GWEN_BUFFEREDIOWRITEFN fn);
GWENHYWFAR_API
void GWEN_BufferedIO_SetCloseFn(GWEN_BUFFEREDIO *dm,
                                GWEN_BUFFEREDIOCLOSEFN fn);


/**
 * Closes the stream without flushing any buffers. Call this in case
 * of a severe error. The content of the internal buffers will be lost !
 */
GWENHYWFAR_API
GWEN_ERRORCODE GWEN_BufferedIO_Abandon(GWEN_BUFFEREDIO *dm);


/**
 * Writes the content of the given buffer. If there still is some data
 * inside the internal buffer that data will be flushed first.
 * This allows for mixing calls to this function with calls to the
 * character based write functions.
 * @param bsize pointer to a variable which holds the number of bytes
 * to write. Upon return this variable shows the number of bytes actually
 * written. Please note that if this function has to flush internal buffers
 * the return value might be 0 (indicating that no error occurred, but the
 * internal buffers still contain some bytes which will be flushed upon the
 * next call to this function).
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_BufferedIO_WriteRaw(GWEN_BUFFEREDIO *bt,
                                          const char *buffer,
                                          unsigned int *bsize);


/**
 * Reads multiple bytes. If there is some data inside the internal buffers
 * then that data will be returned first. This allows for mixing with
 * character based read functions.
 * @param bsize pointer to a variable which holds the number of bytes
 * to read. Upon return this variable shows the number of bytes actually
 * read.
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_BufferedIO_ReadRaw(GWEN_BUFFEREDIO *bt,
                                         char *buffer,
                                         unsigned int *bsize);

/**
 * Reads exactly the amount of bytes given or returns an error.
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_BufferedIO_ReadRawForced(GWEN_BUFFEREDIO *bt,
                                               char *buffer,
                                               unsigned int *bsize);



#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_BUFFEREDIO_H */





