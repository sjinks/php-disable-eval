<?php
if (!extension_loaded('disableeval')) {
    die('SKIP chuid not loaded');
}

if (PHP_VERSION_ID >= 80000) {
    die('SKIP this test is for PHP 7');
}
