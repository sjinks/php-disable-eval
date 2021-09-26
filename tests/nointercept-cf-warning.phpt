--TEST--
Test that the extension can issue a warning on create_function()
--SKIPIF--
<?php require 'skipif8.inc'; ?>
--INI--
disableeval.mode = 2
disableeval.intercept_compile_string = 0
--FILE--
<?php
error_reporting(error_reporting() & ~E_DEPRECATED);
$f = create_function('', 'return 21;');
var_export($f());
?>
--EXPECTF--

Warning: create_function() is dangerous in %s on line %d
21
