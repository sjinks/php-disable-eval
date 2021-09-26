--TEST--
Test that the extension can ignore create_function
--SKIPIF--
<?php require 'skipif8.inc'; ?>
--INI--
disableeval.disallow_create_func = Off
--FILE--
<?php
error_reporting(error_reporting() & ~E_DEPRECATED);
$f = create_function('', 'return 21;');
echo $f(), PHP_EOL;
?>
--EXPECT--
21
