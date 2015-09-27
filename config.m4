PHP_ARG_ENABLE(fastcgi, whether to enable the FastCGI extension,
[  --enable-fastcgi	Enable the FastCGI extension])

if test "$PHP_FASTCGI" = "yes"; then
	AC_DEFINE(HAVE_FASTCGI, 1, [Is the FastCGI extension enabled])
	PHP_NEW_EXTENSION(fastcgi, fastcgi.c, $ext_shared)
fi

