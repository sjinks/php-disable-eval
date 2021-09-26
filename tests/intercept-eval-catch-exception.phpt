--TEST--
Test that exceptions thrown by the extension can be caught
--SKIPIF--
<?php require 'skipif.inc'; ?>
--INI--
disableeval.mode = 1
disableeval.intercept_compile_string = 1
--FILE--
<?php
try {
    echo eval('return 2+2;'), PHP_EOL;
} catch (ErrorException $e) {
    echo "Exception aa gaya!", PHP_EOL;
}
echo "CONTINUE", PHP_EOL;
?>
--EXPECT--
Exception aa gaya!
CONTINUE
