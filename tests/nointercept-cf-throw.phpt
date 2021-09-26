--TEST--
Test that the extension can throw exceptions on create_function()
--SKIPIF--
<?php require 'skipif8.inc'; ?>
--INI--
disableeval.mode = 1
disableeval.intercept_compile_string = 0
--FILE--
<?php
error_reporting(error_reporting() & ~E_DEPRECATED);
$f = create_function('', 'return 21;');
var_export($f());
?>
--EXPECTF--
Fatal error: Uncaught ErrorException: create_function() is not allowed in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
