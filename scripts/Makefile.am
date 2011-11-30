
MRNAPP_CONFIG = mrnapp-config
SOURCEME_SH   = sourceme.sh
MRNRUN        = mrnrun

EXTRA_DIST = $(MRNAPP_CONFIG).in $(SOURCEME_SH).in $(MRNRUN).in 

bin_SCRIPTS    = $(MRNAPP_CONFIG) $(MRNRUN)
noinst_SCRIPTS = $(SOURCEME_SH)

$(MRNAPP_CONFIG): $(MRNAPP_CONFIG).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $@
	$(top_srcdir)/substitute $(SED) "@sub_VERSION@" "${VERSION}" $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_CXXFLAGS@" "@MRNET_CXXFLAGS@" $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_LIBSDIR@" "@MRNET_LIBSDIR@" $@
	$(top_srcdir)/substitute $(SED) "@sub_FE_MRNET_LIBS@" "@MRNET_LIBS@" $@
if USE_LIGHTWEIGHT
	$(top_srcdir)/substitute $(SED) "@sub_LIGHTWEIGHT@" "-DLIGHTWEIGHT" $@
	$(top_srcdir)/substitute $(SED) "@sub_BE_MRNET_LIBS@" "@MRNET_LIGHT_LIBS@" $@
else
	$(top_srcdir)/substitute $(SED) "@sub_LIGHTWEIGHT@" "" $@
	$(top_srcdir)/substitute $(SED) "@sub_BE_MRNET_LIBS@" "@MRNET_LIBS@" $@
endif
	chmod u+x $@

$(MRNRUN): $(MRNRUN).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $@
	chmod u+x $@

$(SOURCEME_SH): $(SOURCEME_SH).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNAPP_HOME@" "${prefix}" $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_HOME@" "@MRNET_HOME@" $@
    
install-data-hook:
	chmod u+x ${bindir}/$(MRNAPP_CONFIG)
	$(mkdir_p) $(prefix)/etc
	cp $(SOURCEME_SH) $(prefix)/etc

clean-local:
	rm -f $(MRNAPP_CONFIG) $(SOURCEME_SH) $(MRNRUN)

