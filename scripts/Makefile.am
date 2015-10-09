
SYNAPSE_CONFIG = synapse-config
SOURCEME_SH    = sourceme.sh
MRNRUN         = mrnrun

EXTRA_DIST = $(SYNAPSE_CONFIG).in $(SOURCEME_SH).in $(MRNRUN).in 

bin_SCRIPTS    = $(SYNAPSE_CONFIG) 
#$(MRNRUN) This script was typically used to start the network in Marenostrum, but it is now deprecated
noinst_SCRIPTS = $(SOURCEME_SH)

$(SYNAPSE_CONFIG): $(SYNAPSE_CONFIG).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_PREFIX@" "@MRNET_HOME@" $@
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
	$(top_srcdir)/substitute $(SED) "@sub_SYNAPSE_HOME@" "${prefix}" $@
	$(top_srcdir)/substitute $(SED) "@sub_MRNET_HOME@" "@MRNET_HOME@" $@
    
install-data-hook:
	chmod u+x ${bindir}/$(SYNAPSE_CONFIG)
	$(MKDIR_P) $(prefix)/etc
	cp $(SOURCEME_SH) $(prefix)/etc

clean-local:
	rm -f $(SYNAPSE_CONFIG) $(SOURCEME_SH) $(MRNRUN)

