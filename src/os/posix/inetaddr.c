/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "inetaddr_p.h"
#include <gwenhywfar/misc.h>
#include <stdlib.h>
#include <assert.h>
#include <netdb.h>
#include <string.h>
#include <sys/un.h>

#include "gwenhywfar/debug.h"

/* #define MEMTRACE */

/* needed for Solaris */
#ifndef SUN_LEN
# define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) \
  + strlen ((ptr)->sun_path))
#endif


/* forward declaration */
const char *GWEN_InetAddr_ErrorString(int c);

#ifdef MEMTRACE
static int GWEN_INETADDR__Counter=0;
#endif


static int gwen_inetaddr_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_inetaddr_errorform=0;


GWEN_TYPE_UINT32 GWEN_InetAddr_GetCapabilities(){
  return
    GWEN_INETADDR_CAPS_AF_TCP |
    GWEN_INETADDR_CAPS_AF_UNIX;
}



GWEN_ERRORCODE GWEN_InetAddr_ModuleInit(){
  if (!gwen_inetaddr_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_inetaddr_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_inetaddr_errorform,
                           GWEN_INETADDR_ERROR_TYPE);
    GWEN_ErrorType_SetMsgPtr(gwen_inetaddr_errorform,
			     GWEN_InetAddr_ErrorString);
    err=GWEN_Error_RegisterType(gwen_inetaddr_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_inetaddr_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_InetAddr_ModuleFini(){
  if (gwen_inetaddr_is_initialized) {
    GWEN_ERRORCODE err;

    err=GWEN_Error_UnregisterType(gwen_inetaddr_errorform);
    GWEN_ErrorType_free(gwen_inetaddr_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_inetaddr_is_initialized=0;
  }
  return 0;
}



const char *GWEN_InetAddr_ErrorString(int c) {
  const char *s;

  switch(c) {
  case GWEN_INETADDR_ERROR_MEMORY_FULL:
    s="Memory full";
    break;
  case GWEN_INETADDR_ERROR_BAD_ADDRESS:
    s="Bad address";
    break;
  case GWEN_INETADDR_ERROR_BUFFER_OVERFLOW:
    s="Buffer overflow";
    break;
  case GWEN_INETADDR_ERROR_HOST_NOT_FOUND:
    s="Host not found";
    break;
  case GWEN_INETADDR_ERROR_NO_ADDRESS:
    s="No address";
    break;
  case GWEN_INETADDR_ERROR_NO_RECOVERY:
    s="No recovery";
    break;
  case GWEN_INETADDR_ERROR_TRY_AGAIN:
    s="Try again";
    break;
  case GWEN_INETADDR_ERROR_UNKNOWN_DNS_ERROR:
    s="Unknown DNS error";
    break;
  case GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY:
    s="Unknown address family";
    break;
  default:
    s=(const char*)0;
  } /* switch */
  return s;
}



GWEN_INETADDRESS *GWEN_InetAddr_new(GWEN_AddressFamily af){
  GWEN_INETADDRESS *ia;

  GWEN_NEW_OBJECT(GWEN_INETADDRESS, ia);

  ia->af=af;
  switch(ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    ia->address=(struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    assert(ia->address);
    aptr=(struct sockaddr_in*)(ia->address);
    ia->size=sizeof(struct sockaddr_in);
    memset(ia->address,0,ia->size);
#ifdef PF_INET
    aptr->sin_family=PF_INET;
#else
    aptr->sin_family=AF_INET;
#endif
    break;
  }

  case GWEN_AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    ia->address=(struct sockaddr*)malloc(sizeof(struct sockaddr_un));
    assert(ia->address);
    aptr=(struct sockaddr_un*)(ia->address);
#if defined(PF_UNIX)
    aptr->sun_family=PF_UNIX;
#elif defined (AF_UNIX)
    aptr->sun_family=AF_UNIX;
#else
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    GWEN_InetAddr_free(ia);
    return 0;
#endif
    aptr->sun_path[0]=0;
    ia->size=sizeof(struct sockaddr_un);
    memset(ia->address,0,ia->size);
    break;
  }

  default:
    DBG_INFO(GWEN_LOGDOMAIN, "Unknown address family (%d)",af);
    assert(0);
  } /* switch */
#ifdef MEMTRACE
  GWEN_INETADDR__Counter++;
  DBG_INFO(GWEN_LOGDOMAIN, "InetAddr created, now %d",GWEN_INETADDR__Counter);
#endif
  return ia;
}



GWEN_INETADDRESS *GWEN_InetAddr_dup(const GWEN_INETADDRESS *oa){
  GWEN_INETADDRESS *ia;

  GWEN_NEW_OBJECT(GWEN_INETADDRESS, ia);
  ia->af=oa->af;
  ia->size=oa->size;
  //ia->address=(struct sockaddr *)malloc(sizeof(struct sockaddr));
  if (oa->size) {
    ia->address=(struct sockaddr *)malloc(oa->size);
    assert(ia->address);
    memmove(ia->address, oa->address, oa->size);
  }
  return ia;
}



void GWEN_InetAddr_free(GWEN_INETADDRESS *ia){
  if (ia) {
#ifdef MEMTRACE
    GWEN_INETADDR__Counter--;
    DBG_INFO(GWEN_LOGDOMAIN, "Free InetAddr, makes %d",GWEN_INETADDR__Counter);
#endif
    free(ia->address);
    GWEN_FREE_OBJECT(ia);
  }
}



GWEN_ERRORCODE GWEN_InetAddr_SetAddress(GWEN_INETADDRESS *ia,
					const char *addr){
  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    /* reset */
#ifdef PF_INET
    aptr->sin_family=PF_INET;
#elif defined (AF_UNIX)
    aptr->sun_family=AF_UNIX;
#else
    DBG_ERROR(GWEN_LOGDOMAIN, "No TCP sockets available for this system");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                          GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
#endif
    aptr->sin_addr.s_addr=0;

    if (addr) {
      /* ok, address to be set */
      if (!inet_aton(addr,&aptr->sin_addr))
        /* bad address, so maybe it rather is a name */
	return GWEN_Error_new(0,
                              GWEN_ERROR_SEVERITY_ERR,
                              GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                              GWEN_INETADDR_ERROR_BAD_ADDRESS);
    }
    break;
  }

  case GWEN_AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    aptr=(struct sockaddr_un*)(ia->address);
#ifdef PF_UNIX
    aptr->sun_family=PF_UNIX;
#elif defined (AF_UNIX)
    aptr->sun_family=AF_UNIX;
#else
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                          GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
#endif
    memset(aptr->sun_path, 0, sizeof(aptr->sun_path));
    if (addr) {
      /* ok, address to be set */
      if ((strlen(addr)+1)>sizeof(aptr->sun_path)) {
	/* bad address */
	DBG_INFO(GWEN_LOGDOMAIN, "Path too long (%d>%d)",
		  (int)(strlen(addr)+1),(int)(sizeof(aptr->sun_path)));
	return GWEN_Error_new(0,
			      GWEN_ERROR_SEVERITY_ERR,
			      GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
			      GWEN_INETADDR_ERROR_BAD_ADDRESS);
      }
      strcpy(aptr->sun_path, addr);
      ia->size=SUN_LEN(aptr);
    }
    break;
  }
  default:
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                          GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */

  return 0;
}



