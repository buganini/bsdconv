/*
 * Copyright (c) 2009,2010 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

#define IBUFLEN 1024
#define OBUFLEN 1024

/* {{{ proto resource bsdconv_create(string conversion)
  create bsdconv instance */
PHP_FUNCTION(bsdconv_create){
	char *c;
	int l;
	struct bsdconv_instance *r;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &c, &l) == FAILURE){
		return;
	}
	r=bsdconv_create(c);
	if(r==NULL) RETURN_BOOL(0);
	RETURN_RESOURCE((long int)r);
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
	p->mode=BSDCONV_CM;
	p->feed=c;
	p->feed_len=l;
	bsdconv_init(p);
	bsdconv(p);
	p->back=emalloc(p->back_len);
	bsdconv(p);
	RETURN_STRINGL(p->back, p->back_len, 0);
}
/* }}} */

/* {{{ proto mixed bsdconv_file(resource ins, string infile, string outfile)
  bsdconv_file function
*/
PHP_FUNCTION(bsdconv_file){
	zend_rsrc_list_entry *r;
	struct bsdconv_instance *p;
	char *s1, *s2;
	int l,t;
	FILE *inf, *otf;
	unsigned char in[IBUFLEN], out[OBUFLEN];

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &r, &s1, &l, &s2, &l) == FAILURE){
		RETURN_BOOL(0);
	}
	p=r->ptr;

	inf=fopen(s1,"r");
	if(!inf) RETURN_BOOL(0);
	otf=fopen(s2,"w");
	if(!otf) RETURN_BOOL(0);
	p->in_buf=in;
	p->in_len=IBUFLEN;
	p->out_buf=out;
	p->out_len=OBUFLEN;
	p->mode=BSDCONV_BB;
	bsdconv_init(p);
	do{
		if(p->feed_len) p->feed_len=fread(p->feed, 1, p->feed_len, inf);
		r=bsdconv(p);
		if(p->back_len)fwrite(p->back, 1, p->back_len, otf);
	}while(r);

	fclose(inf);
	fclose(otf);

	RETURN_BOOL(1);
}
/* }}} */

/* {{{ proto array bsdconv_info(resource ins)
  bsdconv conversion info function
*/
PHP_FUNCTION(bsdconv_info){
	zend_rsrc_list_entry *r;
	struct bsdconv_instance *ins;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &r) == FAILURE){
		RETURN_BOOL(0);
	}
	ins=r->ptr;
	array_init(return_value);
	add_assoc_long(return_value, "ierr", ins->ierr);
	add_assoc_long(return_value, "oerr", ins->oerr);
}
/* }}} */

/* {{{ proto string bsdconv(void)
  bsdconv error message
*/
PHP_FUNCTION(bsdconv_error){
	char *c;
	c=bsdconv_error();
	RETVAL_STRING(c, 1);
	free(c);
}
/* }}} */

/* {{{ bsdconv_functions[]
 *
 * Every user visible function must have an entry in bsdconv_functions[].
 */
zend_function_entry bsdconv_functions[] = {
	PHP_FE(bsdconv_create,	NULL)
	PHP_FE(bsdconv_destroy,	NULL)
	PHP_FE(bsdconv_info,	NULL)
	PHP_FE(bsdconv,	NULL)
	PHP_FE(bsdconv_file,	NULL)
	PHP_FE(bsdconv_error,	NULL)
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
	"3.5", /* Replace with version number for your extension */
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
