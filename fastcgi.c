#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcgiapp.h>

#include "php.h"
#include "php_ini.h"
#include "php_fastcgi.h"

ZEND_DECLARE_MODULE_GLOBALS(fastcgi)

/* {{{ PHP_INI
*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("fastcgi.listen_backlog", "5", PHP_INI_ALL, OnUpdateLong, listen_backlog, zend_fastcgi_globals, fastcgi_globals)
PHP_INI_END()
/* }}} */

/* Handlers */
static zend_object_handlers fastcgi_application_class_handlers;

/* Class entries */
zend_class_entry *php_fastcgi_application_interface_entry;
zend_class_entry *php_fastcgi_application_class_entry;

static void php_fastcgi_verify_has_request(php_fastcgi_application_class_object *class_object) /* {{{ */
{
	if (!class_object->has_request) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "There is no request ready to operate on", 0 TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto void FastCGIApplication::__construct([string path [, int backlog]])
   Constructor. */
PHP_METHOD(fastcgi_application_class, __construct)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();
	char *path = NULL;
	int path_len, sock = 0;
	long backlog = FASTCGI_G(listen_backlog);

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &path_len, &backlog)) {
		RETURN_NULL();
	}

	if (path) {
		sock = FCGX_OpenSocket(path, backlog);

		if (sock < 0) {
			zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Unable to open socket: %s", path);
		}
	}

	FCGX_InitRequest(&(class_object->request), sock, 0);
	class_object->initialised = 1;
	class_object->has_request = 0;
}
/* }}} */

/* {{{ proto bool FastCGIApplication::accept()
   Accept. */
PHP_METHOD(fastcgi_application_class, accept)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	class_object->has_request = (FCGX_Accept_r(&(class_object->request)) >= 0);

	RETURN_BOOL(class_object->has_request);
}
/* }}} */

/* {{{ proto bool FastCGIApplication::finish()
   Finish. */
PHP_METHOD(fastcgi_application_class, finish)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	class_object->has_request = 0;
	FCGX_Finish_r(&(class_object->request));
}
/* }}} */

/* {{{ proto void FastCGIApplication::setExitStatus(int exitStatus)
   Set exit status. */
PHP_METHOD(fastcgi_application_class, setExitStatus)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();
	long exit_status;

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &exit_status)) {
		RETURN_NULL();
	}

	php_fastcgi_verify_has_request(class_object);

	FCGX_SetExitStatus(exit_status, (class_object->request).out);
}
/* }}} */

/* {{{ proto string FastCGIApplication::getParam(string param)
   Get param. */
PHP_METHOD(fastcgi_application_class, getParam)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();
	char *param = NULL, *value;
	int param_len;

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &param, &param_len)) {
		RETURN_NULL();
	}

	php_fastcgi_verify_has_request(class_object);

	value = FCGX_GetParam(param, (class_object->request).envp);

	if (value) {
		RETURN_STRING(value, 1);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string FastCGIApplication::getParams()
   Get params. */
PHP_METHOD(fastcgi_application_class, getParams)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();
	char **envp, *name, *value;

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	php_fastcgi_verify_has_request(class_object);

	array_init(return_value);

	envp = (class_object->request).envp;

	while (*envp) {
		name = estrdup(*envp);
		value = strchr(name, '=');

		if (value == NULL) {
			efree(name);
			continue;
		}

		*(value++) = 0;

		add_assoc_string(return_value, name, value, 1);

		efree(name);

		envp++;
	}
}
/* }}} */

/* {{{ proto string FastCGIApplication::stdinRead(int length)
   Read stdin. */
PHP_METHOD(fastcgi_application_class, stdinRead)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();
	long len, read_len;
	char *buffer, *tmp_buffer;

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &len)) {
		RETURN_NULL();
	}

	php_fastcgi_verify_has_request(class_object);

	buffer = (char *) emalloc(len);

	read_len = FCGX_GetStr(buffer, len, (class_object->request).in);

	if (read_len < len) {
		tmp_buffer = erealloc(buffer, read_len);

		if (tmp_buffer) {
			buffer = tmp_buffer;
		}
	}

	ALLOC_INIT_ZVAL(return_value);
	Z_TYPE_P(return_value) = IS_STRING;
	Z_STRVAL_P(return_value) = buffer;
	Z_STRLEN_P(return_value) = read_len;
}
/* }}} */

/* {{{ proto bool FastCGIApplication::stdinEof()
   Stdin eof. */
PHP_METHOD(fastcgi_application_class, stdinEof)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	RETURN_BOOL(FCGX_HasSeenEOF((class_object->request).in));
}
/* }}} */

/* {{{ proto int FastCGIApplication::stdoutWrite(string data)
   Write stdout. */
PHP_METHOD(fastcgi_application_class, stdoutWrite)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();
	char *data;
	int data_len;
	long len;

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len)) {
		RETURN_NULL();
	}

	php_fastcgi_verify_has_request(class_object);

	len = FCGX_PutStr(data, data_len, (class_object->request).out);

	if (len < 0) {
		RETURN_FALSE;
	}

	RETURN_LONG(len);
}

