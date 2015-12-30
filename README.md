mbpoll
======

An application to poll Modbus registers using the ModbusTCP protocol.

Example usage:

    mbpoll -s 1 -t 3 -p 502 192.168.1.1 40001,10,u
    
    -s 1          Slave RTU address 1 (default)
    
    -t 3          A reply timeout of 3 seconds (default)
  
    -p 502        Remote TCP port 502 (default)
  
    192.168.1.1   Connect to remote IP address 192.168.1.1
  
    40001,10,u    Start polling at register 40001, poll 10 registers, display
                  as unsigned short integers.

Formats:

    u   unsigned short integer (16 bits)
    U   unsigned long integer (32 bits)
    s   signed short integer (16 bits)
    S   signed long integer (32 bits)
    f   single precision floating point (32 bits)
    b   binary string (eg 01010101 10101010) (16 bits)
    a   ASCII (eg AB) (16 bits)


Depends on libmodbus (http://libmodbus.org/).

