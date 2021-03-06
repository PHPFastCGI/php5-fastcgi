# php5-fastcgi

THIS EXTENSION IS EXPERIMENTAL.

This extensions provides some of the functionality of libfcgi to PHP applications via a FastCGIApplication class.

## Installing

These instructions below were used to install the extension on a virtual machine running Ubuntu 64-bit Server 15.04.

```sh
sudo apt-get install php5-dev libfcgi libfcgi-dev

git clone https://github.com/PHPFastCGI/php5-fastcgi.git
cd php5-fastcgi

phpize
./configure
make
make install
```

Now you must enable the extension in your PHP configuration.

## API

Currently, the extension provides a single interface and an implementation for it. The signatures for these are below:

```php
interface FastCGIApplicationInterface
{
    public function accept();
    public function finish();
    public function setExitStatus($exitStatus);
    public function getParam($name);
    public function getParams();
    public function stdinRead($length);
    public function stdinEof();
    public function stdoutWrite($data);
    public function stdoutEof();
}

class FastCGIApplication implements FastCGIApplicationInterface
{
    public function __construct($path = null, $backlog = 5);
    public function accept();
    public function finish();
    public function setExitStatus($exitStatus);
    public function getParam($name);
    public function getParams();
    public function stdinRead($length);
    public function stdinEof();
    public function stdoutWrite($data);
    public function stdoutEof();
}
```

