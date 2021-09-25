--TEST--
Test that the extension can issue a loud warning on create_function()
--INI--
disableeval.mode = 3
--SKIPIF--
<?php require 'skipif-cf.inc'; ?>
--FILE--
<?php
error_reporting(error_reporting() & ~E_DEPRECATED);
$f = @create_function('', 'return 21;');
var_export($f());
?>
--EXPECTF--
Warning: create_function() is dangerous in %s on line %d
21
