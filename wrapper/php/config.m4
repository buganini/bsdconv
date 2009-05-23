dnl config.m4 for extension bsdconv

CPPFLAGS=-I/usr/local/include

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(bsdconv, for bsdconv support,
dnl Make sure that the comment is aligned:
dnl [  --with-bsdconv             Include bsdconv support])

dnl Otherwise use enable:

 PHP_ARG_ENABLE(bsdconv, whether to enable bsdconv support,
 [  --enable-bsdconv           Enable bsdconv support])

if test "$PHP_BSDCONV" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-bsdconv -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/bsdconv.h"  # you most likely want to change this
  dnl if test -r $PHP_BSDCONV/$SEARCH_FOR; then # path given as parameter
  dnl   BSDCONV_DIR=$PHP_BSDCONV
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for bsdconv files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BSDCONV_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BSDCONV_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the bsdconv distribution])
  dnl fi

  dnl # --with-bsdconv -> add include path
  dnl PHP_ADD_INCLUDE($BSDCONV_DIR/include)

  dnl # --with-bsdconv -> check for lib and symbol presence
  dnl LIBNAME=bsdconv # you may want to change this
  dnl LIBSYMBOL=bsdconv # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BSDCONV_DIR/lib, BSDCONV_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_BSDCONVLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong bsdconv lib version or lib not found])
  dnl ],[
  dnl   -L$BSDCONV_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(BSDCONV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(bsdconv, bsdconv.c, $ext_shared)
fi
