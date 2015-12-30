#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#ifndef _WIN32
  #include <arpa/inet.h>
#else
  #include <Ws2tcpip.h>
  /*#include <winsock2.h>*/
#endif

#include <modbus/modbus.h>

#ifndef _WIN32
  extern const char *__progname;
#else
  const char *__progname;
#endif

typedef enum {
  S_SHORT,
  S_LONG,
  U_SHORT,
  U_LONG,
  BINARY,
  FLOAT_T,
  ASCII
} format_type;

struct modbus_db_params {
  int starting_register;
  int num_registers;
  format_type format;
};

struct modbus_comm_params {
  char *ip_address;
  int port;
  int response_timeout;
  int rtu_address;
  int slave;
};

/* declarations */
void usage();
int is_valid_ip(char *ip_addr);
void validate_params(struct modbus_comm_params *mbcp,
                     struct modbus_db_params *mbdp);
int parse_args(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
               int argc, char **argv);
int poll(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
         uint16_t *tab_reg);

void print_results(struct modbus_db_params *mbdp,
                   uint16_t *tab_reg, int num_results);
void print_s_short(uint16_t value);
void print_s_long(uint16_t value1, uint16_t value2);
void print_u_short(uint16_t value);
void print_u_long(uint16_t value1, uint16_t value2);
void print_binary(uint16_t value);
void print_float(uint16_t value1, uint16_t value2);
void print_ascii(uint16_t value);

