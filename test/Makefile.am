
SYNAPSE_CONFIG = ${top_srcdir}/scripts/synapse-config

noinst_PROGRAMS = test_FE test_BE

EXTRA_DIST = run.sh topology_1x4.txt run-example Makefile-example

test_FE_SOURCES  = FE.cpp Ping_FE.cpp Ping_FE.h tags.h
test_FE_CXXFLAGS = -I${top_srcdir}/src @MRNET_CXXFLAGS@
test_FE_LDFLAGS  = -L${top_srcdir}/src -lsynapse_frontend -L@MRNET_LIBSDIR@ @MRNET_LIBS@

test_BE_SOURCES  = BE.cpp Ping_BE.cpp Ping_BE.h tags.h
test_BE_CXXFLAGS = -I${top_srcdir}/src @MRNET_CXXFLAGS@
test_BE_LDFLAGS  = -L${top_srcdir}/src -lsynapse_backend -L@MRNET_LIBSDIR@ @MRNET_LIBS@

install-data-hook:
	mkdir -p ${prefix}/example
	cp ${test_FE_SOURCES} ${test_BE_SOURCES} ${prefix}/example
	cp Makefile-example ${prefix}/example/Makefile
	cp run-example ${prefix}/example/run.sh
	cp topology_1x4.txt ${prefix}/example
	chmod u+x ${prefix}/example/run.sh
	$(top_srcdir)/substitute $(SED) "@sub_SYNAPSE_HOME@" "${prefix}" ${prefix}/example/Makefile
	$(top_srcdir)/substitute $(SED) "@sub_SYNAPSE_HOME@" "${prefix}" ${prefix}/example/run.sh


