

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/padd.h>



int check1() {
  const char *testString="01234567890123456789";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  const char *p1, *p2;
  int i;
  int len;

  fprintf(stderr, "Check 1 ...");

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode((const unsigned char*)testString,
                        strlen(testString),
                        buf1, 0);
  if (rv) {
    fprintf(stderr, "FAILED: Could not encode.\n");
    return 2;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Decode((const unsigned char*)GWEN_Buffer_GetStart(buf1), 0,
                        buf2);
  if (rv) {
    fprintf(stderr, "FAILED: Could not decode.\n");
    return 2;
  }

  p1=testString;
  len=strlen(testString);
  p2=GWEN_Buffer_GetStart(buf2);
  if (GWEN_Buffer_GetUsedBytes(buf2)!=len) {
    fprintf(stderr, "Data differs in size\n");
    return 3;
  }
  rv=0;
  for (i=0; i<len; i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n%s\n", testString);
      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, stderr, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }

  fprintf(stderr, "PASSED.\n");

  return 0;
}



int _check2() {
  const char *testString="This is a little test string";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_CRYPTKEY *key;
  GWEN_CRYPTKEY *pubKey;
  GWEN_CRYPTKEY *privKey;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *dbPubKey;
  GWEN_DB_NODE *dbPrivKey;

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "FAILED: Could not create RSA key.\n");
    return 2;
  }

  err=GWEN_CryptKey_Generate(key, 1024);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not generate key.\n");
    return 2;
  }

  /* private key */
  dbPrivKey=GWEN_DB_Group_new("privkey");
  err=GWEN_CryptKey_toDb(key, dbPrivKey, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not extract private key.\n");
    return 2;
  }
  privKey=GWEN_CryptKey_fromDb(dbPrivKey);
  if (privKey==0) {
    fprintf(stderr, "FAILED: Could not create private key.\n");
    return 2;
  }

  /* public key */
  dbPubKey=GWEN_DB_Group_new("pubkey");
  err=GWEN_CryptKey_toDb(key, dbPubKey, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not extract public key.\n");
    return 2;
  }
  pubKey=GWEN_CryptKey_fromDb(dbPubKey);
  if (pubKey==0) {
    fprintf(stderr, "FAILED: Could not create public key.\n");
    return 2;
  }

  /* hash */
  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_MD_HashToBuffer("SHA1", testString, strlen(testString), buf1);
  if (rv) {
    fprintf(stderr, "FAILED: Could not hash data.\n");
    return 2;
  }

  /* padd */
  rv=GWEN_Padd_PaddWithPkcs1Bt1(buf1, 128);
  if (rv) {
    fprintf(stderr, "FAILED: Could not padd data.\n");
    return 2;
  }

  /* sign */
  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  err=GWEN_CryptKey_Sign(privKey, buf1, buf2);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not sign hash.\n");
    return 2;
  }

  /* hash */
  GWEN_Buffer_Reset(buf1);
  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_MD_HashToBuffer("SHA1", testString, strlen(testString), buf1);
  if (rv) {
    fprintf(stderr, "FAILED: Could not hash data.\n");
    return 2;
  }

  /* padd */
  rv=GWEN_Padd_PaddWithPkcs1Bt1(buf1, 128);
  if (rv) {
    fprintf(stderr, "FAILED: Could not padd data.\n");
    return 2;
  }

  /* verify */
  err=GWEN_CryptKey_Verify(pubKey, buf1, buf2);
  if (!GWEN_Error_IsOk(err)) {
    GWEN_DB_NODE *dbKeys;

    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not verify hash.\n");

    dbKeys=GWEN_DB_Group_new("keys");
    GWEN_DB_AddGroup(dbKeys, GWEN_DB_Group_dup(dbPrivKey));
    GWEN_DB_AddGroup(dbKeys, GWEN_DB_Group_dup(dbPubKey));
    GWEN_DB_SetBinValue(dbKeys, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "signature",
                        GWEN_Buffer_GetStart(buf2),
                        GWEN_Buffer_GetUsedBytes(buf2));
    GWEN_DB_SetBinValue(dbKeys, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "hash",
                        GWEN_Buffer_GetStart(buf1),
                        GWEN_Buffer_GetUsedBytes(buf1));
    GWEN_DB_WriteFile(dbKeys, "check2_failed_key",
                      GWEN_DB_FLAGS_DEFAULT);
    GWEN_DB_Group_free(dbKeys);
    return 2;
  }

  return 0;
}



int check2() {
  int i;

  fprintf(stderr, "Check 2 ...");
  for (i=0; i<500; i++) {
    int rv;

    fprintf(stderr, ".");
    rv=_check2();
    if (rv)
      return rv;
  }
  fprintf(stderr, "PASSED.\n");

  return 0;
}



int test1() {
  const char *testString="This is a little test string";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_CRYPTKEY *key;
  GWEN_CRYPTKEY *pubKey;
  GWEN_CRYPTKEY *privKey;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *dbKeys;
  GWEN_DB_NODE *dbPubKey;
  GWEN_DB_NODE *dbPrivKey;

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "FAILED: Could not create RSA key.\n");
    return 2;
  }

  err=GWEN_CryptKey_Generate(key, 1024);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not generate key.\n");
    return 2;
  }

  dbKeys=GWEN_DB_Group_new("keys");
  if (GWEN_DB_ReadFile(dbKeys, "check2_failed_key",
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    fprintf(stderr, "FAILED: Could not load file.\n");
    return 2;
  }

  /* private key */
  dbPrivKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			     "privkey");
  assert(dbPrivKey);
  privKey=GWEN_CryptKey_fromDb(dbPrivKey);
  assert(privKey);

  /* public key */
  dbPubKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			    "pubkey");
  assert(dbPubKey);
  pubKey=GWEN_CryptKey_fromDb(dbPubKey);
  assert(pubKey);

  /* hash */
  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_MD_HashToBuffer("SHA1", testString, strlen(testString), buf1);
  if (rv) {
    fprintf(stderr, "FAILED: Could not hash data.\n");
    return 2;
  }

  /* padd */
  rv=GWEN_Padd_PaddWithPkcs1Bt1(buf1, 128);
  if (rv) {
    fprintf(stderr, "FAILED: Could not padd data.\n");
    return 2;
  }

  /* sign */
  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  err=GWEN_CryptKey_Sign(privKey, buf1, buf2);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not sign hash.\n");
    return 2;
  }

  /* hash */
  GWEN_Buffer_Reset(buf1);
  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_MD_HashToBuffer("SHA1", testString, strlen(testString), buf1);
  if (rv) {
    fprintf(stderr, "FAILED: Could not hash data.\n");
    return 2;
  }

  /* padd */
  rv=GWEN_Padd_PaddWithPkcs1Bt1(buf1, 128);
  if (rv) {
    fprintf(stderr, "FAILED: Could not padd data.\n");
    return 2;
  }

  /* verify */
  err=GWEN_CryptKey_Verify(pubKey, buf1, buf2);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    fprintf(stderr, "FAILED: Could not verify hash.\n");
    return 2;
  }

  return 0;
}



int main(int argc, char **argv) {
  int rv;
  const char *cmd;

  if (argc>1)
    cmd=argv[1];
  else
    cmd="check";

  if (strcasecmp(cmd, "check")==0) {
    rv=check1();
    if (rv)
      return rv;
    rv=check2();
    if (rv)
      return rv;
  }
  else if (strcasecmp(cmd, "test1")==0) {
    rv=test1();
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", cmd);
    return 1;
  }
  return 0;
}


