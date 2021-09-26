--TEST--
Test module info
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php
phpinfo(INFO_MODULES);
?>
--EXPECTREGEX--
.*Disable Eval Module\s+=>\s+enabled.*
