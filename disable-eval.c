#include "php_disableeval.h"
#include <ext/standard/info.h>
#include <Zend/zend_exceptions.h>

ZEND_DECLARE_MODULE_GLOBALS(de);

static zend_module_entry de_module_entry;

static void throw_exception(const char* function)
{
#if PHP_VERSION_ID >= 80000
    zend_string* message = zend_strpprintf(0, "%s is not allowed", function);
    zend_throw_error_exception(zend_ce_error_exception, message, 0, E_ERROR);
#else
    char* message;
    zend_spprintf(&message, 0, "%s is not allowed", function);
    zend_throw_error_exception(zend_ce_error_exception, message, 0, E_ERROR);
    efree(message);
#endif
}

static void scream(const char* function)
{
    int old_reporting = EG(error_reporting);
    if ((old_reporting & E_WARNING) == 0) {
        EG(error_reporting) |= E_WARNING;
    }

    zend_error(E_WARNING, "%s is dangerous", function);
    EG(error_reporting) = old_reporting;
}

static void complain(const char* function)
{
    switch (DE_G(mode)) {
        case MODE_THROW:
            throw_exception(function);
            break;

        case MODE_WARN:
            zend_error(E_WARNING, "%s is dangerous", function);
            break;

        case MODE_SCREAM:
            scream(function);
            break;

        default:
            zend_error(E_ERROR, "%s is not allowed", function);
            /* NOT REACHED */
    }
}

PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("disableeval.enabled",                "1", PHP_INI_SYSTEM, OnUpdateBool, enabled,      zend_de_globals, de_globals)
#if PHP_VERSION_ID < 80000
    STD_PHP_INI_BOOLEAN("disableeval.disallow_create_func",   "1", PHP_INI_SYSTEM, OnUpdateBool, watch_cf,     zend_de_globals, de_globals)
    STD_PHP_INI_BOOLEAN("disableeval.disallow_string_assert", "1", PHP_INI_SYSTEM, OnUpdateBool, watch_assert, zend_de_globals, de_globals)
#endif
    STD_PHP_INI_ENTRY("disableeval.mode",                     "1", PHP_INI_SYSTEM, OnUpdateLong, mode,         zend_de_globals, de_globals)
PHP_INI_END()

#if PHP_VERSION_ID < 80000
static PHP_FUNCTION(create_function)
{
    complain("create_function()");
    if (!EG(exception)) {
        DE_G(orig_create_function)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

static PHP_FUNCTION(assert)
{
    zval* assertion;
    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_ZVAL(assertion)
    ZEND_PARSE_PARAMETERS_END();

    if (Z_TYPE_P(assertion) == IS_STRING) {
        complain("assert() with a string argument");
    }

    if (!EG(exception)) {
        DE_G(orig_assert)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }
}

static void patch_function(const char* name, size_t len, zif_handler* orig_handler, zif_handler new_handler)
{
    zend_function* f = zend_hash_str_find_ptr(CG(function_table), name, len);
    if (f && f->type == ZEND_INTERNAL_FUNCTION && f->internal_function.handler) {
        *orig_handler                = f->internal_function.handler;
        f->internal_function.handler = new_handler;
    }
}

static void unpatch_function(const char* name, size_t len, zif_handler orig_handler)
{
    zend_function* f = zend_hash_str_find_ptr(CG(function_table), name, len);
    if (f) {
        f->internal_function.handler = orig_handler;
    }
}
#endif

static int op_ZEND_INCLUDE_OR_EVAL(zend_execute_data* execute_data)
{
    const zend_op* opline = execute_data->opline;
    if (opline->extended_value == ZEND_EVAL && DE_G(mode) != MODE_IGNORE) {
        complain("eval()");

        if (EG(exception)) {
            if (opline->result_type & (IS_VAR | IS_TMP_VAR)) {
                ZVAL_UNDEF(EX_VAR(opline->result.var));
            }

            return ZEND_USER_OPCODE_CONTINUE;
        }
    }

    if (DE_G(prev_eval_handler)) {
        return DE_G(prev_eval_handler)(execute_data);
    }

    return ZEND_USER_OPCODE_DISPATCH;
}

static PHP_MINIT_FUNCTION(de)
{
    REGISTER_INI_ENTRIES();

    if (DE_G(enabled) && DE_G(mode) != MODE_IGNORE) {
        DE_G(prev_eval_handler) = zend_get_user_opcode_handler(ZEND_INCLUDE_OR_EVAL);
        if (zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, op_ZEND_INCLUDE_OR_EVAL) == FAILURE) {
            zend_error(E_CORE_WARNING, "Unable to install a handler for ZEND_INCLUDE_OR_EVAL");
        }

#if PHP_VERSION_ID < 80000
        if (DE_G(watch_cf)) {
            patch_function(ZEND_STRL("create_function"), &DE_G(orig_create_function), PHP_FN(create_function));
        }

        if (DE_G(watch_assert)) {
            patch_function(ZEND_STRL("assert"), &DE_G(orig_assert), PHP_FN(assert));
        }
#endif
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

#if PHP_VERSION_ID < 80000
        if (DE_G(watch_cf)) {
            unpatch_function(ZEND_STRL("create_function"), DE_G(orig_create_function));
        }

        if (DE_G(watch_assert)) {
            unpatch_function(ZEND_STRL("assert"), DE_G(orig_assert));
        }
#endif
    }

    return SUCCESS;
}

static PHP_GINIT_FUNCTION(de)
{
#if PHP_VERSION_ID < 80000
    de_globals->orig_create_function = NULL;
    de_globals->orig_assert          = NULL;
#endif
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

static const zend_module_dep deps[] = {
    ZEND_MOD_CONFLICTS_EX("Xdebug", "<", "2.9.3")
    ZEND_MOD_END
};

static zend_module_entry de_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    deps,
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
