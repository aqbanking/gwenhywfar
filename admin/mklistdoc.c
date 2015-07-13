/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Jun 02 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include <ctype.h>


#define MAX_LINESIZE 8192


typedef struct MACRO_DEF MACRO_DEF;
struct MACRO_DEF {
  char *macroName;
  char *tmplFileName;
  char *fileSuffix;
};


typedef struct MACRO_STRUCT MACRO_STRUCT;
struct MACRO_STRUCT {
  MACRO_STRUCT *next;
  MACRO_DEF *macroDef;
  char *typeName;
  char *typePrefix;
};



MACRO_DEF macroDefs[]= {
{
  "GWEN_LIST2_FUNCTION_DEFS",
  "list2.tmpl",
  "_list2p"
},
{
  "GWEN_LIST2_FUNCTION_LIB_DEFS",
  "list2.tmpl",
  "_list2"
},
{
  "GWEN_CONSTLIST2_FUNCTION_DEFS",
  "list2.tmpl",
  "_constlist2p"
},
{
  "GWEN_CONSTLIST2_FUNCTION_LIB_DEFS",
  "list2.tmpl",
  "_constlist2"
},
{
  "GWEN_LIST_FUNCTION_DEFS",
  "list1.tmpl",
  "_listp"
},
{
  "GWEN_LIST_FUNCTION_LIB_DEFS",
  "list1.tmpl",
  "_list"
},
{
  "GWEN_CONSTLIST_FUNCTION_DEFS",
  "list1.tmpl",
  "_constlistp"
},
{
  "GWEN_CONSTLIST_FUNCTION_LIB_DEFS",
  "list1.tmpl",
  "_constlist"
},
{
  0, 0, 0
}
};



#define MODE_CHECKONLY   1
#define MODE_WRITEHEADER 2
#define MODE_WRITESTDOUT 3


const char *templatePath=GWEN_HEADERS;
int mode=MODE_WRITESTDOUT;
int verbosity=0;



MACRO_STRUCT *MacroStruct_new(MACRO_DEF *macroDef,
                              const char *typeName,
                              const char *typePrefix) {
  MACRO_STRUCT *m;

  m=(MACRO_STRUCT*)malloc(sizeof(MACRO_STRUCT));
  memset(m, 0, sizeof(MACRO_STRUCT));
  m->macroDef=macroDef;
  if (typeName)
    m->typeName=strdup(typeName);
  if (typePrefix)
    m->typePrefix=strdup(typePrefix);

  return m;
}



void MacroStruct_free(MACRO_STRUCT *m) {
  if (m) {
    free(m->typeName);
    free(m->typePrefix);
    free(m);
  }
}




int scanLine(MACRO_DEF *macroDef,
             char *buffer,
             MACRO_STRUCT **m) {
  char *p;
  char *macroBegin;

  p=strstr(buffer, macroDef->macroName);
  if (p) {
    /* found something, search for opening bracket */
    macroBegin=p;
    p+=strlen(macroDef->macroName);
    while(*p && isspace((int)*p)) p++;
    if (*p && *p!='(')
      return -1;

    while(*p) {
      if (*p=='(') {
        char *typeName;
        char *typeNameEnd;
        char *typePrefix;
        char *typePrefixEnd;

        typeName=typePrefix=0;
        typeNameEnd=typePrefixEnd=0;

        /* found it, now read the typename */
        p++;
        while(*p && isspace((int)*p)) p++;
        typeName=p;

        /* find comma */
        while(*p) {
          if (*p==',') {
            /* found it, name ends here */
            if (!typeNameEnd)
              typeNameEnd=p;
            p++;
            while(*p && isspace((int)*p)) p++;
            typePrefix=p;

            /* find closing bracket */
            while(*p) {
              if (*p==')' || *p==',') {
                MACRO_STRUCT *lm;
                MACRO_STRUCT *sm;
                char *s;
                char c1, c2;

                /* found it, now all is complete */
                if (!typePrefixEnd)
                  typePrefixEnd=p;

                /* check whether this is a definition */
                s=buffer;
                while(*s && isspace((int)*s)) s++;
                if (*s=='#') {
                  s++;
                  /* preprocessor command, check for define */
                  while(*s && isspace((int)*s)) s++;
                  if (strncasecmp(s, "define ", 7)==0) {
                    s+=7;
                    /* it is a define, now check if the next nonblank is
                     * the beginning of this macro */
                    while(*s && isspace((int)*s)) s++;
                    if (s==macroBegin) {
                      if (verbosity>1)
                        fprintf(stderr, "Found definition for macro \"%s\".\n",
                                macroDef->macroName);
                      return -1;
                    }
                  }
                }

                c1=*typePrefixEnd;
                c2=*typeNameEnd;
                *typePrefixEnd=0;
                *typeNameEnd=0;
                sm=*m;
                while(sm) {
                  if (strcmp(sm->macroDef->macroName, macroDef->macroName)==0)
                    if (strcmp(sm->typeName, typeName)==0)
                      /* already exists */
                      break;
                  sm=sm->next;
                }
                if (!sm) {
                  /* macro is new, store it */
                  lm=MacroStruct_new(macroDef, typeName, typePrefix);
                  *typePrefixEnd=c1;
                  *typeNameEnd=c2;
                  if (*m==0)
                    *m=lm;
                  else {
                    lm->next=*m;
                    *m=lm;
                  }
                  if (verbosity>1)
                    fprintf(stderr, "Found macro \"%s\" (%s, %s)\n",
                            lm->macroDef->macroName,
                            lm->typeName,
                            lm->typePrefix);
                }
                return 0;
              }
              else if (isspace((int)*p)) {
                if (!typePrefixEnd)
                  typePrefixEnd=p;
              }
              else {
                if (typePrefixEnd)
                  return -1;
              }

              p++;
            }
            /* closing bracket missing, probably need more data... */
            return 1;
          }
          else if (isspace((int)*p)) {
            if (!typeNameEnd)
              typeNameEnd=p;
          }
          else {
            if (typeNameEnd)
              return -1;
          }
          p++;
        }
        /* comma missing, probably need more data... */
        return 1;
      }
      p++;
    }
    /* opening bracket missing, probably need more data... */
    return 1;
  }
  /* macro not found in this line */
  return -1;
}



