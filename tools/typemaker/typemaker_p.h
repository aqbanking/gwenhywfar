

#ifndef TYPEMAKER_P_H
#define TYPEMAKER_P_H

#include <gwenhywfar/xml.h>
#include <gwenhywfar/bufferedio.h>


int write_hp_group_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                    GWEN_BUFFEREDIO *bio);

int write_hp_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);

int write_h_setget_c(ARGUMENTS *args,
                     GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio,
                     const char *acc);

int write_h_header(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_BUFFEREDIO *bio);
int write_h_elem_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_BUFFEREDIO *bio);
int write_h_struct_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio);
int write_hp_group_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio);

int write_hp_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_hp_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);

int write_hl_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_hl_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);


int write_ha_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_ha_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);


int write_code_const_c(ARGUMENTS *args,
                       GWEN_XMLNODE *node,
                       GWEN_BUFFEREDIO *bio);


int write_code_setget_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio);

int write_code_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_code_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);

int write_code_freeElem_c(ARGUMENTS *args,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFEREDIO *bio);
int write_code_freeElems_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_BUFFEREDIO *bio);

int write_code_dupArg_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio,
                        const char *param);

int write_code_dup_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio);
int write_code_todb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                      GWEN_BUFFEREDIO *bio);

int write_code_todbArg_c(ARGUMENTS *args,
                         GWEN_XMLNODE *node,
                         GWEN_BUFFEREDIO *bio);
int write_code_fromdbArg_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_BUFFEREDIO *bio);
int write_code_fromdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio);

GWEN_XMLNODE *find_type_node(GWEN_XMLNODE *node, const char *name);
const char *get_function_name(GWEN_XMLNODE *node, const char *ftype);

int write_code_builtin_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_BUFFEREDIO *bio);

const char *get_property(GWEN_XMLNODE *node,
                         const char *pname,
                         const char *defval);

const char *get_struct_property(GWEN_XMLNODE *node,
                                const char *pname,
                                const char *defval);
int write_apidoc_c(ARGUMENTS *args,
                   GWEN_XMLNODE *node,
                   GWEN_BUFFEREDIO *bio,
                   const char *acc);


#endif

