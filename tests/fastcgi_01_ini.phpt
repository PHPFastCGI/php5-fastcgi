--TEST--
FastCGI extension ini test
--FILE--
<?php

echo ini_get('fastcgi.listen_backlog') . PHP_EOL;
echo 'Done' . PHP_EOL;

?>
--EXPECTF--
5
Done

