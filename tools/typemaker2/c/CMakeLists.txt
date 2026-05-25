# ===========================================================================
# tools/typemaker2/c/CMakeLists.txt — C builder sub-library
# ===========================================================================

add_library(tm2_c STATIC
  tm_builder_c.c
  tm2c_misc.c
  tm2c_enums.c
  tm2c_constructor.c
  tm2c_destructor.c
  tm2c_dup.c
  tm2c_copy.c
  tm2c_typedef.c
  tm2c_struct.c
  tm2c_macros.c
  tm2c_getter.c
  tm2c_setter.c
  tm2c_readdb.c
  tm2c_writedb.c
  tm2c_readxml.c
  tm2c_writexml.c
  tm2c_object.c
  tm2c_readobject.c
  tm2c_writeobject.c
  tm2c_list1.c
  tm2c_tree1.c
  tm2c_tree2.c
  tm2c_signal.c
  tm2c_slot.c
  tm2c_virtualfn.c
)

target_include_directories(tm2_c PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
