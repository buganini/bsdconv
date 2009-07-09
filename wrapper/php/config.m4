dnl config.m4 for extension bsdconv

dnl If your extension references something external, use with:

 PHP_ARG_WITH(bsdconv, for bsdconv support,
dnl Make sure that the comment is aligned:
 [ --with-bsdconv             Include bsdconv support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(bsdconv, whether to enable bsdconv support,
dnl [  --enable-bsdconv           Enable bsdconv support])

if test "$PHP_BSDCONV" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-bsdconv -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/bsdconv.h"  # you most likely want to change this
  if test -r $PHP_BSDCONV/$SEARCH_FOR; then # path given as parameter
    BSDCONV_DIR=$PHP_BSDCONV
  else # search default path list
    AC_MSG_CHECKING([for bsdconv files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        BSDCONV_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  if test -z "$BSDCONV_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the bsdconv distribution])
  fi

  dnl # --with-bsdconv -> add include path
  PHP_ADD_INCLUDE($BSDCONV_DIR/include)

  dnl # --with-bsdconv -> check for lib and symbol presence
  LIBNAME=bsdconv
  LIBSYMBOL=bsdconv_create

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BSDCONV_DIR/lib, BSDCONV_SHARED_LIBADD)
    AC_DEFINE(HAVE_BSDCONVLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong bsdconv lib version or lib not found])
  ],[
    -L$BSDCONV_DIR/lib -lbsdconv
  ])
  PHP_SUBST(BSDCONV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(bsdconv, bsdconv.c, $ext_shared)
fi
