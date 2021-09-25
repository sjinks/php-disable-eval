--TEST--
Test that exceptions thrown by the extension can be caught
--INI--
disableeval.mode = 1
--SKIPIF--
<?php require 'skipif.inc'; ?>
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
