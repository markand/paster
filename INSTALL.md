paster INSTALL
==============

Installation instructions.

Requirements
------------

- [kcgi][], minimal CGI/FastCGI library for C,
- [sqlite][], most used database in the world,
- [curl][], (Optional) only for `paster(8)` client.

Basic installation
------------------

Quick install.

	$ tar xvzf paster-x.y.z-tar.xz
	$ cd paster-x.y.z
	$ make
	# sudo make install

To only install the web application:

	$ make pasterd
	# make install-pasterd

To only install the client:

	$ make paster
	# make install-paster

[curl]: https://curl.haxx.se
[kcgi]: https://kristaps.bsd.lv/kcgi
[sqlite]: https://www.sqlite.org
