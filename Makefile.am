SUBDIRS=src test

bin_SCRIPTS=gwenhywfar-config
M4dir=@prefix@/share/aclocal
M4_DATA=gwenhywfar.m4
ACLOCAL_AMFLAGS = -I m4


gwenhywfar-config: gwenhywfar-config.in
	echo "#! /bin/sh" >gwenhywfar-config && \
	echo 'dir="@prefix@"' >>gwenhywfar-config && \
	cat gwenhywfar-config.in >>gwenhywfar-config && \
	chmod a+x gwenhywfar-config

