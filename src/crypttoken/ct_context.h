/* This file is auto-generated from "ct_context.xml" by the typemaker
   tool of Gwenhywfar. 
   Do not edit this file -- all changes will be lost! */
#ifndef CT_CONTEXT_H
#define CT_CONTEXT_H

/** @page P_GWEN_CRYPT_TOKEN_CONTEXT_PUBLIC GWEN_Crypt_Token_Context (public)
This page describes the properties of GWEN_CRYPT_TOKEN_CONTEXT
@anchor GWEN_CRYPT_TOKEN_CONTEXT_Id
<h3>Id</h3>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetId, 
get it with @ref GWEN_Crypt_Token_Context_GetId
</p>

<h3>Key Ids</h3>
<p>
This group contains the ids of the keys for multiple given purposes which are to be used in this context.</p>
@anchor GWEN_CRYPT_TOKEN_CONTEXT_SignKeyId
<h4>SignKeyId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetSignKeyId, 
get it with @ref GWEN_Crypt_Token_Context_GetSignKeyId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_VerifyKeyId
<h4>VerifyKeyId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetVerifyKeyId, 
get it with @ref GWEN_Crypt_Token_Context_GetVerifyKeyId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_EncipherKeyId
<h4>EncipherKeyId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetEncipherKeyId, 
get it with @ref GWEN_Crypt_Token_Context_GetEncipherKeyId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_DecipherKeyId
<h4>DecipherKeyId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetDecipherKeyId, 
get it with @ref GWEN_Crypt_Token_Context_GetDecipherKeyId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_AuthSignKeyId
<h4>AuthSignKeyId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetAuthSignKeyId, 
get it with @ref GWEN_Crypt_Token_Context_GetAuthSignKeyId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_AuthVerifyKeyId
<h4>AuthVerifyKeyId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetAuthVerifyKeyId, 
get it with @ref GWEN_Crypt_Token_Context_GetAuthVerifyKeyId
</p>

<h3>User Information</h3>
<p>
This group contains some information about this context. With some crypt token user information might be available with a context.</p>
@anchor GWEN_CRYPT_TOKEN_CONTEXT_ServiceId
<h4>ServiceId</h4>
<p>
For Homebanking this is the bank code</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetServiceId, 
get it with @ref GWEN_Crypt_Token_Context_GetServiceId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_UserId
<h4>UserId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetUserId, 
get it with @ref GWEN_Crypt_Token_Context_GetUserId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_UserName
<h4>UserName</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetUserName, 
get it with @ref GWEN_Crypt_Token_Context_GetUserName
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_PeerId
<h4>PeerId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetPeerId, 
get it with @ref GWEN_Crypt_Token_Context_GetPeerId
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_PeerName
<h4>PeerName</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetPeerName, 
get it with @ref GWEN_Crypt_Token_Context_GetPeerName
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_Address
<h4>Address</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetAddress, 
get it with @ref GWEN_Crypt_Token_Context_GetAddress
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_Port
<h4>Port</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetPort, 
get it with @ref GWEN_Crypt_Token_Context_GetPort
</p>

@anchor GWEN_CRYPT_TOKEN_CONTEXT_SystemId
<h4>SystemId</h4>
<p>
</p>
<p>
Set this property with @ref GWEN_Crypt_Token_Context_SetSystemId, 
get it with @ref GWEN_Crypt_Token_Context_GetSystemId
</p>

*/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct GWEN_CRYPT_TOKEN_CONTEXT GWEN_CRYPT_TOKEN_CONTEXT;

#ifdef __cplusplus
} /* __cplusplus */
#endif

#include <gwenhywfar/db.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
/* headers */
#include <gwenhywfar/types.h>

#ifdef __cplusplus
extern "C" {
#endif


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_CRYPT_TOKEN_CONTEXT, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_Crypt_Token_Context, GWENHYWFAR_API)
GWENHYWFAR_API GWEN_CRYPT_TOKEN_CONTEXT_LIST *GWEN_Crypt_Token_Context_List_dup(const GWEN_CRYPT_TOKEN_CONTEXT_LIST *stl);

GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_CRYPT_TOKEN_CONTEXT, GWEN_Crypt_Token_Context, GWENHYWFAR_API)

