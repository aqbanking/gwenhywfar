# "make installcheck" runs the gwentest commands that "gwentest --list"
# shows as needing no arguments, or only optional ones, and as neither
# interactive nor tools. They run after installation, because some of
# them load the installed plugins. Each command is one test of the
# Automake test harness. The commands run one at a time in the order of
# the list, all in run/, because some of them read the files that earlier
# ones write.

.NOTPARALLEL:

EXTRA_DIST = gwentest-run.sh

# installcheck-local sets the tests.
TESTS =
TEST_EXTENSIONS = .gwt
GWT_LOG_COMPILER = $(SHELL) $(srcdir)/gwentest-run.sh
AM_GWT_LOG_FLAGS = $(abs_top_builddir)/test/gwentest$(EXEEXT)

gwentest_outputs = run list *.gwt *.log *.trs

installcheck-local:
	rm -rf $(gwentest_outputs)
	$(MKDIR_P) run
	../gwentest$(EXEEXT) --list >list
	tests=`tr -d '\r' <list | \
	  $(AWK) -F '\t' '$$2 != "required" && $$3 != "interactive" && \
	    $$3 != "tool" { printf "%s.gwt ", $$1 }'` && \
	for t in $$tests; do : >$$t; done && \
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS TESTS="$$tests"

clean-local:
	rm -rf $(gwentest_outputs)
