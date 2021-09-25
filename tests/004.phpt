--TEST--
Test that the extension can issue a warning on eval()
--INI--
disableeval.mode = 2
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
Warning: eval() is dangerous in %s(%d) : eval()'d code on line %d
4
