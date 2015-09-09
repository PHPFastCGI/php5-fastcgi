# php5-fastcgi

THIS EXTENSION IS EXPERIMENTAL.

This extensions provides some of the functionality of libfcgi to PHP applications via a FastCGIApplication class.

## Installing

These instructions below were used to install the extension on a virtual machine running Ubuntu 64-bit Server 15.04.

I had a couple of issues with installing libfcgi. Firstly, you may need to add "#include &lt;stdio.h&gt;" to the top of fcgio.cpp to get it to compile. Secondly, check that ./configure is configuring the library to install to a directory that your linker actually looks in (not always the case).

```sh
sudo apt-get install php5-dev

git clone https://github.com/PHPFastCGI/php5-fastcgi.git
cd php5-fastcgi

git clone https://github.com/CopernicaMarketingSoftware/PHP-CPP.git
cd PHP-CPP
make
make install
cd ../

wget http://www.fastcgi.com/dist/fcgi.tar.gz
tar -xzvf fcgi.tar.gz
cd fcgi-2.4.1-SNAP-0311112127
./configure
make
make install
cd ../

make
make install
```

Now you must enable the extension in your PHP configuration.

