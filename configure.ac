#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libgeneva,0.5.0,ruediger.berlich@iwr.fzk.de )
AC_CONFIG_SRCDIR([src/GDouble.cpp])
AC_CONFIG_AUX_DIR(m4) 
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])

# Geneva needs Boost C++ Libraries to compile 


#AC_ARG_WITH(boost,
#    [ --with-boost=path prefix of Boost Libraries  installation. e.g. /usr/local or /opt],
#            [BOOST_PREFIX=$with_boost],
#	     AC_MSG_ERROR([You must call configure with the --with-boost option.
#	     This tells configure where to find the  Boost C++ library and headers.
#	    e.g. --with-boost=/usr/local or --with-boost=/opt])
#)


# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LN_S
AC_PROG_MAKE_SET

#BOOST_REQUIRE([3.14])
BOOST_REQUIRE
BOOST_SERIALIZATION(mt)
BOOST_DATE_TIME(mt)
BOOST_THREADS(mt)
BOOST_SYSTEM

CPPFLAGS="-O2"
CXXFLAGS="$CPPFLAGS"
CFLAGS="$CPPFLAGS"

#AC_SUBST(BOOST_PREFIX)
#BOOST_THREAD_LIBS="-L${BOOST_PREFIX}/lib -lboost_date_time  -lboost_thread -lboost_serialization -lboost_system"
#BOOST_CPPFLAGS="-I${BOOST_PREFIX}/include/boost"
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS" 
#AC_SUBST(BOOST_THREAD_LIBS)
#AC_SUBST(BOOST_CPPFLAGS)



#AC_DEFAULT_PREFIX(/opt) 


# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T


# Checks for library functions.
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([floor pow select socket sqrt])

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(libgeneva, doxygen.cfg, doc)

AC_CONFIG_FILES([Makefile src/Makefile tests/GenEvATestCase1/Makefile tests/GenEvATestCase2/Makefile tests/GenEvATestCase3/Makefile tests/GenEvATestCase4/Makefile])
AC_OUTPUT

