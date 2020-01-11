
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "derivedclass.h"
#include <assert.h>

int main(GWEN_UNUSED int argc, GWEN_UNUSED char *argv[])
{
  BASECLASS *object = DerivedClass_new("some data");
  assert(object);
  printf("DerivedClass_new: Test object created with data \"%s\"\n",
         DerivedClass_GetTestData(object));
  BaseClass_free(object);
  printf("BaseClass_free: Object freed.\n");
  return 0;
}
