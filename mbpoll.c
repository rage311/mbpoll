#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <modbus/modbus.h>


extern const char *__progname;

struct modbus_params {
  char *ip_address;
  int port;
  int starting_register;
  int num_registers;
  int response_timeout;
};


// declarations
void usage();
int is_valid_ip(char *ip_addr);
void validate_params(struct modbus_params *mbp);
int parse_args(struct modbus_params *mbp, int argc, char **argv);
int poll(struct modbus_params *mbp, uint16_t *tab_reg);


int main(int argc, char **argv) {
  // variable declaration;
  struct modbus_params mbp;
  uint16_t *tab_reg;
  int result, idx;

  // parse the command line arguments
  parse_args(&mbp, argc, argv);

  // print input variables
  printf("IP Address: %s\n", mbp.ip_address);
  printf("Port: %d\n", mbp.port);
  printf("Starting Register: %d\n", mbp.starting_register);
  printf("Number of Registers To Read: %d\n", mbp.num_registers);
  printf("Registers: %u-%u\n\n", mbp.starting_register,
         mbp.starting_register + mbp.num_registers - 1);

  // allocate memory based on number of registers to be polled
  tab_reg = (uint16_t *) malloc(sizeof(uint16_t) * mbp.num_registers);

  // poll and check that we have results
  if ((result = poll(&mbp, tab_reg)) < 1) {
    fprintf(stderr, "Read 0 registers.\nError: %s\n",
            modbus_strerror(errno));
    free(tab_reg);
    exit(4);
  }

  printf("%d results:\n", result);

  // loop through results and print
  for(idx = 0; idx < result; idx++) {
    printf("%d: %d\n", mbp.starting_register + idx, tab_reg[idx]);
  }

  // free allocated memory
  free(tab_reg);

	return 0;
}

// print usage
void usage() {
  printf("Usage: %s [OPTION]... IP_ADDRESS STARTING_REGISTER\n",
         __progname);
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
int is_valid_ip(char *ip_addr) {
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ip_addr, &(sa.sin_addr));
  return result != 0;
}

// validates input parameters
void validate_params(struct modbus_params *mbp) {
  // check for a valid IP argument
  if (!is_valid_ip(mbp->ip_address)) {
    fprintf(stderr, "Invalid IP.\n");
    exit(1);
  }

	if (mbp->port < 1 || mbp->port > 65535) {
    fprintf(stderr, "Invalid port.\n");
    exit(1);
  }

  if (mbp->starting_register < 40000 ||
      mbp->starting_register > 49999) {
    fprintf(stderr, "Invalid starting register.\n");
    exit(1);
  }

  if (mbp->starting_register + mbp->num_registers - 1 > 49999) {
    fprintf(stderr, "Register high limit exceeded."
                    " Try fewer registers.\n");
    exit(1);
  }
  
  if (mbp->num_registers < 1 || mbp->num_registers > 125) {
    fprintf(stderr, "Invalid number of registers (max. 125).\n");
    exit(1);
  }
}

// parses command line arguments
int parse_args(struct modbus_params *mbp, int argc, char **argv) {
  int current_arg;

  //defaults
  mbp->port = 502;
  mbp->num_registers = 1;
  mbp->response_timeout = 3;

  // parse flags
  while ((current_arg = getopt(argc, argv, "hn:p:t:")) != -1) {
    switch (current_arg) {
      case 'h':
        usage(argv[0]);
        exit(0);
      case 'n':
        mbp->num_registers = atoi(optarg);
        break;
      case 'p':
        mbp->port = atoi(optarg);
        break;
      case 't':
        mbp->response_timeout = atoi(optarg);
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
    usage(argv[0]);
    exit(1);
  }

  // set IP address
  mbp->ip_address = argv[optind];
  optind++;

  // set starting register
  mbp->starting_register = atoi(argv[optind]);

  // validate set parameters
  validate_params(mbp);

  return 0;
}

// do actual modbus polling
int poll(struct modbus_params *mbp, uint16_t *tab_reg) {
  int mb_friendly_starting_reg = mbp->starting_register - 40001;
  struct timeval response_timeout;
  int result;
  modbus_t *mb;

  // create tcp connection
  mb = modbus_new_tcp(mbp->ip_address, mbp->port);
  if (modbus_connect(mb) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(mb);
    exit(3);
  }

  // set a longer timeout -- the default was too short
  response_timeout.tv_sec = mbp->response_timeout;
  modbus_set_response_timeout(mb, &response_timeout);

  // read modbus registers on active tcp connection
  // returns -1 for error, otherwise number of registers read
  result = modbus_read_registers(mb, mb_friendly_starting_reg,
                                 mbp->num_registers, tab_reg);

  // close and free mb connection
  modbus_close(mb);
  modbus_free(mb);

  return result;
}

