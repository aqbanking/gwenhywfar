/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
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



#ifndef GWENHYFWAR_TEXT_H
#define GWENHYFWAR_TEXT_H

#include <gwenhyfwar/gwenhyfwarapi.h>

#ifdef __cplusplus
extern "C" {
#endif


#define GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS  0x00000001
#define GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS 0x00000002
#define GWEN_TEXT_FLAGS_DEL_MULTIPLE_BLANKS 0x00000004
#define GWEN_TEXT_FLAGS_NEED_DELIMITER      0x00000008
#define GWEN_TEXT_FLAGS_NULL_IS_DELIMITER   0x00000010


/**
 * This function cuts out a word from a given string.
 * @return address of the new word, 0 on error
 * @param src pointer to the beginning of the source string
 * @param delims pointer to a string containing all delimiters
 * @param buffer pointer to the destination buffer
 * @param maxsize length of the buffer. Actually up to this number of
 * characters are copied to the buffer. If after this number of chars no
 * delimiter follows the string will be terminated. You will have to check
 * whether there is a delimiter directly after the copied string
 * @param flags defines how the source string is to be processed
 * @param next pointer to a pointer to receive the address up to which the
 * source string has been handled. You can use this to continue with the
 * source string behind the word we've just cut out. This variable is only
 * modified upon successfull return
 */
char *GWEN_Text_GetWord(const char *src,
                        const char *delims,
                        char *buffer,
                        unsigned int maxsize,
                        unsigned int flags,
                        const char **next);

/**
 * This function does escaping like it is used for HTTP URL encoding.
 * All characters which are not alphanumeric are escaped by %XX where
 * XX ist the hexadecimal code of the character.
 */
char *GWEN_Text_Escape(const char *src,
                       char *buffer,
                       unsigned int maxsize);

char *GWEN_Text_Unescape(const char *src,
                         char *buffer,
                         unsigned int maxsize);

char *GWEN_Text_ToHex(const char *src, int l, char *buffer, unsigned maxsize);

/**
 * Converts a string to Hex. After "groupsize" bytes the "delimiter" is
 * inserted.
 */
char *GWEN_Text_ToHexGrouped(const char *src,
                             int l,
                             char *buffer,
                             unsigned maxsize,
                             unsigned int groupsize,
                             char delimiter,
                             int skipLeadingZeros);

int GWEN_Text_FromHex(const char *src, char *buffer, unsigned maxsize);

/**
 * @param fillchar if 0 then no filling takes place, positive values
 * extend to the right, negative values to the left.
 */
int GWEN_Text_NumToString(int num, char *buffer, unsigned int bufsize,
                          int fillchar);

/**
 * Compares two strings. If either of them is given but empty, that string
 * will be treaten as not given. This way a string NOT given equals a string
 * which is given but empty.
 * @param ign set to !=0 to ignore cases
 */
int GWEN_Text_Compare(const char *s1, const char *s2, int ign);


/**
 * This function compares two string and returns the number of matches or
 * -1 on error.
 * @param w string to compare
 * @param p pattern to compare against
 * @param sensecase if 0 then cases are ignored
 */
int GWEN_Text_ComparePattern(const char *w, const char *p, int sensecase);


#ifdef __cplusplus
}
#endif


#endif



