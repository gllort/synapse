
MRNAPP_CONFIG = ${prefix}/bin/mrnapp-config

noinst_PROGRAMS = test_FE test_BE

test_FE_SOURCES  = FE.cpp Ping_FE.cpp
test_FE_CXXFLAGS = `${MRNAPP_CONFIG} --fe-cflags`
test_FE_LDFLAGS  = `${MRNAPP_CONFIG} --fe-libs`

test_BE_SOURCES  = BE.cpp Ping_BE.cpp
test_BE_CXXFLAGS = `${MRNAPP_CONFIG} --be-cflags`
test_BE_LDFLAGS  = `${MRNAPP_CONFIG} --be-libs`

