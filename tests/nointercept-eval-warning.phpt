--TEST--
Test that the extension can issue a warning on eval()
--SKIPIF--
<?php require 'skipif.inc'; ?>
--INI--
disableeval.mode = 2
disableeval.intercept_compile_string = 0
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
Warning: eval() is dangerous in %s on line %d
4
