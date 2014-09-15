# AX_FLAGS_SAVE
# -------------
AC_DEFUN([AX_FLAGS_SAVE],
[
   saved_LIBS="${LIBS}"
   saved_CC="${CC}"
   saved_CFLAGS="${CFLAGS}"
   saved_CXXFLAGS="${CXXFLAGS}"
   saved_CPPFLAGS="${CPPFLAGS}"
   saved_LDFLAGS="${LDFLAGS}"
])


# AX_FLAGS_RESTORE
# ----------------
AC_DEFUN([AX_FLAGS_RESTORE],
[
   LIBS="${saved_LIBS}"
   CC="${saved_CC}"
   CFLAGS="${saved_CFLAGS}"
   CXXFLAGS="${saved_CXXFLAGS}"
   CPPFLAGS="${saved_CPPFLAGS}"
   LDFLAGS="${saved_LDFLAGS}"
])


# AX_FIND_INSTALLATION
# --------------------
AC_DEFUN([AX_FIND_INSTALLATION],
[
	AC_REQUIRE([AX_SELECT_BINARY_TYPE])

	dnl Search for home directory
	AC_MSG_CHECKING([for $1 installation])
    for home_dir in [$2 "not found"]; do
        if test -d "$home_dir/$BITS" ; then
            home_dir="$home_dir/$BITS"
            break
        elif test -d "$home_dir" ; then
            break
        fi
    done
	AC_MSG_RESULT([$home_dir])
	$1_HOME="$home_dir"
	if test "$$1_HOME" = "not found" ; then
		$1_HOME=""
	else

		dnl Did the user passed a headers directory to check first?
		AC_ARG_WITH([$3-headers],
			AC_HELP_STRING(
				[--with-$3-headers@<:@=ARG@:>@],
				[Specify location of include files for package $3]
			),
			[ForcedHeaders="$withval"],
			[ForcedHeaders=""]
		)

		dnl Search for includes directory
		AC_MSG_CHECKING([for $1 includes directory])

		if test "${ForcedHeaders}" = "" ; then
			for incs_dir in [$$1_HOME/include$BITS $$1_HOME/include "not found"] ; do
				if test -d "$incs_dir" ; then
					break
				fi
			done
		else
			for incs_dir in [${ForcedHeaders} "not found"] ; do
				if test -d "$incs_dir" ; then
					break
				fi
			done
		fi

		AC_MSG_RESULT([$incs_dir])
		$1_INCLUDES="$incs_dir"
		if test "$$1_INCLUDES" = "not found" ; then
			AC_MSG_ERROR([Unable to find header directory for package $3. Check option --with-$3-headers.])
		else
			$1_CFLAGS="-I$$1_INCLUDES"
			$1_CXXFLAGS="-I$$1_INCLUDES"
			$1_CPPFLAGS="-I$$1_INCLUDES"
		fi

		dnl Did the user passed a headers directory to check first?
		AC_ARG_WITH([$3-libs],
			AC_HELP_STRING(
				[--with-$3-libs@<:@=ARG@:>@],
				[Specify location of library files for package $3]
			),
			[ForcedLibs="$withval"],
			[ForcedLibs=""]
		)

		dnl Search for libs directory
		AC_MSG_CHECKING([for $1 libraries directory])
		if test "${ForcedLibs}" = "" ; then
			for libs_dir in [$$1_HOME/lib$BITS $$1_HOME/lib "not found"] ; do
				if test -d "$libs_dir" ; then
					break
				fi
			done
		else
			for libs_dir in [${ForcedLibs} "not found"] ; do
				if test -d "$libs_dir" ; then
					break
				fi
			done
		fi

		AC_MSG_RESULT([$libs_dir])
		$1_LIBSDIR="$libs_dir"
		if test "$$1_LIBSDIR" = "not found" ; then
			AC_MSG_ERROR([Unable to find library directory for package $3. Check option --with-$3-libs.])
		else
       $1_LDFLAGS="-L$$1_LIBSDIR"
       if test -d "$$1_LIBSDIR/shared" ; then
          $1_SHAREDLIBSDIR="$$1_LIBSDIR/shared"
       else
          $1_SHAREDLIBSDIR=$$1_LIBSDIR
       fi
		fi
	fi

	dnl Everything went OK?
	if test "$$1_HOME" != "" -a "$$1_INCLUDES" != "" -a "$$1_LIBSDIR" != "" ; then
		$1_INSTALLED="yes"

		AC_SUBST($1_HOME)
		AC_SUBST($1_INCLUDES)

    AC_SUBST($1_CFLAGS)
    AC_SUBST($1_CXXFLAGS)
    AC_SUBST($1_CPPFLAGS)

    AC_SUBST($1_LDFLAGS)
    AC_SUBST($1_SHAREDLIBSDIR)
    AC_SUBST($1_LIBSDIR)

    dnl Update the default variables so the automatic checks will take into account the new directories
    CFLAGS="$CFLAGS $$1_CFLAGS"
    CXXFLAGS="$CXXFLAGS $$1_CXXFLAGS"
    CPPFLAGS="$CPPFLAGS $$1_CPPFLAGS"
    LDFLAGS="$LDFLAGS $$1_LDFLAGS"
	else	
		$1_INSTALLED="no"
	fi
])


