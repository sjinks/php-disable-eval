--TEST--
Test that the extension ignores include/require calls
--INI--
disableeval.mode = 1
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
require __DIR__ . '/f1.inc';
include __DIR__ . '/f2.inc';
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
F1
F2

Fatal error: Uncaught ErrorException: eval() is not allowed in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d
