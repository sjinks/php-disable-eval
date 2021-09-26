--TEST--
Test that the extension can abort on create_function()
--SKIPIF--
<?php require 'skipif8.inc'; ?>
--INI--
disableeval.mode = 4
disableeval.intercept_compile_string = 1
--FILE--
<?php
error_reporting(error_reporting() & ~E_DEPRECATED);
$f = create_function('', 'return 21;');
var_export($f());
?>
--EXPECTF--
Fatal error: create_function() is not allowed in %s on line %d
