#include "php_disableeval.h"
#include <ext/standard/info.h>
#include <Zend/zend_exceptions.h>

ZEND_DECLARE_MODULE_GLOBALS(de);

static zend_module_entry de_module_entry;

static void complain(const char* function)
{
    switch (DE_G(mode)) {
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
            break;
        }

        case MODE_WARN:
            zend_error(E_WARNING, "%s() is dangerous", function);
            break;

        case MODE_SCREAM: {
            int old_reporting = EG(error_reporting);
            if ((old_reporting & E_WARNING) == 0) {
                EG(error_reporting) |= E_WARNING;
            }

            zend_error(E_WARNING, "%s() is dangerous", function);
            EG(error_reporting) = old_reporting;
            break;
        }

        default:
        case MODE_BAILOUT:
            zend_error(E_ERROR, "%s() is not allowed", function);
            break ;/* NOT REACHED */
    }
}

PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("disableeval.enabled",              "1", PHP_INI_SYSTEM, OnUpdateBool, enabled,  zend_de_globals, de_globals)
    STD_PHP_INI_BOOLEAN("disableeval.disallow_create_func", "1", PHP_INI_SYSTEM, OnUpdateBool, watch_cf, zend_de_globals, de_globals)
    STD_PHP_INI_ENTRY("disableeval.mode",                   "1", PHP_INI_SYSTEM, OnUpdateLong, mode,     zend_de_globals, de_globals)
PHP_INI_END()

static PHP_FUNCTION(create_function)
{
    complain("create_function");
    if (!EG(exception)) {
        DE_G(orig_create_function)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

static int op_ZEND_INCLUDE_OR_EVAL(zend_execute_data* execute_data)
{
    const zend_op* opline = execute_data->opline;
    if (opline->extended_value == ZEND_EVAL && DE_G(mode) != MODE_IGNORE) {
        complain("eval");

        if (EG(exception) && (opline->result_type & (IS_VAR | IS_TMP_VAR))) {
            ZVAL_UNDEF(EX_VAR(opline->result.var));
            return ZEND_USER_OPCODE_CONTINUE;
        }
    }

    if (DE_G(prev_eval_handler)) {
        return DE_G(prev_eval_handler)(execute_data);
    }

    return ZEND_USER_OPCODE_DISPATCH;
}

static void patch_create_function()
{
    zend_function* f = zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("create_function"));
    if (f && f->type == ZEND_INTERNAL_FUNCTION && f->internal_function.handler) {
        DE_G(orig_create_function)   = f->internal_function.handler;
        f->internal_function.handler = PHP_FN(create_function);
    }
}

static void unpatch_create_function()
{
    zend_function* f = zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("create_function"));
    if (f) {
        f->internal_function.handler = DE_G(orig_create_function);
    }
}

static PHP_MINIT_FUNCTION(de)
{
    REGISTER_INI_ENTRIES();

    if (DE_G(enabled) && DE_G(mode) != MODE_IGNORE) {
        DE_G(prev_eval_handler) = zend_get_user_opcode_handler(ZEND_INCLUDE_OR_EVAL);
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

    if (DE_G(enabled) && DE_G(mode) != MODE_IGNORE) {
        zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, NULL);

        if (DE_G(watch_cf) && DE_G(orig_create_function)) {
            unpatch_create_function();
        }
    }

    return SUCCESS;
}

static PHP_GINIT_FUNCTION(de)
{
    de_globals->orig_create_function = NULL;
    de_globals->prev_eval_handler    = NULL;
}

static PHP_MINFO_FUNCTION(de)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Disable Eval Module", "enabled");
    php_info_print_table_row(2, "version", PHP_DISABLEEVAL_EXTVER);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

static zend_module_entry de_module_entry = {
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
    PHP_GINIT(de),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

ZEND_GET_MODULE(de);
