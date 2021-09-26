--TEST--
Test that the extension can abort on eval()
--SKIPIF--
<?php require 'skipif.inc'; ?>
--INI--
disableeval.mode = 4
disableeval.intercept_compile_string = 1
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
Fatal error: eval() is not allowed in %s on line %d
