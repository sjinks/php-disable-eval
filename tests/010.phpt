--TEST--
Test that the extension can ignore create_function()
--INI--
disableeval.mode = 0
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
error_reporting(error_reporting() & ~E_DEPRECATED);
$f = create_function('', 'return 21;');
var_export($f());
?>
--EXPECTF--
21
