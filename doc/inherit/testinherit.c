#include "derivedclass.h"
#include <assert.h>

int main(int argc, char *argv[]) {
  BASECLASS *object = DerivedClass_new("some data");
  assert(object);
  printf("DerivedClass_new: Test object created with data \"%s\"\n",
	 DerivedClass_GetTestData(object));
  BaseClass_free(object);
  printf("BaseClass_free: Object freed.\n");
  return 0;
}