# AX_CHECK_POINTER_SIZE
# ---------------------
AC_DEFUN([AX_CHECK_POINTER_SIZE],
[
   AC_TRY_RUN(
      [
         int main()
         {
            return sizeof(void *)*8;
         }
      ],
      [ POINTER_SIZE="0" ],
      [ POINTER_SIZE="$?"]
   )
])


# AX_SELECT_BINARY_TYPE
# ---------------------
# Check the binary type the user wants to build and verify whether it can be successfully built
AC_DEFUN([AX_SELECT_BINARY_TYPE],
[
	AC_ARG_WITH(binary-type,
		AC_HELP_STRING(
			[--with-binary-type@<:@=ARG@:>@],
			[choose the binary type between: 32, 64, default @<:@default=default@:>@]
		),
		[Selected_Binary_Type="$withval"],
		[Selected_Binary_Type="default"]
	)

	if test "$Selected_Binary_Type" != "default" -a "$Selected_Binary_Type" != "32" -a "$Selected_Binary_Type" != "64" ; then
		AC_MSG_ERROR([--with-binary-type: Invalid argument '$Selected_Binary_Type'. Valid options are: 32, 64, default.])
	fi

	C_compiler="$CC"
	CXX_compiler="$CXX"

	AC_LANG_SAVE([])
	m4_foreach([language], [[C], [C++]], [
		AC_LANG_PUSH(language)

		AC_CACHE_CHECK(
			[for $_AC_LANG_PREFIX[]_compiler compiler default binary type], 
			[[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type],
			[
				AX_CHECK_POINTER_SIZE
				Default_Binary_Type="$POINTER_SIZE"
				[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type="$Default_Binary_Type""-bit"
			]
		)

		if test "$Default_Binary_Type" != "32" -a "$Default_Binary_Type" != 64 ; then
			AC_MSG_ERROR([Unknown default binary type (pointer size is $POINTER_SIZE!?)])
		fi

		if test "$Selected_Binary_Type" = "default" ; then
			Selected_Binary_Type="$Default_Binary_Type"
		fi

		if test "$Selected_Binary_Type" != "$Default_Binary_Type" ; then

			force_bit_flags="-m32 -q32 -32 -maix32 -m64 -q64 -64 -maix64 none"

			AC_MSG_CHECKING([for $_AC_LANG_PREFIX[]_compiler compiler flags to build a $Selected_Binary_Type-bit binary])
			for flag in [$force_bit_flags]; do
				old_[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS"
				[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $flag"

				AX_CHECK_POINTER_SIZE()
				if test "$POINTER_SIZE" = "$Selected_Binary_Type" ; then
					AC_MSG_RESULT([$flag])
					break
				else
					[]_AC_LANG_PREFIX[]FLAGS="$old_[]_AC_LANG_PREFIX[]FLAGS"
					if test "$flag" = "none" ; then
						AC_MSG_RESULT([unknown])
						AC_MSG_NOTICE([${Selected_Binary_Type}-bit binaries not supported])
						AC_MSG_ERROR([Please use '--with-binary-type' to select an appropriate binary type.])

					fi
				fi
			done
		fi
		AC_LANG_POP(language)
	])
	AC_LANG_RESTORE([])
	BITS="$Selected_Binary_Type"
])


# AX_CHECK_ENDIANNESS
# -------------------
# Test if the architecture is little or big endian
AC_DEFUN([AX_CHECK_ENDIANNESS],
[
   AC_CACHE_CHECK([for the architecture endianness], [ac_cv_endianness],
   [
      AC_LANG_SAVE()
      AC_LANG([C])
      AC_TRY_RUN(
      [
         int main()
         {
            short s = 1;
            short * ptr = &s;
            unsigned char c = *((char *)ptr);
            return c;
         }
      ],
      [ac_cv_endianness="big endian" ],
      [ac_cv_endianness="little endian" ]
      )
      AC_LANG_RESTORE()
   ])
   if test "$ac_cv_endianness" = "big endian" ; then
      AC_DEFINE(IS_BIG_ENDIAN, 1, [Define to 1 if architecture is big endian])
   fi
   if test "$ac_cv_endianness" = "little endian" ; then
      AC_DEFINE(IS_LITTLE_ENDIAN, 1, [Define to 1 if architecture is little endian])
   fi
])

# AX_PROG_MRNET
# -------------
AC_DEFUN([AX_PROG_MRNET],
[
	AX_FLAGS_SAVE()
	AC_LANG_SAVE()
	AC_LANG([C++])

	AC_ARG_WITH(mrnet,
		AC_HELP_STRING(
			[--with-mrnet@<:@=DIR@:>@],
			[specify where to find MRNet libraries and includes]
		),
		[mrnet_paths="$withval"],
		[mrnet_paths="/home/bsc41/bsc41127/apps/MRNET/last"] dnl List of possible default paths
	)

	dnl Search for MRNet installation
	AX_FIND_INSTALLATION([MRNET], [$mrnet_paths], [mrnet])

	if test "$MRNET_INSTALLED" = "yes" ; then

dnl		dnl Begin hack
dnl		MRNET_CXXFLAGS="${MRNET_CXXFLAGS} -I${MRNET_HOME}/src/src -Dos_linux"
dnl		AC_SUBST(MRNET_CXXFLAGS)
dnl		dnl End hack

		MRNET_CXXFLAGS="${MRNET_CXXFLAGS} -I${MRNET_INCLUDES}/mrnet -I${MRNET_LIBSDIR}/mrnet-4.0.0/include -I${MRNET_LIBSDIR}/xplat-4.0.0/include"

		AC_MSG_CHECKING([for lightweight libraries for the back-ends])
		AC_ARG_WITH(lightweight,
			AC_HELP_STRING(
				[--with-lightweight],
				[use the MRNet lightweight libraries for the back-ends]
			),
			[light_libs="$withval"],
			[light_libs="no"]
		)
		AC_MSG_RESULT([$light_libs])

		dnl Check for MRNet header files.
		CXXFLAGS="${CXXFLAGS} ${MRNET_CXXFLAGS}"
		CPPFLAGS="${CPPFLAGS} ${MRNET_CXXFLAGS}"
		AC_CHECK_HEADERS([MRNet.h], [], [MRNET_INSTALLED="no"])

		dnl Check for libraries.
		AC_MSG_CHECKING([for libmrnet and libxplat])

		if test -f ${MRNET_LIBSDIR}/libmrnet.a -a -f ${MRNET_LIBSDIR}/libxplat.a ; then
			MRNET_LIBS="-lmrnet -lxplat -lpthread -ldl"
			MRNET_LIGHT_LIBS="-lmrnet_lightweight -lxplat_lightweight -lpthread -ldl"
			AC_SUBST(MRNET_LIBS)
			AC_SUBST(MRNET_LIGHT_LIBS)
			AC_MSG_RESULT([yes])
		else
			MRNET_INSTALLED="no"
			AC_MSG_RESULT([no])
		fi
	fi

	if test "${MRNET_INSTALLED}" = "no" ; then
		AC_MSG_WARN([MRNet support has been disabled])
	else
		AC_DEFINE([HAVE_MRNET], 1, [Define to 1 if MRNET is installed in the system])
	fi

	dnl Did the checks pass?
	AM_CONDITIONAL(HAVE_MRNET, test "${MRNET_INSTALLED}" = "yes")
	AM_CONDITIONAL(USE_LIGHTWEIGHT, test "${light_libs}" = "yes")
	
	AX_FLAGS_RESTORE()
	AC_LANG_RESTORE()
])