/* {{{ proto bool FastCGIApplication::stdoutEof()
   Stdout eof. */
PHP_METHOD(fastcgi_application_class, stdoutEof)
{
	php_fastcgi_application_class_object *class_object;
	zval *object = getThis();

	class_object = (php_fastcgi_application_class_object *) zend_object_store_get_object(object TSRMLS_CC);

	RETURN_BOOL(FCGX_HasSeenEOF((class_object->request).out));
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_fastcgi_application_construct, 0, 0, 0)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, backlog)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fastcgi_application_set_exit_status, 0, 0, 1)
	ZEND_ARG_INFO(0, exitStatus)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fastcgi_application_get_param, 0, 0, 1)
	ZEND_ARG_INFO(0, param)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fastcgi_application_stdin_read, 0, 0, 1)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fastcgi_application_stdout_write, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fastcgi_void, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ php_fastcgi_application_interface_methods */
static zend_function_entry php_fastcgi_application_interface_methods[] = {
	PHP_ABSTRACT_ME(fastcgi_application_interface, accept,        arginfo_fastcgi_void)
	PHP_ABSTRACT_ME(fastcgi_application_interface, finish,        arginfo_fastcgi_void)
	PHP_ABSTRACT_ME(fastcgi_application_interface, setExitStatus, arginfo_fastcgi_application_set_exit_status)
	PHP_ABSTRACT_ME(fastcgi_application_interface, getParam,      arginfo_fastcgi_application_get_param)
	PHP_ABSTRACT_ME(fastcgi_application_interface, getParams,     arginfo_fastcgi_void)
	PHP_ABSTRACT_ME(fastcgi_application_interface, stdinRead,     arginfo_fastcgi_application_stdin_read)
	PHP_ABSTRACT_ME(fastcgi_application_interface, stdinEof,      arginfo_fastcgi_void)
	PHP_ABSTRACT_ME(fastcgi_application_interface, stdoutWrite,   arginfo_fastcgi_application_stdout_write)
	PHP_ABSTRACT_ME(fastcgi_application_interface, stdoutEof,     arginfo_fastcgi_void)
	PHP_FE_END
};
/* }}} */

/* {{{ php_fastcgi_application_class_methods */
static zend_function_entry php_fastcgi_application_class_methods[] = {
	PHP_ME(fastcgi_application_class, __construct,   arginfo_fastcgi_application_construct,       ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, accept,        arginfo_fastcgi_void,                        ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, finish,        arginfo_fastcgi_void,                        ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, setExitStatus, arginfo_fastcgi_application_set_exit_status, ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, getParam,      arginfo_fastcgi_application_get_param,       ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, getParams,     arginfo_fastcgi_void,                        ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, stdinRead,     arginfo_fastcgi_application_stdin_read,      ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, stdinEof,      arginfo_fastcgi_void,                        ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, stdoutWrite,   arginfo_fastcgi_application_stdout_write,    ZEND_ACC_PUBLIC)
	PHP_ME(fastcgi_application_class, stdoutEof,     arginfo_fastcgi_void,                        ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/* }}} */

static void php_fastcgi_application_class_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	php_fastcgi_application_class_object *intern = (php_fastcgi_application_class_object *) object;

	if (!intern) {
		return;
	}

	if (intern->initialised) {
		FCGX_Free(&(intern->request), 1);
		intern->initialised = 0;
	}

	zend_object_std_dtor(&(intern->zo) TSRMLS_CC);
	efree(intern);
}
/* }}} */

static zend_object_value php_fastcgi_application_class_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	php_fastcgi_application_class_object *intern;
	zend_object_value retval;

	intern = emalloc(sizeof(php_fastcgi_application_class_object));
	intern->initialised = 0;

	zend_object_std_init(&(intern->zo), class_type TSRMLS_CC);
	object_properties_init(&(intern->zo), class_type);

	retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) php_fastcgi_application_class_free_storage, NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &fastcgi_application_class_handlers;

	return retval;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(fastcgi)
{
	zend_class_entry local_application_class_entry, local_application_interface_entry;

	/* Initialise libfcgi */
	FCGX_Init();

	/* Register FastCGIApplicationInterface */
	INIT_CLASS_ENTRY(local_application_interface_entry, "FastCGIApplicationInterface", php_fastcgi_application_interface_methods);
	php_fastcgi_application_interface_entry = zend_register_internal_interface(&local_application_interface_entry TSRMLS_CC);

	/* Register FastCGIApplication */
	INIT_CLASS_ENTRY(local_application_class_entry, "FastCGIApplication", php_fastcgi_application_class_methods);
	local_application_class_entry.create_object = php_fastcgi_application_class_new;
	php_fastcgi_application_class_entry = zend_register_internal_class(&local_application_class_entry TSRMLS_CC);
	memcpy(&fastcgi_application_class_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	fastcgi_application_class_handlers.clone_obj = NULL;

	/* Make FastCGIApplication implement FastCGIApplicationInterface */
	zend_class_implements(php_fastcgi_application_class_entry, 1, php_fastcgi_application_interface_entry);

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
