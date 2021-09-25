#ifndef PHP_DISABLEEVAL_H
#define PHP_DISABLEEVAL_H

#include <main/php.h>
#include <main/php_ini.h>
#include <Zend/zend_extensions.h>

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

#if COMPILE_DL_DISABLEEVAL
#   define XXX_EXTENSION_ENTRY zend_extension_entry
zend_extension zend_extension_entry;
#else
DE_VISIBILITY_HIDDEN extern zend_extension de_extension_entry;
#endif

ZEND_BEGIN_MODULE_GLOBALS(de)
	zend_bool enabled;
	zend_bool watch_cf;
	zend_long mode;
	user_opcode_handler_t prev_eval_handler;
ZEND_END_MODULE_GLOBALS(de)

DE_VISIBILITY_HIDDEN extern ZEND_DECLARE_MODULE_GLOBALS(de);

#define MODE_IGNORE  0
#define MODE_THROW   1
#define MODE_WARN    2
#define MODE_SCREAM  3
#define MODE_BAILOUT 4

#endif /* PHP_DISABLEEVAL_H */
