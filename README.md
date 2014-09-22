mbpoll
======

An application to poll Modbus registers using the ModbusTCP protocol.

Example usage:

    mbpoll -t 5 -p 502 192.168.1.1 40001,10,u
    
    
    -t 5          A reply timeout of 5 seconds
  
    -p 502        Remote TCP port 502
  
    192.168.1.1   Connect to remote IP address 192.168.1.1
  
    40001,10,u    Start polling at register 40001, poll 10 registers, display
                  as unsigned short integers.
  

Depends on libmodbus (http://libmodbus.org/).

