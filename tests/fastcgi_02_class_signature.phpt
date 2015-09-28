--TEST--
Test the FastCGIApplication class signature
--FILE--
<?php

$reflectionClass = new ReflectionClass('FastCGIApplication');

foreach ($reflectionClass->getMethods() as $method) {
	echo  "{$method->getName()} has {$method->getNumberOfParameters()} parameters of which {$method->getNumberOfRequiredParameters()} are required\n";
}

?>
--EXPECTF--
__construct has 2 parameters of which 0 are required
accept has 0 parameters of which 0 are required
finish has 0 parameters of which 0 are required
setExitStatus has 1 parameters of which 1 are required
getParam has 1 parameters of which 1 are required
getParams has 0 parameters of which 0 are required
stdinRead has 1 parameters of which 1 are required
stdinEof has 0 parameters of which 0 are required
stdoutWrite has 1 parameters of which 1 are required
stdoutEof has 0 parameters of which 0 are required

