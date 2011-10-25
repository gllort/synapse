
bin_PROGRAMS = FE BE

FE_SOURCES   = FE.cpp Ping_FE.cpp
FE_CXXFLAGS  = -I. -I../src @MRNET_CXXFLAGS@
FE_LDADD     = ../src/.libs/libmrnapp_frontend.la

BE_SOURCES   = BE.cpp Ping_BE.cpp
BE_CXXFLAGS  = -I. -I../src @MRNET_CXXFLAGS@
BE_LDADD     = ../src/.libs/libmrnapp_backend.la

# You need to compile with -DLIGHTWEIGHT if the 
# back-ends are using the lightweight version 
# of the MRNet library!
if USE_LIGHTWEIGHT
BE_CXXFLAGS += -DLIGHTWEIGHT
endif
