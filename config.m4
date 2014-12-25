dnl $Id$
dnl config.m4 for extension phoenix

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(phoenix, for phoenix support,
dnl Make sure that the comment is aligned:
dnl [  --with-phoenix             Include phoenix support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(phoenix, whether to enable phoenix support,
dnl Make sure that the comment is aligned:
dnl [  --enable-phoenix           Enable phoenix support])

if test "$PHP_PHOENIX" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-phoenix -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/phoenix.h"  # you most likely want to change this
  dnl if test -r $PHP_PHOENIX/$SEARCH_FOR; then # path given as parameter
  dnl   PHOENIX_DIR=$PHP_PHOENIX
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for phoenix files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PHOENIX_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PHOENIX_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the phoenix distribution])
  dnl fi

  dnl # --with-phoenix -> add include path
  dnl PHP_ADD_INCLUDE($PHOENIX_DIR/include)

  dnl # --with-phoenix -> check for lib and symbol presence
  dnl LIBNAME=phoenix # you may want to change this
  dnl LIBSYMBOL=phoenix # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PHOENIX_DIR/$PHP_LIBDIR, PHOENIX_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PHOENIXLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong phoenix lib version or lib not found])
  dnl ],[
  dnl   -L$PHOENIX_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PHOENIX_SHARED_LIBADD)

  PHP_NEW_EXTENSION(phoenix, phoenix.c, $ext_shared)
fi
