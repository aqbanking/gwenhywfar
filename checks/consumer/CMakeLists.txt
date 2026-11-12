# Builds consumer.c against the installed Gwenhywfar for "make installcheck".
# Pass -Dgwenhywfar_DIR=<directory of the package files> and
# -DGWENHYWFAR_EXPECTED_VERSION=<version>.
cmake_minimum_required(VERSION 3.10)
project(gwenhywfar_consumer C)

find_package(gwenhywfar ${GWENHYWFAR_EXPECTED_VERSION} EXACT REQUIRED)

add_executable(consumer consumer.c)
target_link_libraries(consumer PRIVATE gwenhywfar::core)

enable_testing()
add_test(NAME consumer COMMAND consumer)
