#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <modbus/modbus.h>



extern const char *__progname;

typedef enum {
  INTEGER,
  LONG,
  BINARY,
  FLOAT,
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
};


// declarations
void usage();
int is_valid_ip(char *ip_addr);
void validate_params(struct modbus_comm_params *mbcp,
                     struct modbus_db_params *mbdp);
int parse_args(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
               int argc, char **argv);
int poll(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
         uint16_t *tab_reg);
void int_to_binary_string(uint16_t value);


int main(int argc, char **argv)
{
  // variable declaration
  struct modbus_comm_params mbcp;
  struct modbus_db_params mbdp;
  uint16_t *tab_reg;
  int result, idx;

  // parse the command line arguments
  parse_args(&mbcp, &mbdp, argc, argv);

  // print input variables
  printf("IP Address: %s\n", mbcp.ip_address);
  printf("Port: %d\n", mbcp.port);
  printf("Starting Register: %d\n", mbdp.starting_register);
  printf("Number of Registers To Read: %d\n", mbdp.num_registers);
  printf("Registers: %u-%u\n\n", mbdp.starting_register,
         mbdp.starting_register + mbdp.num_registers - 1);

  // allocate memory based on number of registers to be polled
  tab_reg = (uint16_t *) malloc(sizeof(uint16_t) * mbdp.num_registers);

  // poll and check that we have results
  if ((result = poll(&mbcp, &mbdp, tab_reg)) < 1) {
    fprintf(stderr, "Read 0 registers.\nError: %s\n", modbus_strerror(errno));
    free(tab_reg);
    exit(4);
  }

  printf("%d results:\n", result);

  // loop through results and print
  for(idx = 0; idx < result; idx++) {
    printf("%d: %d\n", mbdp.starting_register + idx, tab_reg[idx]);

    if (mbdp.format == BINARY) int_to_binary_string(tab_reg[idx]);
  }

  // free allocated memory
  free(tab_reg);

  return 0;
}

// print usage
void usage()
{
  printf("Usage: %s [OPTION]... IP_ADDRESS STARTING_REGISTER\n", __progname);
  puts("Poll the specified register(s) via MODBUS/TCP.");
  puts("  -h    show this usage");
  puts("  -n    number of registers to poll (default 1)");
  puts("  -p    destination port number (default 502)"); 
  puts("  -t    response timeout in seconds (default 3)\n"); 

  puts("Examples:");
  printf("  %s -n 10 -p 502 -t 5 192.168.1.5 40001\n"
         "    Poll 10 registers from 192.168.1.5 on port 502"
         " with a 5 second timeout\n"
         "    starting at register 40001.\n",
         __progname);
}

// checks to see if a char* IP address can be parsed as
// a proper IP address
int is_valid_ip(char *ip_addr)
{
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ip_addr, &(sa.sin_addr));
  return result != 0;
}

// validates input parameters
void validate_params(struct modbus_comm_params *mbcp,
                     struct modbus_db_params *mbdp)
{
  // check for a valid IP argument
  if (!is_valid_ip(mbcp->ip_address)) {
    fprintf(stderr, "Invalid IP.\n");
    exit(1);
  }

  if (mbcp->port < 1 || mbcp->port > 65535) {
    fprintf(stderr, "Invalid port.\n");
    exit(1);
  }

  if (mbdp->starting_register < 40000 || mbdp->starting_register > 49999) {
    fprintf(stderr, "Invalid starting register.\n");
    exit(1);
  }

  if (mbdp->starting_register + mbdp->num_registers - 1 > 49999) {
    fprintf(stderr, "Register high limit exceeded. Try fewer registers.\n");
    exit(1);
  }
  
  if (mbdp->num_registers < 1 || mbdp->num_registers > 125) {
    fprintf(stderr, "Invalid number of registers (max. 125).\n");
    exit(1);
  }
}

// parses command line arguments
int parse_args(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
               int argc, char **argv)
{
  int current_arg;

  //defaults
  mbcp->port = 502;
  mbcp->response_timeout = 3;
  mbdp->num_registers = 1;

  // parse flags
  while ((current_arg = getopt(argc, argv, "hn:p:t:")) != -1) {
    switch (current_arg) {
      case 'h':
        usage();
        exit(0);
   /* case 'n':
        mbdp->num_registers = atoi(optarg);
        break; */
      case 'p':
        mbcp->port = atoi(optarg);
        break;
      case 't':
        mbcp->response_timeout = atoi(optarg);
        break;
      case '?':
        if (optopt == 'n' || optopt == 'p' || optopt == 't') {
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        }
        else {
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        }
        
        exit(1);
      default:
        exit(1);
    }
  }

  // need 2 args left over after parsed flags (IP and starting register)
  if (argc - optind != 2) {
    fprintf(stderr, "Wrong number of arguments.\n");
    usage();
    exit(1);
  }

  // set IP address
  mbcp->ip_address = argv[optind];
  optind++;

  char *db_param_string;
  db_param_string = strtok(argv[optind], ",");
  if (db_param_string == NULL) {
    fprintf(stderr, "Invalid CSV string for registers.\n");
    exit(1);
  }
  mbdp->starting_register = atoi(db_param_string);

  db_param_string = strtok(NULL, ",");
  if (db_param_string == NULL) {
    fprintf(stderr, "Invalid CSV string for registers.\n");
    exit(1);
  }
  if (atoi(db_param_string) < mbdp->starting_register)
    mbdp->num_registers = atoi(db_param_string);
  else
    mbdp->num_registers = atoi(db_param_string) - mbdp->starting_register + 1;

  db_param_string = strtok(NULL, ",");
  if (db_param_string == NULL) {
    fprintf(stderr, "Invalid CSV string for registers.\n");
    exit(1);
  }

  char format_char = db_param_string[0];
  if      (format_char == 'i') mbdp->format = INTEGER;
  else if (format_char == 'l') mbdp->format = LONG;
  else if (format_char == 'f') mbdp->format = FLOAT;
  else if (format_char == 'b') mbdp->format = BINARY;
  else if (format_char == 'a') mbdp->format = ASCII;
  else {
      fprintf(stderr, "Invalid format type.\n");
      exit(1);
  }

  // validate set parameters
  validate_params(mbcp, mbdp);

  return 0;
}

// do actual modbus polling
int poll(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
         uint16_t *tab_reg)
{
  int mb_friendly_starting_reg = mbdp->starting_register - 40001;
  struct timeval response_timeout;
  int result;
  modbus_t *mb;

  // create tcp connection
  mb = modbus_new_tcp(mbcp->ip_address, mbcp->port);
  // set slave address to 1 -- usually not needed for Modbus/TCP
  modbus_set_slave(mb, 1);
  if (modbus_connect(mb) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(mb);
    exit(3);
  }

  // set specified timeout
  response_timeout.tv_sec = mbcp->response_timeout;
  modbus_set_response_timeout(mb, &response_timeout);

  // read modbus registers on active tcp connection
  // returns -1 for error, otherwise number of registers read
  result = modbus_read_registers(mb, mb_friendly_starting_reg,
                                 mbdp->num_registers, tab_reg);

  // close and free mb connection
  modbus_close(mb);
  modbus_free(mb);

  return result;
}

void int_to_binary_string(uint16_t value)
{
  int i;
  char bool_string[17];
  char *ptr = bool_string;

  bool_string[16] = '\0';

  for (i = 32768; i > 0; i >>= 1) {
    *ptr++ = (value & i) ? '1' : '0';
  }

  printf("%s\n", bool_string);
}

