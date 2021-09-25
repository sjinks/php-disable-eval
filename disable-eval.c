#include "php_disableeval.h"
#include <ext/standard/info.h>
#include <Zend/zend_exceptions.h>
// #include <Zend/zend_vm_execute.h>

ZEND_DECLARE_MODULE_GLOBALS(de);

zend_module_entry de_module_entry;
static zif_handler orig_create_function;

static int complain(const char* function)
{
    switch (DE_G(mode)) {
        case MODE_IGNORE:
            return 0;

        case MODE_THROW: {
#if PHP_VERSION_ID >= 80000
            zend_string* message = zend_strpprintf(0, "%s() is not allowed", function);
            zend_throw_error_exception(zend_ce_error_exception, message, 0, E_ERROR);
#else
            char* message;
            zend_spprintf(&message, 0, "%s() is not allowed", function);
            zend_throw_error_exception(zend_ce_error_exception, message, 0, E_ERROR);
            efree(message);
#endif
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

static int op_ZEND_INCLUDE_OR_EVAL(zend_execute_data* execute_data)
{
    if (execute_data->opline->extended_value == ZEND_EVAL && DE_G(mode) != MODE_IGNORE) {
        if (execute_data->opline->result_type & (IS_VAR | IS_TMP_VAR)) {
            ZVAL_UNDEF(EX_VAR(execute_data->opline->result.var));
        }

        complain("eval");

        if (EG(exception)) {
            return ZEND_USER_OPCODE_CONTINUE;
        }
    }

    return ZEND_USER_OPCODE_DISPATCH;
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
    REGISTER_INI_ENTRIES();

    if (DE_G(enabled)) {
        if (zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, op_ZEND_INCLUDE_OR_EVAL) == FAILURE) {
            zend_error(E_CORE_WARNING, "Unable to install a handler for ZEND_INCLUDE_OR_EVAL");
        }

        if (DE_G(watch_cf)) {
            patch_create_function();
        }
    }

    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_IGNORE",  MODE_IGNORE,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_THROW",   MODE_THROW,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_WARN",    MODE_WARN,    CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_SCREAM",  MODE_SCREAM,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_BAILOUT", MODE_BAILOUT, CONST_CS | CONST_PERSISTENT);

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
