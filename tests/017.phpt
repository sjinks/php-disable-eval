--TEST--
Test that non-string assertions are allowed
--SKIPIF--
<?php include('skipif8.inc'); ?>
--INI--
zend.assertions  = 1
assert.exception = 1
disableeval.mode = 1
--FILE--
<?php
assert(false);
?>
--EXPECTF--
Fatal error: Uncaught AssertionError: assert(false) in %s:%d
Stack trace:
#0 %s(%d): assert(false, 'assert(false)')
#1 {main}
  thrown in %s on line %d
