--TEST--
Test that the extension can throw exceptions on create_function()
--INI--
disableeval.mode = 1
--SKIPIF--
<?php require 'skipif-cf.inc'; ?>
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
