
#include "lf_storage_l.h"
#include <gwenhywfar/logger.h>
#include <gwenhywfar/plugin.h>
#include <gwenhywfar/st_storage.h>

#define MANY_OBJECTS 5000

static GWEN_TYPE_UINT32 objectId=0;


GWEN_STO_STORAGE *createStorage() {
  return LocalFilesStorage_new("testfolder");
}



int test_register_client() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  int rv;

  fprintf(stderr, "================ TEST: Register client... ");

  st=createStorage();
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "PASSED.\n");
  return 0;
}



int test_create_storage() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  int rv;

  fprintf(stderr, "================ TEST: Create storage...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - create storage\n");
  rv=GWEN_StoClient_Create(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_open_storage() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  int rv;

  fprintf(stderr, "================ TEST: Open storage...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_begin_edit() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  int rv;

  fprintf(stderr, "================ TEST: Begin edit...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_create_type() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  int rv;

  fprintf(stderr, "================ TEST: Create type...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - create type\n");
  rv=GWEN_StoClient_CreateType(cl, "test_t", "test", &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create type (%d).\n", rv);
    return 2;
  }


  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_open_type() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  int rv;

  fprintf(stderr, "================ TEST: Open type...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_add_var() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_VARDEF *vdef;
  int rv;

  fprintf(stderr, "================ TEST: Add variable...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl,
			 GWEN_STO_OPENFLAGS_RD |
			 GWEN_STO_OPENFLAGS_WR);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD |
			     GWEN_STO_OPENFLAGS_WR,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - create variable definition\n");
  vdef=GWEN_StoVarDef_new();
  GWEN_StoVarDef_SetName(vdef, "testVar");
  GWEN_StoVarDef_SetMinNum(vdef, 1);
  GWEN_StoVarDef_SetMaxNum(vdef, 10);

  fprintf(stderr, "  - add variable definition\n");
  rv=GWEN_StoType_AddVar(ty, vdef);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }
  GWEN_StoVarDef_free(vdef);

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_create_object() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_OBJECT *o;
  int rv;

  fprintf(stderr, "================ TEST: Create object...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - create object\n");
  rv=GWEN_StoClient_CreateObject(cl, ty, &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create object (%d).\n", rv);
    return 2;
  }

  objectId=GWEN_StoObject_GetId(o);
  fprintf(stderr, "  - object id is: %x\n", objectId);

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not create object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_open_object() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_OBJECT *o;
  int rv;

  fprintf(stderr, "================ TEST: Open object...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open object\n");
  rv=GWEN_StoClient_OpenObject(cl, ty,
                               objectId,
			       GWEN_STO_OPENFLAGS_RD,
			       &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_set_value() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_OBJECT *o;
  int rv;

  fprintf(stderr, "================ TEST: Set value...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD | GWEN_STO_OPENFLAGS_WR);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open object\n");
  rv=GWEN_StoClient_OpenObject(cl, ty,
                               objectId,
			       GWEN_STO_OPENFLAGS_RD|
			       GWEN_STO_OPENFLAGS_WR,
			       &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open object (%d).\n", rv);
    return 2;
  }

  GWEN_StoObject_SetCharValue(o,
			      "testVar",
			      "testValue1",
                              1);
  GWEN_StoObject_SetCharValue(o,
			      "testVar",
			      "testValue2",
			      0);

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_set_value2() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_OBJECT *o;
  int rv;

  fprintf(stderr, "================ TEST: Set value (2)...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD | GWEN_STO_OPENFLAGS_WR);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open object\n");
  rv=GWEN_StoClient_OpenObject(cl, ty,
                               objectId,
			       GWEN_STO_OPENFLAGS_RD|
			       GWEN_STO_OPENFLAGS_WR,
			       &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open object (%d).\n", rv);
    return 2;
  }

  GWEN_StoObject_SetCharValue(o,
			      "testVar",
			      "testValue3",
			      0);

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_set_value3() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_OBJECT *o;
  int rv;
  const char *s;

  fprintf(stderr, "================ TEST: Set value (3)...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD | GWEN_STO_OPENFLAGS_WR);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open object\n");
  rv=GWEN_StoClient_OpenObject(cl, ty,
                               objectId,
			       GWEN_STO_OPENFLAGS_RD|
			       GWEN_STO_OPENFLAGS_WR,
			       &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open object (%d).\n", rv);
    return 2;
  }

  GWEN_StoObject_SetCharValue(o,
			      "testVar",
			      "testValue4",
			      0);

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_NoUpdate);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - reopen object\n");
  rv=GWEN_StoClient_OpenObject(cl, ty,
                               objectId,
			       GWEN_STO_OPENFLAGS_RD,
			       &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open object (%d).\n", rv);
    return 2;
  }

  s=GWEN_StoObject_GetCharValue(o, "testVar", 3, 0);
  if (s) {
    fprintf(stderr,
	    "FAILED: Value received but not expected.\n");
    return 2;
  }

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close object (%d).\n", rv);
    return 2;
  }


  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_get_value() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_OBJECT *o;
  const char *s;
  int rv;

  fprintf(stderr, "================ TEST: Get value...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open object\n");
  rv=GWEN_StoClient_OpenObject(cl, ty,
                               objectId,
			       GWEN_STO_OPENFLAGS_RD,
			       &o);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - check value 1\n");
  s=GWEN_StoObject_GetCharValue(o, "testVar", 0, 0);
  if (strcmp(s, "testValue1")!=0) {
    fprintf(stderr, "FAILED: Value received does not equal value set.\n");
    return 2;
  }

  fprintf(stderr, "  - check value 2\n");
  s=GWEN_StoObject_GetCharValue(o, "testVar", 1, 0);
  if (strcmp(s, "testValue2")!=0) {
    fprintf(stderr, "FAILED: Value received does not equal value set.\n");
    return 2;
  }

  fprintf(stderr, "  - check value 3\n");
  s=GWEN_StoObject_GetCharValue(o, "testVar", 2, 0);
  if (strcmp(s, "testValue3")!=0) {
    fprintf(stderr, "FAILED: Value received does not equal value set.\n");
    return 2;
  }

  fprintf(stderr, "  - close object\n");
  rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close object (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_list_objects() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_FIND *fnd;
  GWEN_TYPE_UINT32 id;
  int rv;

  fprintf(stderr, "================ TEST: List objects...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - get object ids\n");
  rv=GWEN_StoClient_FindFirstObject(cl, ty, &fnd, &id);
  fprintf(stderr, "      Objects:");
  while(rv==0) {
    fprintf(stderr, " %x", id);
    rv=GWEN_StoClient_FindNextObject(cl, ty, fnd, &id);
  }
  fprintf(stderr, "\n");
  if (rv!=GWEN_ERROR_NOT_FOUND) {
    fprintf(stderr, "FAILED: Could not list objects (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close find handle\n");
  rv=GWEN_StoClient_CloseFind(cl, ty, fnd);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close find handle (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_create_many_objects() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  int i;
  int rv;

  fprintf(stderr, "================ TEST: Create many objects...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - begin edit\n");
  rv=GWEN_StoClient_BeginEdit(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not begin editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - create objects\n");
  for (i=0; i<MANY_OBJECTS; i++) {
    GWEN_STO_OBJECT *o;
    char nbuf[32];

    fprintf(stderr, "\r %u", i);
    rv=GWEN_StoClient_CreateObject(cl, ty, &o);
    if (rv) {
      fprintf(stderr, "\nFAILED: Could not create object (%d).\n", rv);
      return 2;
    }

    snprintf(nbuf, sizeof(nbuf), "testVar%08x", i);
    GWEN_StoObject_SetCharValue(o, nbuf,
				"testValue1",
				1);
    GWEN_StoObject_SetCharValue(o, nbuf,
				"testValue2",
				0);

    rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
    if (rv) {
      fprintf(stderr, "\nFAILED: Could not close object (%d).\n", rv);
      return 2;
    }
  }
  fprintf(stderr, "\n");

  fprintf(stderr, "  - end edit\n");
  rv=GWEN_StoClient_EndEdit(cl, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not end editing (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int test_open_many_objects() {
  GWEN_STO_STORAGE *st;
  GWEN_STO_CLIENT *cl;
  GWEN_STO_TYPE *ty;
  GWEN_STO_FIND *fnd;
  GWEN_TYPE_UINT32 id;
  int rv;

  fprintf(stderr, "================ TEST: Open many objects...\n");

  st=createStorage();
  fprintf(stderr, "  - register client\n");
  rv=GWEN_StoStorage_RegisterClient(st, "testuser", &cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not register client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open storage\n");
  rv=GWEN_StoClient_Open(cl, GWEN_STO_OPENFLAGS_RD);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open type\n");
  rv=GWEN_StoClient_OpenType(cl, "test_t", "test",
			     GWEN_STO_OPENFLAGS_RD,
			     &ty);
  if (rv) {
    fprintf(stderr, "FAILED: Could not open type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - open objects\n");
  rv=GWEN_StoClient_FindFirstObject(cl, ty, &fnd, &id);
  while(rv==0) {
    GWEN_STO_OBJECT *o;

    fprintf(stderr, "\r %u", id);
    rv=GWEN_StoClient_OpenObject(cl, ty,
                                 id,
                                 GWEN_STO_OPENFLAGS_RD,
                                 &o);
    if (rv) {
      fprintf(stderr, "\nFAILED: Could not open object (%d).\n", rv);
      return 2;
    }

    rv=GWEN_StoClient_CloseObject(cl, ty, o, GWEN_StoCloseMode_Normal);
    if (rv) {
      fprintf(stderr, "\nFAILED: Could not close object (%d).\n", rv);
      return 2;
    }

    rv=GWEN_StoClient_FindNextObject(cl, ty, fnd, &id);
  }
  fprintf(stderr, "\n");
  if (rv!=GWEN_ERROR_NOT_FOUND) {
    fprintf(stderr, "FAILED: Could not list objects (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close find handle\n");
  rv=GWEN_StoClient_CloseFind(cl, ty, fnd);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close find handle (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close type\n");
  rv=GWEN_StoClient_CloseType(cl, ty, GWEN_StoCloseMode_Normal);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close type (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - close storage\n");
  rv=GWEN_StoClient_Close(cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not close storage (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  - unregister client\n");
  rv=GWEN_StoStorage_UnregisterClient(st, cl);
  if (rv) {
    fprintf(stderr, "FAILED: Could not unregister client (%d).\n", rv);
    return 2;
  }

  fprintf(stderr, "  PASSED.\n");
  return 0;
}



int main(int argc, char **argv) {
  int rv;
  int okCount=0;
  int errorCount=0;

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);
#define CHECK_RV(rv) \
  if (rv) { \
    errorCount++; \
    fprintf(stderr, "(FAILED: %d)\n", rv); \
  } \
  else { \
    okCount++; \
  }

  rv=test_register_client();
  CHECK_RV(rv);
  rv=test_create_storage();
  CHECK_RV(rv);
  rv=test_open_storage();
  CHECK_RV(rv);
  rv=test_begin_edit();
  CHECK_RV(rv);
  rv=test_create_type();
  CHECK_RV(rv);
  rv=test_open_storage();
  CHECK_RV(rv);
  rv=test_open_type();
  CHECK_RV(rv);
  rv=test_add_var();
  CHECK_RV(rv);
  rv=test_create_object();
  CHECK_RV(rv);
  rv=test_open_object();
  CHECK_RV(rv);
  rv=test_set_value();
  CHECK_RV(rv);
  rv=test_set_value2();
  CHECK_RV(rv);
  rv=test_get_value();
  CHECK_RV(rv);
  rv=test_list_objects();
  CHECK_RV(rv);
  rv=test_set_value3();
  CHECK_RV(rv);
  rv=test_create_many_objects();
  CHECK_RV(rv);
  rv=test_open_many_objects();
  CHECK_RV(rv);

#undef CHECK_RV

  fprintf(stderr, "\n===============================================\n");
  if (errorCount) {
    fprintf(stderr, "%d tests failed, %d tests passed\n",
            errorCount, okCount);
    return 2;
  }
  else
    fprintf(stderr, "OVERALL RESULT: All tests passed\n");
  return 0;
}

