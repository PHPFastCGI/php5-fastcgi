#ifndef PHP_FASTCGI_H
#define PHP_FASTCGI_H 1

#define PHP_FASTCGI_VERSION "dev"
#define PHP_FASTCGI_EXTNAME "fastcgi"

extern zend_module_entry fastcgi_module_entry;
#define phpext_fastcgi_ptr &fastcgi_module_entry

ZEND_BEGIN_MODULE_GLOBALS(fastcgi)
    long listen_backlog;
ZEND_END_MODULE_GLOBALS(fastcgi)

#ifdef ZTS
# define FASTCGI_G(v) TSRMG(fastcgi_globals_id, zend_fastcgi_globals *, v)
#else
# define FASTCGI_G(v) (fastcgi_globals.v)
#endif

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw4 ts=4
 */

