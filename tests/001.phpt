--TEST--
Test that the extension can throw exceptions on eval()
--INI--
disableeval.mode = 1
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
Fatal error: Uncaught ErrorException: eval() is not allowed in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