/* internal function */
int GWEN_InetAddr_TranslateHError(int herr) {
  int rv;

  switch(herr) {
  case HOST_NOT_FOUND:
    rv=GWEN_INETADDR_ERROR_HOST_NOT_FOUND;
    break;
#ifdef NO_ADDRESS
  case NO_ADDRESS:
    rv=GWEN_INETADDR_ERROR_NO_ADDRESS;
    break;
#endif
  case NO_RECOVERY:
    rv=GWEN_INETADDR_ERROR_NO_RECOVERY;
    break;
  case TRY_AGAIN:
    rv=GWEN_INETADDR_ERROR_TRY_AGAIN;
    break;
  default:
    rv=GWEN_INETADDR_ERROR_UNKNOWN_DNS_ERROR;
    break;
  } /* switch */
  return rv;
}



GWEN_ERRORCODE GWEN_InetAddr_SetName(GWEN_INETADDRESS *ia, const char *name){
  struct hostent *he;

  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    /* try to resolve name */
    he=gethostbyname(name);
    if (!he)
      return GWEN_Error_new(0,
                            GWEN_ERROR_SEVERITY_ERR,
                            GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                            GWEN_InetAddr_TranslateHError(h_errno));
    /* name resolved, store address */
    memcpy(&(aptr->sin_addr),
	   he->h_addr_list[0],
	   sizeof(struct in_addr));
    break;
  }

  case GWEN_AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    aptr=(struct sockaddr_un*)(ia->address);
#ifdef PF_INET
    aptr->sun_family=PF_INET;
#elif defined (AF_UNIX)
    aptr->sun_family=AF_UNIX;
