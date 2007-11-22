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
#include <string.h>

#include "gwenhywfar/debug.h"

/* #define MEMTRACE */

#ifdef MEMTRACE
static int GWEN_INETADDR__Counter=0;
#endif


uint32_t GWEN_InetAddr_GetCapabilities(){
  return
    GWEN_INETADDR_CAPS_AF_TCP;
}


int GWEN_InetAddr_ModuleInit(){
  return 0;
}



int GWEN_InetAddr_ModuleFini(){
  return 0;
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
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    GWEN_InetAddr_free(ia);
    return 0;
    break;
  }

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown address family (%d)",af);
    abort();
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
  ia->address=(struct sockaddr *)malloc(sizeof(struct sockaddr));
  assert(ia->address);
  memmove(ia->address, oa->address, sizeof(struct sockaddr));
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



int GWEN_InetAddr_SetAddress(GWEN_INETADDRESS *ia,
					const char *addr){
  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    /* reset */
#ifdef PF_INET
    aptr->sin_family=PF_INET;
#else
    aptr->sin_family=AF_INET;
#endif
    aptr->sin_addr.s_addr=0;


#if 1
    if (addr) {
      /* ok, address to be set */
      aptr->sin_addr.s_addr=inet_addr(addr);
      if (aptr->sin_addr.s_addr==INADDR_NONE)
        /* bad address, so maybe it rather is a name */
        return GWEN_ERROR_BAD_ADDRESS;
    }
#endif

#if 0 /* this would be much better, but windows does not have inet_aton */
    if (addr) {
      /* ok, address to be set */
      if (!inet_aton(addr,&aptr->sin_addr))
        /* bad address, so maybe it rather is a name */
	return GWEN_ERROR_BAD_ADDRESS;
    }
#endif
    break;
  }

  case GWEN_AddressFamilyUnix: {
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
    break;
  }
  default:
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
  } /* switch */

  return 0;
}


/* internal function */
int GWEN_InetAddr_TranslateHError(int herr) {
  int rv;

  switch(herr) {
  case WSAHOST_NOT_FOUND:
    rv=GWEN_ERROR_HOST_NOT_FOUND;
    break;
  case WSANO_ADDRESS:
    rv=GWEN_ERROR_NO_ADDRESS;
    break;
  case WSANO_RECOVERY:
    rv=GWEN_ERROR_NO_RECOVERY;
    break;
  case WSATRY_AGAIN:
    rv=GWEN_ERROR_TRY_AGAIN;
    break;
  default:
    rv=GWEN_ERROR_UNKNOWN_DNS_ERROR;
    break;
  } /* switch */
  return rv;
}



int GWEN_InetAddr_SetName(GWEN_INETADDRESS *ia, const char *name){
  struct hostent *he;

  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    /* try to resolve name */
    he=gethostbyname(name);
    if (!he)
      return GWEN_InetAddr_TranslateHError(h_errno);
    /* name resolved, store address */
    memcpy(&(aptr->sin_addr),
	   he->h_addr_list[0],
	   sizeof(struct in_addr));
    break;
  }

  case GWEN_AddressFamilyUnix: {
    DBG_ERROR(GWEN_LOGDOMAIN, "No unix domain sockets available for this system");
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
    break;
  }

  default:
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
  } /* switch */
  return 0;
}



int GWEN_InetAddr_GetAddress(const GWEN_INETADDRESS *ia,
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
      return GWEN_ERROR_BUFFER_OVERFLOW;
    strcpy(buffer,s);
    break;
  }

  case GWEN_AddressFamilyUnix: {
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
    break;
  }

  default:
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
  } /* switch */
  return 0;
}



int GWEN_InetAddr_GetName(const GWEN_INETADDRESS *ia,
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
      return GWEN_InetAddr_TranslateHError(h_errno);

    /* copy name into given buffer */
    assert(he->h_name);
    if (strlen(he->h_name)+1>bsize)
      return GWEN_ERROR_BUFFER_OVERFLOW;
    /* copy the name into the buffer */
    strcpy(buffer,he->h_name);
    break;
  }

  case GWEN_AddressFamilyUnix: {
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
    break;
  }

  default:
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;

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



int GWEN_InetAddr_SetPort(GWEN_INETADDRESS *ia, int port){
  assert(ia);

  switch (ia->af) {
  case GWEN_AddressFamilyIP: {
    struct sockaddr_in *aptr;

    aptr=(struct sockaddr_in*)(ia->address);
    aptr->sin_port=htons(port);
    break;
  }

  default:
    return GWEN_ERROR_BAD_ADDRESS_FAMILY;
  } /* switch */
  return 0;
}






