

lib_LTLIBRARIES = libmrnapp_frontend.la libmrnapp_backend.la

libmrnapp_frontend_la_SOURCES =          \
  MRNetApp.cpp           MRNetApp.h      \
  FrontEnd.cpp           FrontEnd.h      \
  FrontProtocol.cpp      FrontProtocol.h \
  Protocol.cpp           Protocol.h      \
  PendingConnections.cpp PendingConnections.h
libmrnapp_frontend_la_CXXFLAGS  = -g -O2 -Wall -Wl,-E @MRNET_CXXFLAGS@ @BOOST_CPPFLAGS@
libmrnapp_frontend_la_LDFLAGS   = -L@MRNET_LIBSDIR@ -R @MRNET_LIBSDIR@ @MRNET_LIBS@ 

libmrnapp_backend_la_SOURCES  =          \
  MRNetApp.cpp           MRNetApp.h      \
  BackEnd.cpp            BackEnd.h       \
  BackProtocol.cpp       BackProtocol.h  \
  Protocol.cpp           Protocol.h      \
  PendingConnections.cpp PendingConnections.h
libmrnapp_backend_la_CXXFLAGS  = -g -O2 -Wall -Wl,-E @MRNET_CXXFLAGS@ @BOOST_CPPFLAGS@
libmrnapp_backend_la_LDFLAGS   = -L@MRNET_LIBSDIR@ -R @MRNET_LIBSDIR@
if USE_LIGHTWEIGHT
libmrnapp_backend_la_CXXFLAGS += -DLIGHTWEIGHT
libmrnapp_backend_la_LDFLAGS  += @MRNET_LIGHT_LIBS@
else
libmrnapp_backend_la_LDFLAGS  += @MRNET_LIBS@
endif

include_HEADERS = MRNetApp.h FrontEnd.h BackEnd.h Protocol.h FrontProtocol.h BackProtocol.h MRNet_wrappers.h MRNet_tags.h PendingConnections.h

