# Builds consumer.c against the installed Gwenhywfar for "make installcheck".
# Pass -Dgwenhywfar_DIR=<directory of the package files>,
# -DGWENHYWFAR_EXPECTED_VERSION=<version> and
# -DCONSUMER_GENERATED_DIR=<directory with the typemaker output>.
cmake_minimum_required(VERSION 3.10)
project(gwenhywfar_consumer C)

find_package(gwenhywfar ${GWENHYWFAR_EXPECTED_VERSION} EXACT REQUIRED)

add_executable(consumer consumer.c
  ${CONSUMER_GENERATED_DIR}/consumer_type1.c
  ${CONSUMER_GENERATED_DIR}/consumer_type2.c)
target_include_directories(consumer PRIVATE ${CONSUMER_GENERATED_DIR})
target_link_libraries(consumer PRIVATE gwenhywfar::core)

enable_testing()
add_test(NAME consumer COMMAND consumer)
