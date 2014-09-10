mbpoll
======

Modbus polling application

An application to poll Modbus registers using the ModbusTCP protocol.

Example usage:
  mbpoll -t 5 -p 502 -n 10 192.168.1.1 40001
  
  -t 5          A reply timeout of 5 seconds
  -p 502        Remote TCP port to connect to
  -n 10         Number of contiguous registers to poll
  192.168.1.1   Remote IP address to connect to
  40001         Starting register of the polling block

Depends on libmodbus (http://libmodbus.org/).

