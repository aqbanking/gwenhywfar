

#ifndef TYPEMAKER_P_H
#define TYPEMAKER_P_H

#include <gwenhywfar/xml.h>

int write_xml_to_bio(GWEN_XMLNODE *n, GWEN_SYNCIO *sio, uint32_t flags);


int write_hp_group_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio);

int write_hp_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);

int write_h_setget_c(ARGUMENTS *args,
                     GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio,
                     const char *acc);

int write_h_header(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_SYNCIO *sio,
                   const char *where);

int write_h_enums(ARGUMENTS *args, GWEN_XMLNODE *node,
                  GWEN_SYNCIO *sio, const char *acc);

int write_h_funcs(ARGUMENTS *args, GWEN_XMLNODE *node,
                  GWEN_SYNCIO *sio,
                  const char *acc);

int write_c_enums(ARGUMENTS *args, GWEN_XMLNODE *node,
                  GWEN_SYNCIO *sio);

int write_h_elem_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_SYNCIO *sio);
int write_h_func_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_SYNCIO *sio);

int write_h_struct_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio);
int write_hp_group_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio);

int write_hp_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_hp_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);

int write_hl_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_hl_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);


int write_ha_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_ha_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);


int write_code_const_c(ARGUMENTS *args,
                       GWEN_XMLNODE *node,
                       GWEN_SYNCIO *sio);


int write_code_setget_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio);

int write_code_file_c(ARGUMENTS *args, GWEN_XMLNODE *node);
int write_code_files_c(ARGUMENTS *args, GWEN_XMLNODE *node);

int write_code_freeElem_c(ARGUMENTS *args,
                          GWEN_XMLNODE *node,
                          GWEN_SYNCIO *sio);
int write_code_freeElems_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_SYNCIO *sio);

int write_code_dupArg_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio,
                        const char *param);

int write_code_dupList_c(ARGUMENTS *args, GWEN_XMLNODE *n,
                         GWEN_SYNCIO *sio,
                         const char *listName);


int write_code_dup_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio);
int write_code_todb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                      GWEN_SYNCIO *sio);

int write_code_todbArg_c(ARGUMENTS *args,
                         GWEN_XMLNODE *node,
                         GWEN_SYNCIO *sio);
int write_code_fromdbArg_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_SYNCIO *sio);
int write_code_fromdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio);

int write_code_readdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio);


const char *get_function_name(GWEN_XMLNODE *node, const char *ftype);
const char *get_function_name2(GWEN_XMLNODE *node, const char *ftype,
                               const char *name);
GWEN_XMLNODE *get_typedef(GWEN_XMLNODE *node, const char *name);

int write_code_builtin_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_SYNCIO *sio);

const char *get_property(GWEN_XMLNODE *node,
                         const char *pname,
                         const char *defval);

const char *get_struct_property(GWEN_XMLNODE *node,
                                const char *pname,
                                const char *defval);

GWEN_XMLNODE *get_struct_node(GWEN_XMLNODE *node);


int write_apidoc_c(ARGUMENTS *args,
                   GWEN_XMLNODE *node,
                   GWEN_SYNCIO *sio,
                   const char *acc);


#endif