int scanLineForAllMacros(char *buffer,
                         MACRO_STRUCT **m) {
  int i;


  for (i=0; ; i++) {
    if (macroDefs[i].macroName==0)
      return 0;
    if (scanLine(&macroDefs[i], buffer, m)==1)
      return 1;
  }

  return 0;
}



MACRO_STRUCT *scanForMacros(const char *fname) {
  FILE *f;
  MACRO_STRUCT *mst;
  char buffer[MAX_LINESIZE];
  int line;
  char *p;

  mst=0;
  line=0;
  if (verbosity)
    fprintf(stderr, "Reading file \"%s\"\n", fname);
  f=fopen(fname, "r");
  if (!f) {
    fprintf(stderr, "fopen(%s): %s\n", fname, strerror(errno));
    return 0;
  }

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  while(!feof(f)) {
    int sl;
    int rv;

    line++;
    if (verbosity>3)
      fprintf(stderr, "Reading line %d\n", line);

    /* read line */
    if (buffer[0]==0) {
      p=fgets(buffer, sizeof(buffer)-1, f);
    }
    else {
      int sizeLeft;

      if (verbosity>3)
        fprintf(stderr, "Multi-line macro (%d)\n", line);
      sl=strlen(buffer);
      sizeLeft=(sizeof(buffer)-1)-sl;
      if (sizeLeft<2) {
        fprintf(stderr, "Line %d: Line full, macro too long\n", line);
        fclose(f);
        return 0;
      }
      strcat(buffer, " ");
      p=fgets(buffer+sl+1, sizeLeft, f);
    }

    if (!p) {
      if (ferror(f)) {
        fprintf(stderr, "fread(%s): %s\n", fname, strerror(errno));
        fclose(f);
        return 0;
      }
      else
        /* simple eof */
        break;
    }

    sl=strlen(buffer);
    if (buffer[sl-1]=='\n')
      buffer[--sl]=0;

    if (buffer[sl-1]=='\\') {
      buffer[--sl]=0;
    }
    else {
      /* now check for macros */
      rv=scanLineForAllMacros(buffer, &mst);
      if (rv!=1)
        /* no more data needed */
        buffer[0]=0;
    }
  } /* while */

  fclose(f);
  return mst;
}



int transformF(FILE *inFile,
               FILE *outFile,
               const char *outFileName,
               MACRO_STRUCT *m) {
  char *p;
  char buffer[MAX_LINESIZE];
  int didIt;

  while(!feof(inFile)) {
    char *vname;
    int sl;

    /* read line */
    p=fgets(buffer, sizeof(buffer)-1, inFile);
    if (!p) {
      if (ferror(inFile)) {
        fprintf(stderr, "fread(): %s\n", strerror(errno));
        return 0;
      }
      else
        /* simple eof */
        break;
    }

    sl=strlen(buffer);
    if (buffer[sl-1]=='\n')
      buffer[--sl]=0;

    vname=0;
    p=buffer;
    while(*p) {
      while(*p && *p!='@') {
        if (EOF==fputc(*p, outFile)) {
          fprintf(stderr, "fputc(): %s (1)\n", strerror(errno));
          return 2;
        }
        p++;
      }
      didIt=0;
      if (*p=='@') {
        char *psave;
  
        /* got a var... */
        psave=p;
        p++;
        vname=p;
        while(*p && *p!='@') p++;
        if (*p=='@') {
          char c;
          const char *replacement;
  
          /* got it */
          c=*p;
          *p=0;
          if (strcmp(vname, "TYPENAME")==0)
            replacement=m->typeName;
          else if (strcmp(vname, "FN_PREFIX")==0)
            replacement=m->typePrefix;
          else if (strcmp(vname, "FILENAME")==0)
            replacement=outFileName;
          else {
            /* not a var */
            replacement=0;
          }
          *p=c;
          p++;
  
          if (replacement) {
            if (fprintf(outFile, "%s", replacement)<1) {
              fprintf(stderr, "fprintf(): %s\n", strerror(errno));
              return 2;
            }
            didIt=1;
          }
          else {
            /* write the original data */
            p=psave;
          }
        }
        else
          /* write the original data */
          p=psave;
      }
      if (!didIt) {
        if (*p) {
          if (EOF==fputc(*p, outFile)) {
            fprintf(stderr, "fputc(): %s\n", strerror(errno));
            return 2;
          }
          p++;
        }
      }
    } /* while line */
    fprintf(outFile, "\n");
  } /* while */

  return 0;
}



