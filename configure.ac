
# Process this file with autoconf to produce a configure script.

#########################
#    Initializations    #
#########################

# Initialize autoconf & define package name, version and bug-report address 
AC_INIT(MRNetApp Library, 1.0.2, tools@bsc.es)

# GNU Autotools intermediate files are stored in the following directory
AC_CONFIG_AUX_DIR(config)

# Loads some shell variables like host_cpu and host_os, to get the host information 
AC_CANONICAL_SYSTEM

# Initialize automake
AM_INIT_AUTOMAKE
AC_PROG_MKDIR_P

# Specify the output configuration header file
AM_CONFIG_HEADER(config.h)

# Search for libtool support
AC_LIBTOOL_DLOPEN
AC_ENABLE_SHARED
AC_PROG_LIBTOOL

#############################
#    Checks for programs    #
#############################

AC_PROG_CC(gcc xlc cc)
AC_PROG_CXX(g++ xlC CC)
AC_PROG_CPP
AC_PROG_INSTALL

# Automake 1.10 reports a problem if this is not defined
AM_PROG_CC_C_O

##############################
#    Checks for libraries    #
##############################
AX_PROG_MRNET
if test "x${MRNET_INSTALLED}" = "xno"; then
  AC_ERROR([MRNET libraries not found in your system. Try using '--with-mrnet'])
fi
BOOST_REQUIRE([1.36],
              [AC_MSG_ERROR([This package requires the Boost Shared Pointer, but
                             it was not found in your system])])

#############################
#    Checks for headers     #
#############################

###########################
#    Checks for types     #
###########################

AC_C_CONST
AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile src/Makefile scripts/Makefile test/Makefile])

AC_OUTPUT
