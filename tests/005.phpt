--TEST--
Test that extension's normal warnings can be silenced
--INI--
disableeval.mode = 2
--SKIPIF--
<?php require 'skipif.inc'; ?>
--FILE--
<?php
echo @eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
4