int transform(const char *inFile,
              const char *outFile,
              MACRO_STRUCT *m) {
  FILE *f1;
  FILE *f2;
  int rv;

  f1=fopen(inFile, "r");
  if (!f1) {
    fprintf(stderr, "fopen(%s, \"r\"): %s\n", inFile, strerror(errno));
    return 2;
  }
  if (outFile==0) {
    f2=stdout;
    rv=transformF(f1, f2, "(stdout)", m);
  }
  else {
    f2=fopen(outFile, "w+");
    if (!f2) {
      fprintf(stderr, "fopen(%s, \"w+\"): %s\n", outFile, strerror(errno));
      fclose(f1);
      return 2;
    }
    rv=transformF(f1, f2, outFile, m);
  }

  if (outFile!=0) {
    if (fclose(f2)) {
      fprintf(stderr, "fclose(%s): %s\n", outFile, strerror(errno));
      fclose(f1);
      return 2;
    }
  }
  if (fclose(f1)) {
    fprintf(stderr, "fclose(%s): %s\n", inFile, strerror(errno));
    return 2;
  }

  return rv;
}



int processFile(const char *fname) {
  MACRO_STRUCT *m;

  m=scanForMacros(fname);
  if (m) {
    /* do something with the macros */
    if (mode==MODE_WRITESTDOUT) {
      MACRO_STRUCT *cm;

      cm=m;
      while(cm) {
        int rv;
        char tmplBuffer[256];

        rv=snprintf(tmplBuffer, sizeof(tmplBuffer),
                    "%s/%s",
                    templatePath,
                    cm->macroDef->tmplFileName);
        if (rv<1 || rv>=sizeof(tmplBuffer)) {
          fprintf(stderr, "Internal error: buffer too small [processFile]\n");
          return 3;
        }
        rv=transform(tmplBuffer, 0, cm);
        if (rv) {
          fprintf(stderr, "Error transforming template file.\n");
          return rv;
        }

        cm=cm->next;
      }
    }
  }

  return 0;
}









int main(int argc, char **argv) {
  int i = 1;
  int wantHelp;

  wantHelp=0;
  if (argc<2) {
    wantHelp=1;
  }
  else {
    i=1;
    while(i<argc) {
      if (strcmp(argv[i], "-I")==0) {
        i++;
        if (i>=argc) {
          fprintf(stderr, "Argument missing for -I\n");
          return 1;
        }
        templatePath=argv[i];
      }
      else if (strcmp(argv[i], "-v")==0) {
        verbosity++;
      }
      else if (strcmp(argv[i], "-h")==0 ||
               strcmp(argv[i], "--help")==0) {
        wantHelp=1;
      }
      else if (strcmp(argv[i], "--doc")==0) {
        /* only generate header */
        mode=MODE_WRITEHEADER;
      }
      else if (strcmp(argv[i], "--stdout")==0) {
        /* write all headers to stdout instead of creating one header per
         * input */
        mode=MODE_WRITESTDOUT;
      }
      else
        break;
      i++;
    }
  }

  if (wantHelp) {
    fprintf(stderr,
            "MkListDoc (c) 2004 Martin Preuss<martin@libchipcard.de>\n"
            "Part of Gwenhywfar " GWENHYWFAR_VERSION_FULL_STRING "\n"
            "This tool is licensed under the LGPL (see COPYING in\n"
            "toplevel source folder).\n"
            "MkListDoc expands GWEN_LIST macros and creates a suitable\n"
            "header file for every encounterd list definition macro.\n"
            "Example:\n"
            "You are using GWEN_LIST2 macros to manage list of some objects:\n"
            " GWEN_LIST2_FUNCTION_DEFS(TYPE_SOMETHING, TypeSomething);\n"
            "The list is now available but not documented by Doxygen.\n"
            "This tool now creates output that can then be fed to doxygen\n"
            "to create appropriate API documentation.\n"
            "\n"
            "Usage: %s [OPTIONS] file1 file2 ... fileN\n"
            "Options are:\n"
            "-v     increases the verbosity level\n"
            "-I     path to GWEN include headers\n"
            "Output is sent to stdout.\n",
            argv[0]);
    return 0;
  }

  if (i>=argc) {
    /* no input ? */
    fprintf(stderr, "No input files.\n");
    return 1;
  }

  while(i<argc) {
    int rv;

    /* process all files */
    rv=processFile(argv[i]);
    if (rv) {
      fprintf(stderr, "Too bad...\n");
      return rv;
    }
    i++;
  } /* while */
  return 0;
}











