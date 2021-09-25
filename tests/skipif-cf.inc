<?php
if (!extension_loaded('disableeval')) {
    die('SKIP chuid not loaded');
}

if (!function_exists('create_function')) {
    die('SKIP create_function is not supported');
}
