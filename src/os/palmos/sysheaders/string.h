

#ifndef GWEN_PALMOS_STRING_H
#define GWEN_PALMOS_STRING_H

#include <MemoryMgr.h>
#include <StringMgr.h>


typedef void FILE;

#define fprintf(x, args...) printf(## args)

#define	strcpy(x,y)     StrCopy((Char*)x,(Char*)y)
#define	strncpy(x,y,z)  StrNCopy(x,y,z)
#define	strcmp(x,y)     StrCompare(x,y)
#define	strcasecmp(x,y) StrCaselessCompare(x,y)
#define	strcat(x,y)     StrCat(x,y)
#define	strlen(x)       StrLen((Char*)x)
#define	strchr(x,y)     StrChr(x,y)
#define	strstr(x,y)     StrStr(x,y)

#define	memcpy(x,y,z)   MemMove(x,(void *)y,z)
#define	memmove(x,y,z)  MemMove(x,(void *)y,z)
#define	memset(x,y,z)   MemSet(x,z,y)
#define	memcmp(x,y,z)   MemCmp(x,y,z)


/* TODO */

#define fopen(x, y) (0)
#define fclose(x) (-1)


#define strerror(x) ("ERROR")
#define strdup(x) ("strdup not implemented")



#endif
