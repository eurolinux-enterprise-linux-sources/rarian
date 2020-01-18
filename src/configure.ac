#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(rarian, 0.8.1)
AC_CONFIG_SRCDIR([util/rarian-example.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE ([foreign dist-bzip2 dist-gzip])

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_CXX

# All the fun stuff that is used for installing
AC_ARG_ENABLE([installed],
	[AC_HELP_STRING([--enable-installed],
			[Make rarian run in install mode [default=yes]])],,
	enable_installed=yes)
AC_ARG_ENABLE([scrollkeeper-compat],
	[AC_HELP_STRING([--enable-scrollkeeper-compat],
			[Build and install scrollkeeper compatibility gunk [default=yes]])],,
	enable_scrollkeeper_compat=yes)
AC_ARG_ENABLE([omf-read],
	[AC_HELP_STRING([--enable-omf-read],
			[Allow librarian to read omf files [default=yes]])],,
	enable_omf_read=yes)
AC_ARG_ENABLE([skdb-update],
	[AC_HELP_STRING([--enable-skdb-update],
			[Update the Scrollkeeper database [default=yes]])],,
	enable_skdb_update=yes)
AC_ARG_WITH([convert-dir],
	[AC_HELP_STRING([--with-convert-dir=<path/to/omfs>],
			[Chose a specific path to convert when installed [default=<prefix>/share/omf]])], ,
	with_convert_dir="$prefix/share/omf")

DISTCHECK_CONFIGURE_FLAGS="--disable-skdb-update"
AC_SUBST(DISTCHECK_CONFIGURE_FLAGS)

# If we try and install without setting the prefix, the convert dir will turn into NONE/omf
# Reset it here properly
if test "x$with_convert_dir" = "xNONE/share/omf"; then
echo Resetting conv dir
with_convert_dir="/usr/local/share/omf"
fi

echo conv dir is $with_convert_dir

if test "x$enable_installed" = "xyes"; then
    AC_DEFINE(ENABLE_INSTALL, 1, [turn on Installed Mode])
    AM_CONDITIONAL(ENABLE_INSTALL, true)
	ENABLE_INSTALL=1
	AC_SUBST([ENABLE_INSTALL])
else
	AC_DEFINE(ENABLE_INSTALL, 0, [turn on Installed Mode])
    AM_CONDITIONAL(ENABLE_INSTALL, false)
	ENABLE_INSTALL=0
   	AC_SUBST([ENABLE_INSTALL])
fi

if test "x$enable_scrollkeeper_compat" = "xyes"; then
    AC_DEFINE(ENABLE_SK_COMPAT, 1, [Scrollkeeper compatibility stuff])
    AM_CONDITIONAL(ENABLE_SK_COMPAT, true)
	ENABLE_SK_COMPAT=1
	AC_SUBST([ENABLE_SK_COMPAT])
else
	AC_DEFINE(ENABLE_SK_COMPAT, 0, [scrollkeeper compatibility stuff])
    AM_CONDITIONAL(ENABLE_SK_COMPAT, false)
	ENABLE_SK_COMPAT=0
   	AC_SUBST([ENABLE_SK_COMPAT])
fi

if test "x$enable_omf_read" = "xyes"; then
    AC_DEFINE(ENABLE_OMF_READ, 1, [Scrollkeeper compatibility stuff])
    AM_CONDITIONAL(ENABLE_OMF_READ, true)
	ENABLE_OMF_READ=1
	AC_SUBST([ENABLE_OMF_READ])
else
	AC_DEFINE(ENABLE_OMF_READ, 0, [scrollkeeper compatibility stuff])
    AM_CONDITIONAL(ENABLE_OMF_READ, false)
	ENABLE_OMF_READ=0
   	AC_SUBST([ENABLE_OMF_READ])
fi

if test "x$enable_skdb_update" = "xyes"; then
    AC_DEFINE(ENABLE_SKDB_UPDATE, 1, [Update the Scrollkeeper database])
    AM_CONDITIONAL(ENABLE_SKDB_UPDATE, true)
	ENABLE_SKDB_UPDATE=1
	AC_SUBST([ENABLE_SKDB_UPDATE])
else
	AC_DEFINE(ENABLE_SKDB_UPDATE, 0, [Update the Scrollkeeper database])
    AM_CONDITIONAL(ENABLE_SKDB_UPDATE, false)
	ENABLE_SKDB_UPDATE=0
   	AC_SUBST([ENABLE_SKDB_UPDATE])
fi

#AC_DEFINE_UNQUOTED(CONVERT_DIR, "$convert_dir", [Choose a directory to convert])
CONVERT_DIR="$with_convert_dir"
AC_SUBST([CONVERT_DIR])

# Check for existing scrollkeeper install.  If so, we're moving the
# files and symlinking
#AC_CHECK_PROG (SK_PRESENT, scrollkeeper-update, 1, 0, $bindir)
#echo $SK_PRESENT

if test "x$prefix" = xNONE ; then
   REAL_PREFIX=/usr/local
else
   REAL_PREFIX=$prefix
fi
old_prefix=$prefix
prefix=$REAL_PREFIX

if test "x$exec_prefix" = xNONE ; then
   REAL_EXEC_PREFIX=$prefix
else
   REAL_EXEC_PREFIX=$exec_prefix
fi
old_exec_prefix=$exec_prefix
exec_prefix=$REAL_EXEC_PREFIX

BINDIR_TMP="$bindir"
EXPANDED_BINDIR=`eval echo $BINDIR_TMP`
bindir=$EXPANDED_BINDIR

DATAROOTDIR_TMP="$datarootdir"
EXPANDED_DATAROOTDIR=`eval echo $DATAROOTDIR_TMP`
datarootdir=$EXPANDED_DATAROOTDIR

DATADIR_TMP="$datadir"
EXPANDED_DATADIR=`eval echo $DATADIR_TMP`
datadir=$EXPANDED_DATADIR

STATEDIR_TMP="$localstatedir"
EXPANDED_STATEDIR=`eval echo $STATEDIR_TMP`
localstatedir=$EXPANDED_STATEDIR

prefix=$old_prefix

#AC_SUBST(EXPANDED_BINDIR)
#echo $bindir

#exec_prefix=${prefix}
#echo ${exec_prefix} ...
#ENABLE_SK_COMPAT=0

# Ensure the sysconfdir exists

# Checks for libraries.
AC_CHECK_PROG(have_xslt, xsltproc, yes, no)

AM_CONDITIONAL(ENABLE_EXTRACT, test x"$have_xslt" = xyes)

AC_PATH_PROG(have_bash, bash)

AC_SUBST(BASH, $have_bash)

# Checks for header files.
AC_CHECK_HEADERS(malloc.h)

# Check for databases supported for man page support
# Adapted from man-db sources
## Check for Berkeley db
#AC_CHECK_HEADER(db4/db_185.h, 
#		AC_CHECK_LIB(db, dbopen, ,AC_CHECK_LIB(db-4, dbopen)))
#AC_CHECK_HEADER(db_185.h, 
#		AC_CHECK_LIB(db, dbopen, ,AC_CHECK_LIB(db-4, dbopen)))

dnl Check for GNU dbm routines
dnl
#AC_CHECK_HEADER(gdbm.h,[
#      AC_CHECK_LIB(gdbm, gdbm_open)])
dnl
dnl Check for UNIX ndbm routines
dnl
#AC_CHECK_HEADER(ndbm.h,[
#      AC_CHECK_LIB(ndbm, dbm_open)])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		 rarian.pc
                 data/Makefile
		 docs/Makefile
                 librarian/Makefile
                 util/Makefile
 		 util/rarian-sk-update
		 util/rarian-sk-install
		 util/rarian-sk-rebuild
		 util/rarian-sk-config
		 util/rarian-sk-get-scripts
		 util/rarian-sk-extract
		 util/rarian-sk-get-content-list
		 util/rarian-sk-get-extended-content-list
		 docs/rarian.document])
AC_OUTPUT

echo "
Rarian-$VERSION:
This is not API-stable yet."
if test "x$enable_installed" = "xyes"; then
	echo Prefix......... $prefix
else
	echo "Running in uninstalled (i.e. testing and broken) mode"
fi
