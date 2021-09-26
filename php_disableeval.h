#ifndef PHP_DISABLEEVAL_H
#define PHP_DISABLEEVAL_H

#include <main/php.h>

#define PHP_DISABLEEVAL_EXTNAME     "disableeval"
#define PHP_DISABLEEVAL_EXTVER      "0.1"

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#ifdef ZTS
#   include "TSRM.h"
#   define DE_G(v) TSRMG(de_globals_id, zend_de_globals*, v)
#else
#	define DE_G(v) (de_globals.v)
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#	define DE_VISIBILITY_HIDDEN __attribute__((visibility("hidden")))
#else
#	define DE_VISIBILITY_HIDDEN
#endif

#define phpext_disableeval_ptr  &disableeval_module_entry

#if PHP_VERSION_ID < 80000
typedef zend_op_array* (*de_zend_compile_string)(zval*, char*);
#else
typedef zend_op_array* (*de_zend_compile_string)(zend_string*, const char*);
#endif

ZEND_BEGIN_MODULE_GLOBALS(de)
#if PHP_VERSION_ID < 80000
	zif_handler orig_create_function;
	zif_handler orig_assert;
#endif
	user_opcode_handler_t prev_eval_handler;
	de_zend_compile_string zend_compile_string;
	zend_long mode;
	zend_bool enabled;
	zend_bool aggressive;
	zend_bool intercept_compile_string;
#if PHP_VERSION_ID < 80000
	zend_bool watch_assert;
#endif
ZEND_END_MODULE_GLOBALS(de)

DE_VISIBILITY_HIDDEN extern ZEND_DECLARE_MODULE_GLOBALS(de);

#define MODE_THROW   1
#define MODE_WARN    2
#define MODE_SCREAM  3
#define MODE_BAILOUT 4

#endif /* PHP_DISABLEEVAL_H */
