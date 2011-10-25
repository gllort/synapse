

lib_LTLIBRARIES = libmrnapp_frontend.la libmrnapp_backend.la

libmrnapp_frontend_la_SOURCES =     \
  MRNetApp.cpp      MRNetApp.h      \
  FrontEnd.cpp      FrontEnd.h      \
  FrontProtocol.cpp FrontProtocol.h Protocol.h
libmrnapp_frontend_la_CXXFLAGS  = -g -O2 -Wall @MRNET_CXXFLAGS@ 
libmrnapp_frontend_la_LDFLAGS   = -L@MRNET_LIBSDIR@ -R @MRNET_LIBSDIR@ @MRNET_LIBS@ -Wl,-E 

libmrnapp_backend_la_SOURCES  =     \
  MRNetApp.cpp      MRNetApp.h      \
  BackEnd.cpp       BackEnd.h       \
  BackProtocol.cpp  BackProtocol.h  Protocol.h 
libmrnapp_backend_la_CXXFLAGS  = -g -O2 -Wall @MRNET_CXXFLAGS@ 
libmrnapp_backend_la_LDFLAGS   = -L@MRNET_LIBSDIR@ -R @MRNET_LIBSDIR@ -Wl,-E 
if USE_LIGHTWEIGHT
libmrnapp_backend_la_CXXFLAGS += -DLIGHTWEIGHT
libmrnapp_backend_la_LDFLAGS  += @MRNET_LIGHT_LIBS@
else
libmrnapp_backend_la_LDFLAGS  += @MRNET_LIBS@
endif
