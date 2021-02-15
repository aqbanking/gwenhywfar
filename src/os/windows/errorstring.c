/***************************************************************************
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


#include "errorstring.h"

#include <winsock2.h>
#include <windows.h>
#include <winerror.h>





const char *GWEN_ErrorString_Windows(int errnum)
{
  /* Modified new whole routine */
  static char stringWSAEINTR[] = "A blocking call was cancelled.";
  static char stringWSAEBADF[] = "WSAEBADF (10009) No description.";
  static char stringWSAEACCES[] =  "The requested address is a broadcast address.";
  static char stringWSAEFAULT[] =  "Bad address.";
  static char stringWSAEINVAL[] = "The socket is not bound to an address.";
  static char stringWSAEMFIL[] = "The maximum number of sockets has exceeded.";
  static char stringWSAEWOULDBLOCK[] = "Socket would block.";
  static char stringWSAEINPROGRESS[] = "A blocking call is in progress.";
  static char stringWSAEALREADY[] = "Operation already in progress.";
  static char stringWSAENOTSOCK[] = "Invalid socket descriptor.";
  static char stringWSAEDESTADDRREQ[] = "Destination address required.";
  static char stringWSAEMSGSIZE[] = "Message too long.";
  static char stringWSAEPROTOTYPE[] = "Protocol wrong type for socket.";
  static char stringWSAENOPROTOOPT[] = "Bad protocol option";
  static char stringWSAEPROTONOSUPPORT[] = "Protocol not supported.";
  static char stringWSAESOCKTNOSUPPORT[] = "Socket type not supported.";
  static char stringWSAEOPNOTSUPP[] = "Operation not supported.";
  static char stringWSAEPFNOSUPPORT[] = "Protocol family not supported.";
  static char stringWSAEAFNOSUPPORT[] = "Address family not supported by protocol family.";
  static char stringWSAEADDRINUSE[] = "The specified address is already in use.";
  static char stringWSAEADDRNOTAVAIL[] = "The specified address is not available from the local machine.";
  static char stringWSAENETDOWN[] = "The network subsystem has failed.";
  static char stringWSAENETUNREACH[] = "Network cannot be reached from this host at this time.";
  static char stringWSAENETRESET[] = "Network dropped connection on reset.";
  static char stringWSAECONNABORTED[] = "The virtual circuit was aborted due to timeout or other failure.";
  static char stringWSAECONNRESET[] = "The virtual circuit was reset by the remote side.";
  static char stringWSAENOBUFS[] = "No buffer space is available.";
  static char stringWSAEISCONN[] = "The socket is already connected.";
  static char stringWSAENOTCONN[] = "The socket is not connected.";
  static char stringWSAESHUTDOWN[] = "The socket has been shut down.";
  static char stringWSAETOOMANYREFS[] = "WSAETOOMANYREFS (10059) No description.";
  static char stringWSAETIMEDOUT[] = "Attempt to connect timed out without establishing a connection.";
  static char stringWSAECONNREFUSED[] = "The connection attempt was refused.";
  static char stringWSAELOOP[] = "WSAELOOP (10062) No description.";
  static char stringWSAENAMETOOLONG[] = "WSAENAMETOOLONG (10063) No description.";
  static char stringWSAEHOSTDOWN[] = "Host is down.";
  static char stringWSAEHOSTUNREACH[] = "No route to host.";
  static char stringWSAENOTEMPTY[] = "WSAENOTEMPTY (10065) No description.";
  static char stringWSAEPROCLIM[] = "Too many processes.";
  static char stringWSAEUSERS[] = "WSAEUSERS (10068) No description.";
  static char stringWSAEDQUOT[] = "WSAEDQUOT (10069) No description.";
  static char stringWSAESTALE[] = "WSAESTALE (10070) No description.";
  static char stringWSAEREMOTE[] = "WSAEREMOTE (10071) No description.";
  static char stringWSASYSNOTREADY[] = "Network subsystem is unavailable.";
  static char stringWSAVERNOTSUPPORTED[] = "Winsock.dll version out of range.";
  static char stringWSANOTINITIALISED[] = "Successful WSAStartup not yet performed.";
  static char stringWSAEDISCON[] = "Graceful shutdown in progress.";
  static char stringWSAENOMORE[] = "WSAENOMORE (10102) No description.";
  static char stringWSAECANCELLED[] = "WSAECANCELLED (10103) No description.";
  static char stringWSAEINVALIDPROVIDER[] = "WSAEINVALIDPROVIDER (10105) No description.";
  static char stringWSAEPROVIDERFAILEDINIT[] = "WSAEPROVIDERFAILEDINIT (10106) No description.";
  static char stringWSASYSCALLFAILURE[] = "System call failure.";
  static char stringWSASERVICE_NOT_FOUND[] = "WSASERVICE_NOT_FOUND (10108) No description.";
  static char stringWSATYPE_NOT_FOUND[] = "Class type not found.";
  static char stringWSA_E_NO_MORE[] = "WSA_E_NO_MORE (10110) No description.";
  static char stringWSA_E_CANCELLED[] = "WSA_E_CANCELLED No description.";
  static char stringWSAEREFUSED[] = "WSAEREFUSED No description.";
  static char stringWSAHOST_NOT_FOUND[] = "Authoritative Answer Host is not found.";
  static char stringWSATRY_AGAIN[] = "Nonauthoritative host not found.";
  static char stringWSANO_RECOVERY[] = "This is a nonrecoverable error.";
  static char stringWSANO_DATA[] = "Valid name, no data record of requested type.";
  /* static char stringWSANO_ADDRESS[] = "WSANO_ADDRESS (11004) No description."; */
  static char stringWSA_IO_PENDING[] = "Overlapped operations will complete later.";
  static char stringWSA_INVALID_HANDLE[] = "Specified event object handle is invalid.";
  static char stringWSA_INVALID_PARAMETER[] = "One or more parameters are invalid.";
  /* static char stringWSAINVALIDPROCTABLE[] = "Invalid procedure table from service provider."; */
  /* static char stringWSAINVALIDPROVIDER[] = "Invalid service provider version number."; */
  static char stringWSA_IO_INCOMPLETE[] = "Overlapped I/O event object not in signaled state.";
  static char stringWSA_NOT_ENOUGH_MEMORY[] = "Insufficient memory available.";
  /* static char stringWSAPROVIDERFAILEDINIT[] = "Unable to initialize a service provider."; */
  static char stringWSA_OPERATION_ABORTED[] = "Overlapped operation aborted.";
  static char stringDEFAULT[] = "Unknown Error";
  char *resultString;

  switch (errnum) {
  case WSAEINTR:
    /* (10004)
     * Interrupted function call.
     * A blocking operation was interrupted by a call to
     * WSACancelBlockingCall. */
    resultString = &stringWSAEINTR[0];
    break;
  case WSAEBADF:
    /* (10009)
     * No description */
    resultString = &stringWSAEBADF[0];
    break;

  case WSAEACCES:
    /* (10013)
     * Permission denied.
     * An attempt was made to access a socket in a way forbidden by
     * its access permissions.
     * An example is using a broadcast address for sendto without
     * broadcast permission being set using setsockopt(SO_BROADCAST).
     * Another possible reason for the WSAEACCES error is that when
     * the bind function is called (on Windows NT 4 SP4 or later),
     * another application, service, or kernel mode driver is bound
     * to the same address with exclusive access. Such exclusive access
     * is a new feature of Windows NT 4 SP4 and later, and is implemented
     * by using the SO_EXCLUSIVEADDRUSE option. */
    resultString = &stringWSAEACCES[0];
    break;

  case WSAEFAULT:
    /* (10014)
     * Bad address.
     * The system detected an invalid pointer address in attempting to
     * use a pointer argument of a call. This error occurs if an application
     * passes an invalid pointer value, or if the length of the buffer is
     * too small. For instance, if the length of an argument, which is a
     * SOCKADDR structure, is smaller than the sizeof(SOCKADDR). */
    resultString = &stringWSAEFAULT[0];
    break;

  case WSAEINVAL:
    /* (10022)
     * The socket is not bound to an address. */
    resultString = &stringWSAEINVAL[0];
    break;

  case WSAEMFILE:
    /* (10024)
     * Too many open files.
     * Too many open sockets. Each implementation may have a maximum number
     * of socket handles available, either globally, per process, or per
     * thread. */
    resultString = &stringWSAEMFIL[0];
    break;

  case WSAEWOULDBLOCK:
    /* (10035)
     * Resource temporarily unavailable.
     * This error is returned from operations on nonblocking sockets that
     * cannot be completed immediately, for example recv when no data is
     * queued to be read from the socket. It is a nonfatal error, and the
     * operation should be retried later. It is normal for WSAEWOULDBLOCK
     * to be reported as the result from calling connect on a nonblocking
     * SOCK_STREAM socket, since some time must elapse for the connection
     * to be established. */
    resultString = &stringWSAEWOULDBLOCK[0];
    break;

  case WSAEINPROGRESS:
    /* (10036)
     * Operation now in progress.
     * A blocking operation is currently executing. Windows Sockets only
     * allows a single blocking operation per- task or thread to be
     * outstanding, and if any other function call is made (whether or not it
     * references that or any other socket) the function fails with the
     * WSAEINPROGRESS error. */
    resultString = &stringWSAEINPROGRESS[0];
    break;

  case WSAEALREADY:
    /* (10037)
     * Operation already in progress.
     * An operation was attempted on a nonblocking socket with an operation
     * already in progress that is, calling connect a second time on a
     * nonblocking socket that is already connecting, or canceling an
     * asynchronous request (WSAAsyncGetXbyY) that has already been canceled
     * or completed. */
    resultString = &stringWSAEALREADY[0];
    break;

  case WSAENOTSOCK:
    /* (10038)
     * Socket operation on nonsocket.
     * An operation was attempted on something that is not a socket.
     * Either the socket handle parameter did not reference a valid
     * socket, or for select, a member of an fd_set was not valid. */
    resultString = &stringWSAENOTSOCK[0];
    break;

  case WSAEDESTADDRREQ:
    /* (10039)
     * Destination address required.
     * A required address was omitted from an operation on a socket.
     * For example, this error is returned if sendto is called with
     * the remote address of ADDR_ANY. */
    resultString = &stringWSAEDESTADDRREQ[0];
    break;

  case WSAEMSGSIZE:
    /* (10040)
     * Message too long.
     * A message sent on a datagram socket was larger than the internal
     * message buffer or some other network limit, or the buffer used
     * to receive a datagram was smaller than the datagram itself. */
    resultString = &stringWSAEMSGSIZE[0];
    break;

  case WSAEPROTOTYPE:
    /* (10041)
     * Protocol wrong type for socket.
     * A protocol was specified in the socket function call that does
     * not support the semantics of the socket type requested. For example,
     * the ARPA Internet UDP protocol cannot be specified with a socket type
     * of SOCK_STREAM. */
    resultString = &stringWSAEPROTOTYPE[0];
    break;

  case WSAENOPROTOOPT:
    /* (10042)
     * Bad protocol option.
     * An unknown, invalid or unsupported option or level was specified in
     * a getsockopt or setsockopt call. */
    resultString = &stringWSAENOPROTOOPT[0];
    break;

  case WSAEPROTONOSUPPORT:
    /* (10043)
     * Protocol not supported.
     * The requested protocol has not been configured into the system,
     * or no implementation for it exists. For example, a socket call
     * requests a SOCK_DGRAM socket, but specifies a stream protocol. */
    resultString = &stringWSAEPROTONOSUPPORT[0];
    break;

  case WSAESOCKTNOSUPPORT:
    /* (10044)
     * Socket type not supported.
     * The support for the specified socket type does not exist in
     * this address family. For example, the optional type SOCK_RAW
     * might be selected in a socket call, and the implementation does
     * not support SOCK_RAW sockets at all. */
    resultString = &stringWSAESOCKTNOSUPPORT[0];
    break;

  case WSAEOPNOTSUPP:
    /* (10045)
     * Operation not supported.
     * The attempted operation is not supported for the type of object
     * referenced. Usually this occurs when a socket descriptor to a
     * socket that cannot support this operation is trying to accept
     * a connection on a datagram socket. */
    resultString = &stringWSAEOPNOTSUPP[0];
    break;

  case WSAEPFNOSUPPORT:
    /* (10046)
     * Protocol family not supported.
     * The protocol family has not been configured into the system or no
     * implementation for it exists. This message has a slightly different
     * meaning from WSAEAFNOSUPPORT. However, it is interchangeable in most
     * cases, and all Windows Sockets functions that return one of these
     * messages also specify WSAEAFNOSUPPORT. */
    resultString = &stringWSAEPFNOSUPPORT[0];
    break;

  case WSAEAFNOSUPPORT:
    /* (10047)
     * Address family not supported by protocol family.
     * An address incompatible with the requested protocol was used.
     * All sockets are created with an associated address family
     * (that is, AF_INET for Internet Protocols) and a generic protocol
     * type (that is, SOCK_STREAM). This error is returned if an incorrect
     * protocol is explicitly requested in the socket call, or if
     * an address of the wrong family is used for a socket, for example,
     * in sendto. */
    resultString = &stringWSAEAFNOSUPPORT[0];
    break;

  case WSAEADDRINUSE:
    /* (10048)
     * Address already in use.
     * Typically, only one usage of each socket address (protocol/IP
     * address/port) is permitted. This error occurs if an application
     * attempts to bind a socket to an IP address/port that has already been
     * used for an existing socket, or a socket that was not closed properly,
     * or one that is still in the process of closing. For server applications
     * that need to bind multiple sockets to the same port number, consider
     * using setsockopt(SO_REUSEADDR). Client applications usually need not
     * call bind at all connect chooses an unused port automatically. When
     * bind is called with a wildcard address (involving ADDR_ANY), a
     * WSAEADDRINUSE error could be delayed until the specific address is
     * committed. This could happen with a call to another function
     * later, including connect, listen, WSAConnect, or WSAJoinLeaf. */
    resultString = &stringWSAEADDRINUSE[0];
    break;

  case WSAEADDRNOTAVAIL:
    /* (10049)
     * Cannot assign requested address.
     * The requested address is not valid in its context. This normally
     * results from an attempt to bind to an address that is not valid for the
     * local machine. This can also result from connect, sendto, WSAConnect,
     * WSAJoinLeaf, or WSASendTo when the remote address or port is not valid
     * for a remote machine (for example, address or port 0). */
    resultString = &stringWSAEADDRNOTAVAIL[0];
    break;

  case WSAENETDOWN:
    /* (10050)
     * Network is down.
     * A socket operation encountered a dead network. This could indicate a
     * serious failure of the network system (that is, the protocol stack
     * that the Windows Sockets DLL runs over), the network interface, or
     * the local network itself. */
    resultString = &stringWSAENETDOWN[0];
    break;

  case WSAENETUNREACH:
    /* (10051)
     * Network is unreachable.
     * A socket operation was attempted to an unreachable network. This
     * usually means the local software knows no route to reach the remote
     * host. */
    resultString = &stringWSAENETUNREACH[0];
    break;

  case WSAENETRESET:
    /* (10052)
     * Network dropped connection on reset.
     * The connection has been broken due to keep-alive activity detecting
     * a failure while the operation was in progress. It can also be returned
     * by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection
     * that has already failed. */
    resultString = &stringWSAENETRESET[0];
    break;

  case WSAECONNABORTED:
    /* ((10053)
     * (Software caused connection abort.
     * (An established connection was aborted by the software in your host
     * machine, (possibly due to a data transmission time-out or protocol
     * error. */
    resultString = &stringWSAECONNABORTED[0];
    break;

  case WSAECONNRESET:
    /* (10054)
     * Connection reset by peer.
     * An existing connection was forcibly closed by the remote host.
     * This normally results if the peer application on the remote host
     * is suddenly stopped, the host is rebooted, or the remote host uses
     * a hard close (see setsockopt for more information on the SO_LINGER
     * option on the remote socket.) This error may also result if a
     * connection was broken due to keep-alive activity detecting a failure
     * while one or more operations are in progress. Operations that were
     * in progress fail with WSAENETRESET. Subsequent operations fail
     * with WSAECONNRESET. */
    resultString = &stringWSAECONNRESET[0];
    break;

  case WSAENOBUFS:
    /* (10055)
     * No buffer space available.
     * An operation on a socket could not be performed because the system
     * lacked sufficient buffer space or because a queue was full. */
    resultString = &stringWSAENOBUFS[0];
    break;

  case WSAEISCONN:
    /* (10056)
     * Socket is already connected.
     * A connect request was made on an already-connected socket. Some
     * implementations also return this error if sendto is called on a
     * connected SOCK_DGRAM socket (for SOCK_STREAM sockets, the to parameter
     * in sendto is ignored) although other implementations treat this as
     * a legal occurrence. */
    resultString = &stringWSAEISCONN[0];
    break;

  case WSAENOTCONN:
    /* (10057)
     * Socket is not connected.
     * A request to send or receive data was disallowed because the socket is
     * not connected and (when sending on a datagram socket using sendto) no
     * address was supplied. Any other type of operation might also return
     * this error for example, setsockopt setting SO_KEEPALIVE if the
     * connection has been reset. */
    resultString = &stringWSAENOTCONN[0];
    break;

  case WSAESHUTDOWN:
    /* (10058)
     * Cannot send after socket shutdown.
     * A request to send or receive data was disallowed because the socket
     * had already been shut down in that direction with a previous shutdown
     * call. By calling shutdown a partial close of a socket is requested,
     * which is a signal that sending or receiving, or both have been
     * discontinued. */
    resultString = &stringWSAESHUTDOWN[0];
    break;

  case WSAETOOMANYREFS:
    /* (10059)
     * Keine Beschreibung */
    resultString = &stringWSAETOOMANYREFS[0];
    break;

  case WSAETIMEDOUT:
    /* ((10060)
     * (Connection timed out.
     * (A connection attempt failed because the connected party did not
     * properly (respond after a period of time, or the established connection
     * failed (because the connected host has failed to respond. */
    resultString = &stringWSAETIMEDOUT[0];
    break;

  case WSAECONNREFUSED:
    /* (10061)
     * Connection refused.
     * No connection could be made because the target machine actively refused
     * it. This usually results from trying to connect to a service that is
     * inactive on the foreign host�that is, one with no server application
     * running. */
    resultString = &stringWSAECONNREFUSED[0];
    break;

  case WSAELOOP:
    /* (10062)
     * No description */
    resultString = &stringWSAELOOP[0];
    break;

  case WSAENAMETOOLONG:
    /* (10063)
     * No description */
    resultString = &stringWSAENAMETOOLONG[0];
    break;

  case WSAEHOSTDOWN:
    /* (10064)
     * Host is down.
     * A socket operation failed because the destination host is down.
     * A socket operation encountered a dead host. Networking activity on the
     * local host has not been initiated. These conditions are more likely to
     * be indicated by the error WSAETIMEDOUT. */
    resultString = &stringWSAEHOSTDOWN[0];
    break;

  case WSAEHOSTUNREACH:
    /* (10065)
     * (No route to host.
     * A socket operation was attempted to an unreachable host.
     * See WSAENETUNREACH. */
    resultString = &stringWSAEHOSTUNREACH[0];
    break;

  case WSAENOTEMPTY:
    /* (10065)
     * No description */
    resultString = &stringWSAENOTEMPTY[0];
    break;

  case WSAEPROCLIM:
    /* (10067)
     * Too many processes.
     * A Windows Sockets implementation may have a limit on the number of
     * applications that can use it simultaneously. WSAStartup may fail with
     * this error if the limit has been reached. */
    resultString = &stringWSAEPROCLIM[0];
    break;

  case WSAEUSERS:
    /* (10068)
     * No description */
    resultString = &stringWSAEUSERS[0];
    break;

  case WSAEDQUOT:
    /* (10069)
     * (No description */
    resultString = &stringWSAEDQUOT[0];
    break;

  case WSAESTALE:
    /* (10070)
     * No description */
    resultString = &stringWSAESTALE[0];
    break;

  case WSAEREMOTE:
    /* (10071)
     * No description */
    resultString = &stringWSAEREMOTE[0];
    break;

  case WSASYSNOTREADY:
    /* (10091)
     * Network subsystem is unavailable.
     * This error is returned by WSAStartup if the Windows Sockets
     * implementation cannot function at this time because the underlying
     * system it uses to provide network services is currently unavailable.
     * Users should check:
     * That the appropriate Windows Sockets DLL file is in the current path.
     * That they are not trying to use more than one Windows Sockets
     * implementation simultaneously. If there is more than one Winsock DLL on
     * your system, be sure the first one in the path is appropriate for the
     * network subsystem currently loaded.
     * The Windows Sockets implementation documentation to be sure all
     * necessary components are currently installed and configured correctly.
     */
    resultString = &stringWSASYSNOTREADY[0];
    break;

  case WSAVERNOTSUPPORTED:
    /* (10092)
     * Winsock.dll version out of range.
     * The current Windows Sockets implementation does not support the
     * Windows Sockets specification version requested by the application.
     * Check that no old Windows Sockets DLL files are being accessed. */
    resultString = &stringWSAVERNOTSUPPORTED[0];
    break;

  case WSANOTINITIALISED:
    /* (10093)
     * Successful WSAStartup not yet performed.
     * Either the application has not called WSAStartup or WSAStartup failed.
     * The application may be accessing a socket that the current active task
     * does not own (that is, trying to share a socket between tasks), or
     * WSACleanup has been called too many times. */
    resultString = &stringWSANOTINITIALISED[0];
    break;

  case WSAEDISCON:
    /* (10101)
     * Graceful shutdown in progress.
     * Returned by WSARecv and WSARecvFrom to indicate that the remote party
     * has initiated a graceful shutdown sequence. */
    resultString = &stringWSAEDISCON[0];
    break;

  case WSAENOMORE:
    /* (10102)
     * No description */
    resultString = &stringWSAENOMORE[0];
    break;

  case WSAECANCELLED:
    /* (10103)
     * No description */
    resultString = &stringWSAECANCELLED[0];
    break;

  case WSAEINVALIDPROVIDER:
    /* (10105)
     * No description */
    resultString = &stringWSAEINVALIDPROVIDER[0];
    break;

  case WSAEPROVIDERFAILEDINIT:
    /* (10106)
     * No description */
    resultString = &stringWSAEPROVIDERFAILEDINIT[0];
    break;

  case WSASYSCALLFAILURE:
    /* (OS dependent)
     * System call failure.
     * Returned when a system call that should never fail does. For example,
     * if a call to WaitForMultipleObjects fails or one of the registry
     * functions fails trying to manipulate the protocol/name space catalogs.
     */
    resultString = &stringWSASYSCALLFAILURE[0];
    break;

  case WSASERVICE_NOT_FOUND:
    /* (10108)
     * No description */
    resultString = &stringWSASERVICE_NOT_FOUND[0];
    break;

  case WSATYPE_NOT_FOUND:
    /* (10109)
     * Class type not found.
     * The specified class was not found. */
    resultString = &stringWSATYPE_NOT_FOUND[0];
    break;

  case WSA_E_NO_MORE:
    /* (10110)
     * No description */
    resultString = &stringWSA_E_NO_MORE[0];
    break;

  case WSA_E_CANCELLED:
    /* No description */
    resultString = &stringWSA_E_CANCELLED[0];
    break;

  case WSAEREFUSED:
    /* No description */
    resultString = &stringWSAEREFUSED[0];
    break;

  case WSAHOST_NOT_FOUND:
    /* (11001)
     * Host not found.
     * No such host is known. The name is not an official host name or alias,
     * or it cannot be found in the database(s) being queried. This error may
     * also be returned for protocol and service queries, and means that the
     * specified name could not be found in the relevant database. */
    resultString = &stringWSAHOST_NOT_FOUND[0];
    break;

  case WSATRY_AGAIN:
    /* ((11002)
     * (Nonauthoritative host not found.
     * (This is usually a temporary error during host name resolution and
     * means (that the local server did not receive a response from an
     * authoritative (server. A retry at some time later may be successful. */
    resultString = &stringWSATRY_AGAIN[0];
    break;

  case WSANO_RECOVERY:
    /* (11003)
     * This is a nonrecoverable error.
     * This indicates some sort of nonrecoverable error occurred during a
     * database lookup. This may be because the database files (for example,
     * BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found,
     * or a DNS request was returned by the server with a severe error. */
    resultString = &stringWSANO_RECOVERY[0];
    break;

  case WSANO_DATA:
    /* (11004)
     * Valid name, no data record of requested type.
     * The requested name is valid and was found in the database, but it
     * does not have the correct associated data being resolved for. The
     * usual example for this is a host name-to-address translation attempt
     * (using gethostbyname or WSAAsyncGetHostByName) which uses the DNS
     * (Domain Name Server). An MX record is returned but no A record
     * indicating the host itself exists, but is not directly reachable. */
    resultString = &stringWSANO_DATA[0];
    break;

  /* Multiple defined
   *    case WSANO_ADDRESS:
   *                     * (11004)
   *                     * No description
   *      resultString = &stringWSANO_ADDRESS[0];
   *      break; */

  case WSA_IO_PENDING:
    /* (OS dependent)
     * Overlapped operations will complete later.
     * The application has initiated an overlapped operation that cannot be
     * completed immediately. A completion indication will be given later when
     * the operation has been completed. */
    resultString = &stringWSA_IO_PENDING[0];
    break;

  case WSA_INVALID_HANDLE:
    /* (OS dependent)
     * Specified event object handle is invalid.
     * An application attempts to use an event object, but the specified
     * handle is not valid. */
    resultString = &stringWSA_INVALID_HANDLE[0];
    break;

  case WSA_INVALID_PARAMETER:
    /* (OS dependent)
     * One or more parameters are invalid.
     * An application used a Windows Sockets function which directly maps to
     * a Win32 function. The Win32 function is indicating a problem with one
     * or more parameters. */
    resultString = &stringWSA_INVALID_PARAMETER[0];
    break;

  /* Not defined in GNU Compiler
   *     case WSAINVALIDPROCTABLE:
   * (OS dependent)
   * Invalid procedure table from service provider.
   * A service provider returned a bogus procedure table to Ws2_32.dll.
   * (Usually caused by one or more of the function pointers being null.)
   *       resultString = &stringWSAINVALIDPROCTABLE[0];
   *       break; */

  /* Not defined in GNU Compiler
   *    case WSAINVALIDPROVIDER:
   * (OS dependent)
   * Invalid service provider version number.
   * A service provider returned a version number other than 2.0.
   *      resultString = &stringWSAINVALIDPROVIDER[0];
   *      break; */

  case WSA_IO_INCOMPLETE:
    /* (OS dependent)
     * Overlapped I/O event object not in signaled state.
     * The application has tried to determine the status of an overlapped
     * operation which is not yet completed. Applications that use
     * WSAGetOverlappedResult (with the fWait flag set to FALSE) in a polling
     * mode to determine when an overlapped operation has completed, get this
     * error code until the operation is complete. */
    resultString = &stringWSA_IO_INCOMPLETE[0];
    break;

  case WSA_NOT_ENOUGH_MEMORY:
    /* (OS dependent)
     * Insufficient memory available.
     * An application used a Windows Sockets function that directly maps to
     * a Win32 function. The Win32 function is indicating a lack of required
     * memory resources. */
    resultString = &stringWSA_NOT_ENOUGH_MEMORY[0];
    break;

  /* Not defined in GNU Compiler
   *    case WSAPROVIDERFAILEDINIT:
   * (OS dependent)
   * Unable to initialize a service provider.
   * Either a service provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup/NSPStartup function failed.
   *      resultString = &stringWSAPROVIDERFAILEDINIT[0];
   *      break; */


  case WSA_OPERATION_ABORTED:
    /* (OS dependent)
     * Overlapped operation aborted. */
    resultString = &stringWSA_OPERATION_ABORTED[0];
    break;
  default:
    resultString = &stringDEFAULT[0];

  }
  return resultString;
}





