

#include <gwenhywfar/db.h>
#include <gwenhywfar/directory.h>

#include <stdio.h>
#include <string.h>



int check_db1() {
  GWEN_DB_NODE *cfg;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db1: Could not create group.\n");
    return 2;
  }

  GWEN_DB_Group_free(cfg);
  return 0;
}



int check_db2() {
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *db1;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group.\n");
    return 2;
  }

  db1=GWEN_DB_GetGroup(cfg, 0, "test1");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1].\n");
    return 2;
  }

  GWEN_DB_Group_free(cfg);
  return 0;
}



int check_db3() {
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *db1;
  GWEN_DB_NODE *db2;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db3: Could not create group.\n");
    return 2;
  }

  db1=GWEN_DB_GetGroup(cfg, 0, "test1");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db3: Could not create group [test1].\n");
    return 2;
  }

  db2=GWEN_DB_GetGroup(cfg, 0, "test1/test11");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db3: Could not create group [test1/test11].\n");
    return 2;
  }

  GWEN_DB_Group_free(cfg);
  return 0;
}



int check_db4() {
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *db1;
  GWEN_DB_NODE *db2;
  const char *v1;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db4: Could not create group.\n");
    return 2;
  }

  db1=GWEN_DB_GetGroup(cfg, 0, "test1");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1].\n");
    return 2;
  }

  db2=GWEN_DB_GetGroup(cfg, 0, "test1/test11");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1/test11].\n");
    return 2;
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1111");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db4: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db4: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }

  GWEN_DB_Group_free(cfg);
  return 0;
}



int check_db5() {
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *db1;
  GWEN_DB_NODE *db2;
  const char *v1;
  const char *v2;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db5: Could not create group.\n");
    return 2;
  }

  db1=GWEN_DB_GetGroup(cfg, 0, "test1");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1].\n");
    return 2;
  }

  db2=GWEN_DB_GetGroup(cfg, 0, "test1/test11");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1/test11].\n");
    return 2;
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1111");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db5: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db5: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }


  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1112");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db5: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db5: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }

  v2=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 1, NULL);
  if (v2==NULL) {
    fprintf(stderr,
	    "ERROR in db5: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v2, "value1112")!=0) {
    fprintf(stderr,
	    "ERROR in db5: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v2, "value1112");
    return 2;
  }


  GWEN_DB_Group_free(cfg);
  return 0;
}



int check_db6() {
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *db1;
  GWEN_DB_NODE *db2;
  const char *v1;
  const char *v2;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db6: Could not create group.\n");
    return 2;
  }

  db1=GWEN_DB_GetGroup(cfg, 0, "test1");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1].\n");
    return 2;
  }

  db2=GWEN_DB_GetGroup(cfg, 0, "test1/test11");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1/test11].\n");
    return 2;
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1111");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db6: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db6: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }


  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1112");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db6: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db6: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }

  v2=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 1, NULL);
  if (v2==NULL) {
    fprintf(stderr,
	    "ERROR in db6: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v2, "value1112")!=0) {
    fprintf(stderr,
	    "ERROR in db6: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v2, "value1112");
    return 2;
  }


  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "test1/test11/var111", "value1113");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db6: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1113")!=0) {
    fprintf(stderr,
	    "ERROR in db6: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1113");
    return 2;
  }

  v2=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 1, NULL);
  if (v2!=NULL) {
    fprintf(stderr,
	    "ERROR in db6: Var [test1/test11/var111] should not exist.\n");
    return 2;
  }


  GWEN_DB_Group_free(cfg);
  return 0;
}



int check_db7() {
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *db1;
  GWEN_DB_NODE *db2;
  const char *v1;
  const char *v2;
  const char *v3;
  int rv;

  cfg=GWEN_DB_Group_new("config");
  if (cfg==NULL) {
    fprintf(stderr, "ERROR in db7: Could not create group.\n");
    return 2;
  }

  db1=GWEN_DB_GetGroup(cfg, 0, "test1");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1].\n");
    return 2;
  }

  db2=GWEN_DB_GetGroup(cfg, 0, "test1/test11");
  if (db1==NULL) {
    fprintf(stderr, "ERROR in db2: Could not create group [test1/test11].\n");
    return 2;
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1111");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db7: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db7: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }


  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "test1/test11/var111", "value1112");
  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db7: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db7: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }

  v2=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 1, NULL);
  if (v2==NULL) {
    fprintf(stderr,
	    "ERROR in db7: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v2, "value1112")!=0) {
    fprintf(stderr,
	    "ERROR in db7: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v2, "value1112");
    return 2;
  }

  {
#ifndef MAX_PATH
# define MAX_PATH 200
#endif
    char tmpfile[MAX_PATH];
    GWEN_Directory_GetTmpDirectory(tmpfile, MAX_PATH);
    strncat(tmpfile, 
#ifdef OS_WIN32
	    "\\"
#else
	    "/"
#endif
	    "db1.conf", MAX_PATH - strlen(tmpfile));

    rv=GWEN_DB_WriteFile(cfg, tmpfile, GWEN_DB_FLAGS_DEFAULT, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in db7: Could not write DB file (%d)\n", rv);
    return 2;
  }

  GWEN_DB_ClearGroup(cfg, NULL);

  rv=GWEN_DB_ReadFile(cfg, tmpfile, GWEN_DB_FLAGS_DEFAULT, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in db7: Could not read DB file (%d)\n", rv);
    return 2;
  }
  }

  v1=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 0, NULL);
  if (v1==NULL) {
    fprintf(stderr,
	    "ERROR in db7: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v1, "value1111")!=0) {
    fprintf(stderr,
	    "ERROR in db7: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v1, "value1111");
    return 2;
  }

  v2=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 1, NULL);
  if (v2==NULL) {
    fprintf(stderr,
	    "ERROR in db7: Could not get var [test1/test11/var111].\n");
    return 2;
  }

  if (strcmp(v2, "value1112")!=0) {
    fprintf(stderr,
	    "ERROR in db7: Value for var [test1/test11/var111] does not "
	    "match [%s]!=[%s].\n", v2, "value1112");
    return 2;
  }

  v3=GWEN_DB_GetCharValue(cfg, "test1/test11/var111", 2, NULL);
  if (v3!=NULL) {
    fprintf(stderr,
	    "ERROR in db7: Var [test1/test11/var111:2] not empty.\n");
    return 2;
  }

  GWEN_DB_Group_free(cfg);
  return 0;
}





int check_db() {
  int errs=0;
  int chks=0;

  fprintf(stderr, "DB-checks... ");
  chks++;
  if (check_db1())
    errs++;

  chks++;
  if (check_db2())
    errs++;

  chks++;
  if (check_db3())
    errs++;

  chks++;
  if (check_db4())
    errs++;

  chks++;
  if (check_db5())
    errs++;

  chks++;
  if (check_db6())
    errs++;

  chks++;
  if (check_db7())
    errs++;

  if (chks) {
    if (chks==errs) {
      fprintf(stderr, "DB-checks: all failed.\n");
    }
    else if (errs) {
      fprintf(stderr, "DB-checks: some failed.\n");
    }
    else {
      fprintf(stderr, "passed.\n");
    }
  }

  return errs;
}








