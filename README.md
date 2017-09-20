# hcsd README

This daemon is used to monitor a serial port for messages from the Circuit Cellar HCS II Home Automation system.

# LICENSE
[GNU GPLv2](https://github.com/linuxha/hcsd/blob/master/LICENSE)

# ToDo
* Add IPv6 bindings
* add support for a TCP connection instead of a serial port (for use with terminal servers)
* Support for a binary socket (raw, no translation from the HCS II to the connecting program)
* Support for a 'cooked' socket (binary -> hex/ASCII, where each message ends with a newline)

# Author
Neil Cherry ncherry@linuxha.com
