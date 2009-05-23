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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_bsdconv.h"

/* True global resources - no need for thread safety here */
static int le_bsdconv;

#include <bsdconv.h>

/* {{{ proto resource bsdconv_create(string conversion)
  create bsdconv instance */
PHP_FUNCTION(bsdconv_create){
	char *c, *p;
	int l;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &c, &l) == FAILURE){
		return;
	}
	RETURN_RESOURCE((long int)bsdconv_create(c));
}
/* }}} */

/* {{{ proto bool bsdconv_destroy(resource ins)
  destroy bsdconv instance */
PHP_FUNCTION(bsdconv_destroy){
	zend_rsrc_list_entry *p;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &p) == FAILURE){
		RETURN_BOOL(0);
	}
	bsdconv_destroy(p->ptr);
	RETURN_BOOL(1);
}
/* }}} */

/* {{{ proto mixed bsdconv(resource ins, string str)
  bsdconv main function
*/
PHP_FUNCTION(bsdconv){
	zend_rsrc_list_entry *r;
	struct bsdconv_instance *p;
	char *c;
	int l;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &r, &c, &l) == FAILURE){
		RETURN_BOOL(0);
	}
	p=r->ptr;
	p->mode=BSDCONV_CC;
	p->feed=c;
	p->feed_len=l;
	bsdconv_init(p);
	bsdconv(p);
	RETURN_STRINGL(p->back, p->back_len, 0);
}
/* }}} */

/* {{{ bsdconv_functions[]
 *
 * Every user visible function must have an entry in bsdconv_functions[].
 */
zend_function_entry bsdconv_functions[] = {
	PHP_FE(bsdconv_create,	NULL)
	PHP_FE(bsdconv_destroy,	NULL)
	PHP_FE(bsdconv,	NULL)
	{NULL, NULL, NULL}	/* Must be the last line in bsdconv_functions[] */
};
/* }}} */

/* {{{ bsdconv_module_entry
 */
zend_module_entry bsdconv_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"bsdconv",
	bsdconv_functions,
	PHP_MINIT(bsdconv),
	PHP_MSHUTDOWN(bsdconv),
	NULL,
	NULL,
	PHP_MINFO(bsdconv),
#if ZEND_MODULE_API_NO >= 20010901
	"0.8", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_BSDCONV
ZEND_GET_MODULE(bsdconv)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(bsdconv)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(bsdconv)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(bsdconv)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "bsdconv support", "enabled");
	php_info_print_table_end();
}
/* }}} */
