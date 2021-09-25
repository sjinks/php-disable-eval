--TEST--
Test that the extension can abort on eval()
--INI--
disableeval.mode = 4
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
Fatal error: eval() is not allowed in %s on line %d
