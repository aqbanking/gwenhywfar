INCLUDES=@all_includes@

if IS_WINDOWS
# no test for windows for now
else
noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c
gwentest_LDADD=-L../src @gwenhywfar_libs@
endif
