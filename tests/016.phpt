--TEST--
Test that string assertions are not allowed
--SKIPIF--
<?php include('skipif8.inc'); ?>
--INI--
zend.assertions  = 1
assert.exception = 1
disableeval.mode = 1
--FILE--
<?php
assert('false');
?>
--EXPECTF--
Fatal error: Uncaught ErrorException: assert() with a string argument is not allowed in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
