PHP_ARG_ENABLE(fastcgi, whether to enable the FastCGI extension,
[  --enable-fastcgi Enable the FastCGI extension])

PHP_ARG_WITH(libfcgi, libfcgi,
[  --with-libfcgi[=DIR] FASTCGI: use libfcgi], no, no)

if test "$PHP_FASTCGI" = "yes"; then
    if test "$PHP_LIBFCGI" != "no"; then
        if test -r $PHP_LIBFCGI/include/fcgiapp.h; then
            LIBFCGI_CFLAGS="-I$PHP_LIBFCGI/include"
            LIBFCGI_LIBDIR="$PHP_LIBFCGI/libfcgi/.libs"
            AC_MSG_RESULT(from option: found in $PHP_LIBFCGI)
        fi
    else
        AC_MSG_CHECKING(for libfcgi)
        for i in /usr/local /usr; do
            if test -r $i/include/fcgiapp.h; then
                LIBFCGI_CFLAGS="-I$i/include"
                LIBFCGI_LIBDIR="$i/$PHP_LIBDIR"
                AC_MSG_RESULT(in default path: found in $i)
                break
            fi
        done
    fi

    if test -z "$LIBFCGI_LIBDIR"; then
        AC_MSG_RESULT(not found)
        AC_MSG_ERROR(Please install libfcgi-dev)
    fi

    PHP_CHECK_LIBRARY(fcgi, FCGX_Accept,
    [
        PHP_ADD_LIBRARY_WITH_PATH(fcgi, $LIBFCGI_LIBDIR, FASTCGI_SHARED_LIBADD)
    ], [
        AC_MSG_ERROR(could not find usable libfcgi)
    ], [
        -L$LIBFCGI_LIBDIR
    ])

    AC_DEFINE(HAVE_FASTCGI, 1, [Is the FastCGI extension enabled])
	PHP_NEW_EXTENSION(fastcgi, fastcgi.c, $ext_shared, , $LIBFCGI_CFLAGS)
    PHP_SUBST(FASTCGI_SHARED_LIBADD)
fi

