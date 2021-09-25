#include "php_disableeval.h"
#include <ext/standard/info.h>
#include <Zend/zend_exceptions.h>

ZEND_DECLARE_MODULE_GLOBALS(de);

zend_module_entry de_module_entry;
static int zext_loaded = 0;
static int module_loaded = 0;
static zif_handler orig_create_function;

static int complain(const char* function)
{
    switch (DE_G(mode)) {
        case MODE_IGNORE:
            return 0;

        case MODE_THROW: {
            char* message;
            zend_spprintf(&message, 0, "%s() is not allowed", function);
            zend_throw_error_exception(zend_ce_error_exception, message, 0, E_ERROR);
            efree(message);
            return 1;
        }

        case MODE_WARN:
            zend_error(E_WARNING, "%s() is dangerous", function);
            return 0;

        case MODE_SCREAM: {
            int old_reporting = EG(error_reporting);
            if ((old_reporting & E_WARNING) == 0) {
                EG(error_reporting) |= E_WARNING;
            }

            zend_error(E_WARNING, "%s() is dangerous", function);
            EG(error_reporting) = old_reporting;
            return 0;
        }

        default:
        case MODE_BAILOUT:
            zend_error(E_ERROR, "%s() is not allowed", function);
            return 1; /* NOT REACHED */
    }
}

static int de_zend_startup(zend_extension* extension)
{
    zext_loaded = 1;

    if (!module_loaded) {
        return zend_startup_module(&de_module_entry);
    }

    return SUCCESS;
}

static void de_statement_handler(zend_execute_data* frame)
{
    if (DE_G(enabled) && frame->func->type == ZEND_EVAL_CODE) {
        complain("eval");
    }
}

#if COMPILE_DL_DISABLEEVAL
ZEND_DLEXPORT
#endif
zend_extension XXX_EXTENSION_ENTRY = {
    PHP_DISABLEEVAL_EXTNAME,
    PHP_DISABLEEVAL_EXTVER,
    PHP_DISABLEEVAL_AUTHOR,
    PHP_DISABLEEVAL_URL,
    PHP_DISABLEEVAL_COPYRIGHT,

    de_zend_startup,        /* Startup */
    NULL,                   /* Shutdown */
    NULL,                   /* Activate */
    NULL,                   /* Deactivate */

    NULL,                   /* Message handler */
    NULL,                   /* Op Array Handler */

    de_statement_handler,   /* Statement handler */
    NULL,                   /* fcall begin handler */
    NULL,                   /* fcall end handler */

    NULL,                   /* Op Array Constructor */
    NULL,                   /* Op Array Destructor */

    STANDARD_ZEND_EXTENSION_PROPERTIES
};

#if COMPILE_DL_DISABLEEVAL
#ifndef ZEND_EXT_API
#   define ZEND_EXT_API ZEND_DLEXPORT
#endif
ZEND_EXTENSION();
#endif

PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("disableeval.enabled",              "1", PHP_INI_SYSTEM, OnUpdateBool, enabled,  zend_de_globals, de_globals)
    STD_PHP_INI_BOOLEAN("disableeval.disallow_create_func", "1", PHP_INI_SYSTEM, OnUpdateBool, watch_cf, zend_de_globals, de_globals)
    STD_PHP_INI_ENTRY("disableeval.mode",                   "1", PHP_INI_SYSTEM, OnUpdateLong, mode,     zend_de_globals, de_globals)
PHP_INI_END()

static PHP_FUNCTION(create_function)
{
    if (complain("create_function")) {
        RETURN_NULL();
    }

    (orig_create_function)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static void patch_create_function()
{
    zend_function* f = zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("create_function"));
    if (f && f->type == ZEND_INTERNAL_FUNCTION && f->internal_function.handler) {
        orig_create_function = f->internal_function.handler;
        f->internal_function.handler = PHP_FN(create_function);
    }
}

static PHP_MINIT_FUNCTION(de)
{
    module_loaded = 1;

    REGISTER_INI_ENTRIES();

    if (DE_G(enabled)) {
        CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
        if (DE_G(watch_cf)) {
            patch_create_function();
        }
    }

    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_IGNORE",  MODE_IGNORE,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_THROW",   MODE_THROW,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_WARN",    MODE_WARN,    CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_SCREAM",  MODE_SCREAM,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_BAILOUT", MODE_BAILOUT, CONST_CS | CONST_PERSISTENT);


    if (!zext_loaded) {
        return zend_load_extension_handle(de_module_entry.handle, de_module_entry.name);
    }

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(de)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

static PHP_MINFO_FUNCTION(de)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Disable Eval Module", "enabled");
    php_info_print_table_row(2, "version", PHP_DISABLEEVAL_EXTVER);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

zend_module_entry de_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_DISABLEEVAL_EXTNAME,
    NULL,
    PHP_MINIT(de),
    PHP_MSHUTDOWN(de),
    NULL,
    NULL,
    PHP_MINFO(de),
    PHP_DISABLEEVAL_EXTVER,
    PHP_MODULE_GLOBALS(de),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

ZEND_GET_MODULE(de);
