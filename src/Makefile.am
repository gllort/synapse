

lib_LTLIBRARIES = libsynapse_frontend.la libsynapse_backend.la

libsynapse_frontend_la_SOURCES =         \
  MRNetApp.cpp           MRNetApp.h      \
  FrontEnd.cpp           FrontEnd.h      \
  FrontProtocol.cpp      FrontProtocol.h \
  Protocol.cpp           Protocol.h      \
  PendingConnections.cpp PendingConnections.h
libsynapse_frontend_la_CXXFLAGS = -g -O2 -Wall -Wl,-E @MRNET_CXXFLAGS@ 
libsynapse_frontend_la_LDFLAGS  = -L@MRNET_LIBSDIR@ -R @MRNET_LIBSDIR@ @MRNET_LIBS@ 

libsynapse_backend_la_SOURCES =          \
  MRNetApp.cpp           MRNetApp.h      \
  BackEnd.cpp            BackEnd.h       \
  BackProtocol.cpp       BackProtocol.h  \
  Protocol.cpp           Protocol.h      \
  PendingConnections.cpp PendingConnections.h
libsynapse_backend_la_CXXFLAGS  = -g -O2 -Wall -Wl,-E @MRNET_CXXFLAGS@ 
libsynapse_backend_la_LDFLAGS   = -L@MRNET_LIBSDIR@ -R @MRNET_LIBSDIR@
if USE_LIGHTWEIGHT
libsynapse_backend_la_CXXFLAGS += -DLIGHTWEIGHT
libsynapse_backend_la_LDFLAGS  += @MRNET_LIGHT_LIBS@
else
libsynapse_backend_la_LDFLAGS  += @MRNET_LIBS@
endif

include_HEADERS = MRNetApp.h FrontEnd.h BackEnd.h Protocol.h FrontProtocol.h BackProtocol.h MRNet_wrappers.h MRNet_tags.h PendingConnections.h

