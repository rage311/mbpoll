mbpoll
======

An application to poll Modbus registers using the ModbusTCP protocol.

Example usage:

    mbpoll -t 5 -p 502 -n 10 192.168.1.1 40001
    
    
    -t 5          A reply timeout of 5 seconds
  
    -p 502        Remote TCP port 502
  
    -n 10         Poll 10 contiguous registers
  
    192.168.1.1   Connect to remote IP address 192.168.1.1
  
    40001         Start polling at register 40001
  

Depends on libmodbus (http://libmodbus.org/).