#else
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                          GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
#endif
    aptr->sun_path[0]=0;

    if (name) {
      /* ok, address to be set */
      if ((strlen(name)+1)>sizeof(aptr->sun_path)) {
        /* bad address */
	DBG_INFO(GWEN_LOGDOMAIN, "Path too long (%d>%d)",
		  (int)(strlen(name)+1),(int)(sizeof(aptr->sun_path)));
	return GWEN_Error_new(0,
			 GWEN_ERROR_SEVERITY_ERR,
			 GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
			 GWEN_INETADDR_ERROR_BAD_ADDRESS);
      }
      strcpy(aptr->sun_path,name);
      ia->size=SUN_LEN(aptr);
    }
    break;
  }

  default:
    return GWEN_Error_new(0,
		     GWEN_ERROR_SEVERITY_ERR,
		     GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		     GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */
  return 0;
}



GWEN_ERRORCODE GWEN_InetAddr_GetAddress(const GWEN_INETADDRESS *ia,
					char *buffer, unsigned int bsize){
  const char *s;

  assert(ia);
  assert(buffer);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    s=inet_ntoa(aptr->sin_addr);
    assert(s);
    if (strlen(s)+1>bsize)
      return GWEN_Error_new(0,
		       GWEN_ERROR_SEVERITY_ERR,
		       GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		       GWEN_INETADDR_ERROR_BUFFER_OVERFLOW);
    strcpy(buffer,s);
    break;
  }

  case GWEN_AddressFamilyUnix: {
    struct sockaddr_un *aptr;
    int i;

    aptr=(struct sockaddr_un*)(ia->address);
    s=aptr->sun_path;
    i=ia->size;
    i-=sizeof(aptr->sun_family);
    if (i+1>(int)bsize)
      return GWEN_Error_new(0,
		       GWEN_ERROR_SEVERITY_ERR,
		       GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		       GWEN_INETADDR_ERROR_BUFFER_OVERFLOW);
    memmove(buffer,s,i);
    buffer[i]=0;
    break;
  }

  default:
    return GWEN_Error_new(0,
		     GWEN_ERROR_SEVERITY_ERR,
		     GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		     GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */
  return 0;
}



GWEN_ERRORCODE GWEN_InetAddr_GetName(const GWEN_INETADDRESS *ia,
				     char *buffer, unsigned int bsize){
  struct hostent *he;
  struct in_addr lia;

  assert(ia);
  assert(buffer);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    /* resolve name from address */
    lia=aptr->sin_addr;
#ifdef PF_INET
    he=gethostbyaddr((char*)&lia,sizeof(lia),PF_INET);
#else
    he=gethostbyaddr((char*)&lia,sizeof(lia),AF_INET);
#endif
    if (!he)
      return GWEN_Error_new(0,
		       GWEN_ERROR_SEVERITY_ERR,
		       GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		       GWEN_InetAddr_TranslateHError(h_errno));

    /* copy name into given buffer */
    assert(he->h_name);
    if (strlen(he->h_name)+1>bsize)
      return GWEN_Error_new(0,
		       GWEN_ERROR_SEVERITY_ERR,
		       GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		       GWEN_INETADDR_ERROR_BUFFER_OVERFLOW);
    /* copy the name into the buffer */
    strcpy(buffer,he->h_name);
    break;
  }

  case GWEN_AddressFamilyUnix: {
    struct sockaddr_un *aptr;

    aptr=(struct sockaddr_un*)(ia->address);
    /* simply copy path */
    if (strlen(aptr->sun_path)+1>bsize)
      return GWEN_Error_new(0,
		       GWEN_ERROR_SEVERITY_ERR,
		       GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		       GWEN_INETADDR_ERROR_BUFFER_OVERFLOW);
    strcpy(buffer,aptr->sun_path);
    break;
  }

  default:
    return GWEN_Error_new(0,
		     GWEN_ERROR_SEVERITY_ERR,
		     GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
		     GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);

  } /* switch */

  /* finished */
  return 0;
}



int GWEN_InetAddr_GetPort(const GWEN_INETADDRESS *ia){
  int i;

  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    i=ntohs(aptr->sin_port);
    break;
  }

  default:
    DBG_WARN(GWEN_LOGDOMAIN, "not an IP address");
    i=0;
  } /* switch */

  return i;
}



GWEN_ERRORCODE GWEN_InetAddr_SetPort(GWEN_INETADDRESS *ia, int port){
  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    aptr->sin_port=htons(port);
    break;
  }

  default:
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_INETADDR_ERROR_TYPE),
                          GWEN_INETADDR_ERROR_BAD_ADDRESS_FAMILY);
  } /* switch */
  return 0;
}






