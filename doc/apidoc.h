
/*
 * This file is only to define a basic structure for the API documentation
 */

#ifndef GWENHYWFAR_APIDOC_H
#define GWENHYWFAR_APIDOC_H

/** @defgroup MOD_BASE Basic Stuff */
/** @defgroup MOD_OS Operating System Abstraction */
/** @defgroup MOD_IO Input Output Functions */
/** @defgroup MOD_NET Networking Functions */
/** @defgroup MOD_IPC Inter Process communication */
/** @defgroup MOD_PARSER Data Management And Parsing */
/*@{*/
/** @defgroup MOD_HBCI Implementation of the HBCI Protocol */
/*@}*/
/** @defgroup MOD_CRYPT Cryptographic Functions */


/** @mainpage Gwenhywfar Documentation Main Page
 *

 This is Gwenhywfar, a multi-platform helper library for networking and
 security applications and libraries. It is heavily used by
 <a href="http://www.libchipcard.de">Libchipcard</a>
 and
 <a href="http://www.openhbci.de">OpenHBCI-TNG</a> (The Next Generation).


 @section GWEN_features Features

 This library is written in C, and it follows the object-oriented
 programming paradigm for most of its parts.  The header files can
 directly be used from C++, too.

 Gwenhywfar includes the following features:
 <ul>
   <li>
     Basic Data types for binary buffers, linked lists, error objects,
     string lists (src/base/), buffered IO operations (src/io/)
   </li>
   <li>
     OS abstraction functions for directory and Internet address handling
     (src/os/)
   </li>
   <li>
     extensive data type and function collections for Inter-Process
     communication (IPC, in src/ipc/ and src/impl/ipc/)
   </li>
   <li>
     High-level cryptographic functions on top of OpenSSL functionality
     (src/crypt/)
   </li>
   <li>
     High-level functions for parsing XML-like files and accessing them
     like a hierarchical database (src/parser/)
   </li>
 </ul>

 @section GWEN_contact Contact

 For questions about this library please ask on the mailing list of the
 OpenHBCI project openhbci-general (subscriber-only, please subscribe
 on
 <a href="http://lists.sourceforge.net/lists/listinfo/openhbci-general">
 http://lists.sourceforge.net/lists/listinfo/openhbci-general</a>),
 since this is where the author(s) hang out.


 @section GWEN_author Authors

 This library is being written by and is being maintained by
 <a href="mailto:Martin Preuss <martin@libchipcard.de>">Martin Preuss</a>
 Some minor work has been contributed by others including
 <a href="mailto:Christian Stimming <stimming@tuhh.de>">Christian Stimming</a>.
 Some functions in <i>src/impl/msgengine/seccontext.c</i> have originally been
 written by Fabian Kaiser in C++ for the project OpenHBCI.
 I simply transformed them to plain C.

 */


#endif

