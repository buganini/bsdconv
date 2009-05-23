/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifndef PHP_BSDCONV_H
#define PHP_BSDCONV_H

extern zend_module_entry bsdconv_module_entry;
#define phpext_bsdconv_ptr &bsdconv_module_entry

#ifdef PHP_WIN32
#define PHP_BSDCONV_API __declspec(dllexport)
#else
#define PHP_BSDCONV_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(bsdconv);
PHP_MSHUTDOWN_FUNCTION(bsdconv);
PHP_RINIT_FUNCTION(bsdconv);
PHP_RSHUTDOWN_FUNCTION(bsdconv);
PHP_MINFO_FUNCTION(bsdconv);

#ifdef ZTS
#define BSDCONV_G(v) TSRMG(bsdconv_globals_id, zend_bsdconv_globals *, v)
#else
#define BSDCONV_G(v) (bsdconv_globals.v)
#endif

#endif	/* PHP_BSDCONV_H */