/** Destroys all objects stored in the given LIST2 and the list itself
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_List2_freeAll(GWEN_CRYPT_TOKEN_CONTEXT_LIST2 *stl);

/** Creates a new object.
*/
GWENHYWFAR_API GWEN_CRYPT_TOKEN_CONTEXT *GWEN_Crypt_Token_Context_new();
/** Creates an object from the data in the given GWEN_DB_NODE
*/
GWENHYWFAR_API GWEN_CRYPT_TOKEN_CONTEXT *GWEN_Crypt_Token_Context_fromDb(GWEN_DB_NODE *db);
/** Creates and returns a deep copy of thegiven object.
*/
GWENHYWFAR_API GWEN_CRYPT_TOKEN_CONTEXT *GWEN_Crypt_Token_Context_dup(const GWEN_CRYPT_TOKEN_CONTEXT*st);
/** Destroys the given object.
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_free(GWEN_CRYPT_TOKEN_CONTEXT *st);
/** Increments the usage counter of the given object, so an additional free() is needed to destroy the object.
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_Attach(GWEN_CRYPT_TOKEN_CONTEXT *st);
/** Reads data from a GWEN_DB.
*/
GWENHYWFAR_API int GWEN_Crypt_Token_Context_ReadDb(GWEN_CRYPT_TOKEN_CONTEXT *st, GWEN_DB_NODE *db);
/** Stores an object in the given GWEN_DB_NODE
*/
GWENHYWFAR_API int GWEN_Crypt_Token_Context_toDb(const GWEN_CRYPT_TOKEN_CONTEXT*st, GWEN_DB_NODE *db);
/** Returns 0 if this object has not been modified, !=0 otherwise
*/
GWENHYWFAR_API int GWEN_Crypt_Token_Context_IsModified(const GWEN_CRYPT_TOKEN_CONTEXT *st);
/** Sets the modified state of the given object
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetModified(GWEN_CRYPT_TOKEN_CONTEXT *st, int i);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_Id
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_Id
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/** @name Key Ids
 *
This group contains the ids of the keys for multiple given purposes which are to be used in this context.*/
/*@{*/

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_SignKeyId
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetSignKeyId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_SignKeyId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetSignKeyId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_VerifyKeyId
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetVerifyKeyId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_VerifyKeyId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetVerifyKeyId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_EncipherKeyId
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetEncipherKeyId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_EncipherKeyId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetEncipherKeyId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_DecipherKeyId
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetDecipherKeyId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_DecipherKeyId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetDecipherKeyId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_AuthSignKeyId
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetAuthSignKeyId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_AuthSignKeyId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetAuthSignKeyId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_AuthVerifyKeyId
*/
GWENHYWFAR_API uint32_t GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_AuthVerifyKeyId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetAuthVerifyKeyId(GWEN_CRYPT_TOKEN_CONTEXT *el, uint32_t d);

/*@}*/

/** @name User Information
 *
This group contains some information about this context. With some crypt token user information might be available with a context.*/
/*@{*/

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_ServiceId
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetServiceId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_ServiceId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetServiceId(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_UserId
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetUserId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_UserId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetUserId(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_UserName
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetUserName(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_UserName
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetUserName(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_PeerId
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetPeerId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_PeerId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetPeerId(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_PeerName
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetPeerName(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_PeerName
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetPeerName(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_Address
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetAddress(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_Address
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetAddress(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_Port
*/
GWENHYWFAR_API int GWEN_Crypt_Token_Context_GetPort(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_Port
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetPort(GWEN_CRYPT_TOKEN_CONTEXT *el, int d);

/**
* Returns the property @ref GWEN_CRYPT_TOKEN_CONTEXT_SystemId
*/
GWENHYWFAR_API const char *GWEN_Crypt_Token_Context_GetSystemId(const GWEN_CRYPT_TOKEN_CONTEXT *el);
/**
* Set the property @ref GWEN_CRYPT_TOKEN_CONTEXT_SystemId
*/
GWENHYWFAR_API void GWEN_Crypt_Token_Context_SetSystemId(GWEN_CRYPT_TOKEN_CONTEXT *el, const char *d);

/*@}*/


#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* CT_CONTEXT_H */