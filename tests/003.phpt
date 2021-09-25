--TEST--
Test that the extension can ignore eval()
--INI--
disableeval.mode = 0
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECT--
4
