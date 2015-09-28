--TEST--
Test that FastCGIApplication implements FastCGIApplicationInterface
--FILE--
<?php

$application = new FastCGIApplication;

var_dump($application instanceof FastCGIApplicationInterface);

?>
--EXPECTF--
bool(true)

