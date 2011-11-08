
MRNAPP_CONFIG = mrnapp-config

bin_SCRIPTS = $(MRNAPP_CONFIG)

$(MRNAPP_CONFIG): $(MRNAPP_CONFIG).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@"         "${prefix}"          $@
	$(top_srcdir)/substitute $(SED) "@sub_VERSION@"        "${VERSION}"         $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_CXXFLAGS@" "@MRNET_CXXFLAGS@"   $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_LIBSDIR@"  "@MRNET_LIBSDIR@"    $@
	$(top_srcdir)/substitute $(SED) "@sub_FE_MRNET_LIBS@"  "@MRNET_LIBS@"       $@
if USE_LIGHTWEIGHT
	$(top_srcdir)/substitute $(SED) "@sub_LIGHTWEIGHT@"    "-DLIGHTWEIGHT"      $@
	$(top_srcdir)/substitute $(SED) "@sub_BE_MRNET_LIBS@"  "@MRNET_LIGHT_LIBS@" $@
else
	$(top_srcdir)/substitute $(SED) "@sub_LIGHTWEIGHT@"    ""                   $@
	$(top_srcdir)/substitute $(SED) "@sub_BE_MRNET_LIBS@"  "@MRNET_LIBS@"       $@
endif
	chmod u+x $@

install-data-hook:
	chmod u+x ${bindir}/$(MRNAPP_CONFIG)

clean-local:
	rm -f $(MRNAPP_CONFIG)
