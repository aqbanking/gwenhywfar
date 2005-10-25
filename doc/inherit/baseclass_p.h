

#ifndef GWEN_EXAMPLE_INHERIT_BASECLASS_P_H
#define GWEN_EXAMPLE_INHERIT_BASECLASS_P_H

#include "baseclass.h"


struct BASECLASS {
  /* the following line creates the necessary structs which are needed by
   * GWEN's inheritance code. They are later initialized by invocation of
   * GWEN_INHERIT_INIT from the constructor of this struct. */
  GWEN_INHERIT_ELEMENT(BASECLASS)
  /* the rest are private elements defined by the developer */
  int someMember;
};


#endif /* GWEN_EXAMPLE_INHERIT_BASECLASS_P_H */



