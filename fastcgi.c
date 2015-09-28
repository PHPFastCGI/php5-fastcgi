#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_fastcgi.h"

#include <fcgiapp.h>

ZEND_DECLARE_MODULE_GLOBALS(fastcgi)

/* {{{ PHP_INI
*/
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("fastcgi.listen_backlog", "5", PHP_INI_ALL, OnUpdateLong, listen_backlog, zend_fastcgi_globals, fastcgi_globals)
PHP_INI_END()
/* }}} */

/* Handlers */
static zend_object_handlers fastcgi_application_handlers;

/* Class entries */
zend_class_entry *php_fastcgi_application_entry;

/* {{{ proto void FastCGIApplication::__construct(string param [, int Other Param [, string Other Other Param]])
   Constructor description. */
PHP_METHOD(fastcgi_application, __construct)
{
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_fastcgi_application_construct, 0)
    ZEND_ARG_INFO(0, param1)
    ZEND_ARG_INFO(0, param2)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ php_fastcgi_application_class_methods */
static zend_function_entry php_fastcgi_application_class_methods[] = {
    PHP_ME(fastcgi_application, __construct, arginfo_fastcgi_application_construct, ZEND_ACC_PUBLIC)
};
/* }}} */

static zend_object_value php_fastcgi_application_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
    zend_object_value retval;

    return retval;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(fastcgi)
{
    zend_class_entry ce;

#if defined(ZTS)
    /* TODO: Check thread safe */
#endif

    memcpy(&fastcgi_application_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    INIT_CLASS_ENTRY(ce, "FastCGIApplication", php_fastcgi_application_class_methods);
    ce.create_object = php_fastcgi_application_new;
    fastcgi_application_handlers.clone_obj = NULL;

    php_fastcgi_application_entry = zend_register_internal_class(&ce TSRMLS_CC);

    REGISTER_INI_ENTRIES();

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(fastcgi)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(fastcgi)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "FastCGI support", "enabled");
    php_info_print_table_row(2, "FastCGI module version", PHP_FASTCGI_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(fastcgi)
{
    memset(fastcgi_globals, 0, sizeof(*fastcgi_globals));
}
/* }}} */

/* {{{ fastcgi_module_entry
*/
zend_module_entry fastcgi_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
#endif
    STANDARD_MODULE_HEADER,
    PHP_FASTCGI_EXTNAME,
    NULL,
    PHP_MINIT(fastcgi),
    PHP_MSHUTDOWN(fastcgi),
    NULL,
    NULL,
    PHP_MINFO(fastcgi),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_FASTCGI_VERSION,
#endif
    PHP_MODULE_GLOBALS(fastcgi),
    PHP_GINIT(fastcgi),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_FASTCGI
ZEND_GET_MODULE(fastcgi)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw4 ts=4
 */

