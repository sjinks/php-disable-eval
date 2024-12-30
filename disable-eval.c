#include "php_disableeval.h"
#include <ext/standard/info.h>
#include <Zend/zend_exceptions.h>

ZEND_DECLARE_MODULE_GLOBALS(de);

static zend_module_entry de_module_entry;

static void throw_exception(const char* function)
{
    zend_string* message = zend_strpprintf(0, "%s is not allowed", function);
    zend_throw_error_exception(zend_ce_error_exception, message, 0, E_ERROR);
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
    STD_PHP_INI_BOOLEAN("disableeval.enabled",                  "1", PHP_INI_SYSTEM, OnUpdateBool, enabled,                  zend_de_globals, de_globals)
    STD_PHP_INI_BOOLEAN("disableeval.aggressive",               "0", PHP_INI_SYSTEM, OnUpdateBool, aggressive,               zend_de_globals, de_globals)
    STD_PHP_INI_BOOLEAN("disableeval.intercept_compile_string", "1", PHP_INI_SYSTEM, OnUpdateBool, intercept_compile_string, zend_de_globals, de_globals)
    STD_PHP_INI_ENTRY("disableeval.mode",                       "1", PHP_INI_SYSTEM, OnUpdateLong, mode,                     zend_de_globals, de_globals)
PHP_INI_END()

static int op_ZEND_INCLUDE_OR_EVAL(zend_execute_data* execute_data)
{
    const zend_op* opline = execute_data->opline;
    if (opline->extended_value == ZEND_EVAL) {
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

typedef struct eval_pattern {
    const char* pattern;
    size_t pattern_len;
    const char* api;
    zend_bool* check;
} eval_pattern_t;

static zend_op_array* zend_compile_string_override(
    zend_string* source_string,
    const char* filename
#if PHP_VERSION_ID >= 80200
    , zend_compile_position position
#endif
)
{
#ifndef ZTS
    static
#endif
    const eval_pattern_t patterns[] = {
        { ZEND_STRL(" : eval()'d code"), "eval()", &DE_G(enabled) },
        { NULL, 0, NULL, NULL }
    };

    if (DE_G(aggressive)) {
        complain("code evaluation");
        return NULL;
    }

    if (filename) {
        size_t len = strlen(filename);
        const eval_pattern_t* p = patterns;
        while (p->pattern) {
            if (*(p->check) && p->pattern_len < len && !strcmp(p->pattern, filename + len - p->pattern_len)) {
                complain(p->api);
                if (EG(exception)) {
                    return NULL;
                }

                break;
            }

            ++p;
        }
    }

    return DE_G(zend_compile_string)(
        source_string,
        filename
#if PHP_VERSION_ID >= 80200
        , position
#endif
    );
}

static PHP_MINIT_FUNCTION(de)
{
    REGISTER_INI_ENTRIES();

    if (DE_G(enabled)) {
        DE_G(prev_eval_handler) = zend_get_user_opcode_handler(ZEND_INCLUDE_OR_EVAL);
        if (zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, op_ZEND_INCLUDE_OR_EVAL) == FAILURE) {
            zend_error(E_CORE_WARNING, "Unable to install a handler for ZEND_INCLUDE_OR_EVAL");
        }

        if (DE_G(intercept_compile_string)) {
            DE_G(zend_compile_string) = zend_compile_string;
            zend_compile_string       = zend_compile_string_override;
        }
    }

    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_THROW",   MODE_THROW,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_WARN",    MODE_WARN,    CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_SCREAM",  MODE_SCREAM,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("DISABLEEVAL_MODE_BAILOUT", MODE_BAILOUT, CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(de)
{
    UNREGISTER_INI_ENTRIES();

    if (DE_G(enabled)) {
        zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, NULL);

        if (DE_G(intercept_compile_string)) {
            zend_compile_string = DE_G(zend_compile_string);
        }
    }

    return SUCCESS;
}

static PHP_GINIT_FUNCTION(de)
{
    de_globals->zend_compile_string  = NULL;
    de_globals->prev_eval_handler    = NULL;
}

static PHP_MINFO_FUNCTION(de)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Disable eval() Module", "enabled");
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
