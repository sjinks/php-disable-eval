PHP_ARG_ENABLE(disableeval, whether to enable the Disable Eval extension, [ --enable-disableeval  Enable Disable Eval])

if test "$PHP_DISABLEEVAL" = "yes"; then
	AC_DEFINE([HAVE_DISABLEEVAL], [1], [Whether Disable Eval is enabled])
	PHP_NEW_EXTENSION([disableeval], [disable-eval.c], $ext_shared)
fi
