--TEST--
Test that extension's normal warnings can be silenced
--SKIPIF--
<?php require 'skipif.inc'; ?>
--INI--
disableeval.mode = 2
disableeval.intercept_compile_string = 0
--FILE--
<?php
echo @eval('return 2+2;'), PHP_EOL;
?>
--EXPECTF--
4
