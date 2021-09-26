--TEST--
Test that the extension can be disabled
--SKIPIF--
<?php require 'skipif.inc'; ?>
--INI--
disableeval.enabled = Off
--FILE--
<?php
echo eval('return 2+2;'), PHP_EOL;
?>
--EXPECT--
4
