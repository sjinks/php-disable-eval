--TEST--
Test that the extension can throw exceptions on eval()
--SKIPIF--
<?php require 'skipif.inc'; ?>
--INI--
disableeval.mode = 1
disableeval.intercept_compile_string = 1
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
Fatal error: Uncaught ErrorException: eval() is not allowed in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
