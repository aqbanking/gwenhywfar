

#include "typemaker2.h"

#include <gwenhywfar/args.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/gwenhywfar.h>


#define I18N(msg) msg




int main(int argc, char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  int err;
  GWEN_GUI *gui;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "api",                        /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "D",                          /* short option */
    "api",                        /* long option */
    "API declaration prefix (like GWENHYWFAR_API)",
    "API declaration prefix (like GWENHYWFAR_API)"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "publicFile",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "public",                     /* long option */
    "Name of the public header file to create",
    "Name of the public header file to create"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "libraryFile",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "library",                    /* long option */
    "Name of the library header file to create",
    "Name of the library header file to create"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "protectedFile",              /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "protected",                  /* long option */
    "Name of the protected header file to create",
    "Name of the protected header file to create"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "privateFile",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "private",                    /* long option */
    "Name of the private header file to create",
    "Name of the private header file to create"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "codeFile",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "code",                       /* long option */
    "Name of the code file to create",
    "Name of the code file to create"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "typedefFile",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "typedef",                    /* long option */
    "Name of the typedef file to create",
    "Name of the typedef file to create"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "language",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "lang",                       /* long option */
    "Language for which to create the headers and code",
    "Language for which to create the headers and code"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "include",                    /* name */
    0,                            /* minnum */
    99,                           /* maxnum */
    "I",                          /* short option */
    "include",                    /* long option */
    "Add folder to include for type lookup",
    "Add folder to include for type lookup"
  },

  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  err=GWEN_Init();
  if (err) {
    fprintf(stderr, "Could not initialize Gwenhywfar.\n");
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_Open(0, "typemaker2", 0,
		   GWEN_LoggerType_Console,
		   GWEN_LoggerFacility_User);

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
		     GWEN_ARGS_MODE_ALLOW_FREEPARAM,
		     args,
		     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    return -1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
                             I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf,
                             I18N(" [GLOBAL OPTIONS] COMMAND "
                                  "[LOCAL OPTIONS]\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nGlobal Options:\n"));
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  build:\n"
                                  "    This command creates source and header files for the given file"
                                  "\n\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  rv=build(db);

  err=GWEN_Fini();
  if (err) {
    fprintf(stderr,
            "WARNING: Could not deinitialize Gwenhywfar.\n");
  }

  return rv;
}

