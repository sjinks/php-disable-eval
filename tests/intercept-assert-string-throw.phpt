--TEST--
Test that string assertions are not allowed
--SKIPIF--
<?php include('skipif8.inc'); ?>
--INI--
zend.assertions  = 1
assert.exception = 1
disableeval.mode = 1
disableeval.intercept_compile_string = 1
--FILE--
<?php
error_reporting(E_ALL & ~E_DEPRECATED);
assert('false');
?>
--EXPECTF--
Fatal error: Uncaught ErrorException: assert() with a string argument is not allowed in %s:%d
Stack trace:
#0 {main}

Next Error: Failure evaluating code:%w
false in %s:%d
Stack trace:
#0 %s(%d): assert('false')
#1 {main}
  thrown in %s on line %d
