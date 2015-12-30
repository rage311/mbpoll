#include "mbpoll.h"

int main(int argc, char **argv)
{
#ifdef _WIN32
  __progname = argv[0];
#endif

  /* variable declaration */
  struct modbus_comm_params mbcp;
  struct modbus_db_params mbdp;
  uint16_t *tab_reg;
  int result;

  /* parse the command line arguments */
  parse_args(&mbcp, &mbdp, argc, argv);

  /* print input variables */
  printf("IP Address: %s\n", mbcp.ip_address);
  printf("Port: %d\n", mbcp.port);
  printf("Slave RTU: %d\n", mbcp.slave);
  printf("Starting Register: %d\n", mbdp.starting_register);
  printf("Number of Registers To Read: %d\n", mbdp.num_registers);
  printf("Registers: %u-%u\n\n", mbdp.starting_register,
         mbdp.starting_register + mbdp.num_registers - 1);

  /* allocate memory based on number of registers to be polled */
  tab_reg = (uint16_t *) malloc(sizeof(uint16_t) * mbdp.num_registers);

  /* poll and check that we have results */
  if ((result = poll(&mbcp, &mbdp, tab_reg)) < 1) {
    fprintf(stderr, "Read 0 registers.\nError: %s\n", modbus_strerror(errno));
    free(tab_reg);
    exit(EXIT_FAILURE);
  }

  /* printf("%d results:\n", result); */
  print_results(&mbdp, tab_reg, result);

  /* free allocated memory */
  free(tab_reg);

  return 0;
}

/* print usage */
void usage()
{
  printf("Usage: %s [option]... ip_address starting_register,\n"
         "number_of_registers (or end register),format\n", __progname);
  puts("Poll the specified register(s) via MODBUS/TCP.\n");
  puts("  -h    show this usage");
  puts("  -p    destination port number (optional -- default 502)"); 
  puts("  -t    response timeout in seconds (optional -- default 3)"); 
  puts("  -s    slave RTU address (optional -- default 1)\n"); 

  puts("Formats:");
  puts("  u - unsigned short integer (16 bits)");
  puts("  U - unsigned long integer (32 bits)");
  puts("  s - signed short integer (16 bits)");
  puts("  S - signed long integer (32 bits)");
  puts("  f - float (32 bits)");
  puts("  b - binary (16 bits)");
  puts("  a - ascii characters (16 bits)\n");

  puts("Examples:");
  printf("  %s -s 1 -t 3 -p 502 192.168.1.5 40001,10,u\n"
         "    Poll 10 registers from 192.168.1.5, slave address 1,\n"
         "    on port 502 with a 3 second timeout starting at\n"
         "    register 40001. Print values as unsigned shorts.\n",
         __progname);
}

/* checks to see if a char* IP address can be parsed as */
/* a proper IP address */
int is_valid_ip(char *ip_addr)
{
  struct sockaddr_in sa;
#ifdef _WIN32
  return (inet_addr(ip_addr) != INADDR_NONE);
#else
  return inet_pton(AF_INET, ip_addr, &(sa.sin_addr));
#endif
}

/* validates input parameters */
void validate_params(struct modbus_comm_params *mbcp,
                     struct modbus_db_params *mbdp)
{
  /* check for a valid IP argument */
  if (!is_valid_ip(mbcp->ip_address)) {
    fprintf(stderr, "Invalid IP.\n");
    exit(EXIT_FAILURE);
  }

  if (mbcp->port < 1 || mbcp->port > 65535) {
    fprintf(stderr, "Invalid port.\n");
    exit(EXIT_FAILURE);
  }

  if (mbcp->slave < 1 || mbcp->slave > 65535) {
    fprintf(stderr, "Invalid slave RTU address.\n");
    exit(EXIT_FAILURE);
  }

  if (mbdp->starting_register < 40000 || mbdp->starting_register > 49999) {
    fprintf(stderr, "Invalid starting register.\n");
    exit(EXIT_FAILURE);
  }

  if (mbdp->starting_register + mbdp->num_registers - 1 > 49999) {
    fprintf(stderr, "Register high limit exceeded. Try fewer registers.\n");
    exit(EXIT_FAILURE);
  }
  
  if (mbdp->num_registers < 1 || mbdp->num_registers > 125) {
    fprintf(stderr, "Invalid number of registers (max. 125).\n");
    exit(EXIT_FAILURE);
  }
}

/* parses command line arguments */
int parse_args(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
               int argc, char **argv)
{
  int current_arg;

  /* defaults */
  mbcp->port = 502;
  mbcp->response_timeout = 3;
  mbcp->slave = 1;
  mbdp->num_registers = 1;

  /* parse flags */
  while ((current_arg = getopt(argc, argv, "hn:p:t:s:")) != -1) {
    switch (current_arg) {
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
      case 'p':
        mbcp->port = atoi(optarg);
        break;
      case 't':
        mbcp->response_timeout = atoi(optarg);
        break;
      case 's':
        mbcp->slave = atoi(optarg);
        break;
      case '?':
        if (optopt == 'n' || optopt == 'p' || optopt == 't' || optopt == 's') {
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        }
        else {
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        }
        exit(EXIT_FAILURE);
      default:
        exit(EXIT_FAILURE);
    }
  }

  /* need 2 args left over after parsed flags (IP and starting register) */
  if (argc - optind != 2) {
    fprintf(stderr, "Wrong number of arguments.\n");
    usage();
    exit(EXIT_FAILURE);
  }

  /* set IP address */
  mbcp->ip_address = argv[optind];
  optind++;

  /* register parameter csv string should look like: 40001,40002,f */
  /* or 40001,10,i */
  char *db_param_string;
  db_param_string = strtok(argv[optind], ",");
  if (db_param_string == NULL) {
    fprintf(stderr, "Invalid CSV string for registers.\n");
    exit(EXIT_FAILURE);
  }
  /* first csv parameter is starting register */
  mbdp->starting_register = atoi(db_param_string);

  db_param_string = strtok(NULL, ",");
  if (db_param_string == NULL) {
    fprintf(stderr, "Invalid CSV string for registers.\n");
    exit(EXIT_FAILURE);
  }
  /* if 2nd param is less than start reg, then it's a quantity, not a reg */
  if (atoi(db_param_string) < mbdp->starting_register) {
    mbdp->num_registers = atoi(db_param_string);
  } else {
    mbdp->num_registers = atoi(db_param_string) - mbdp->starting_register + 1;
  }

  db_param_string = strtok(NULL, ",");
  if (db_param_string == NULL) {
    fprintf(stderr, "Invalid CSV string for registers.\n");
    exit(EXIT_FAILURE);
  }

  /* need to change to accommodate unsigned short/unsigned long separately */
  switch (db_param_string[0]) {
    case 's':
      mbdp->format = S_SHORT;
      break;
    case 'S':
      mbdp->format = S_LONG;
      break;
    case 'u':
      mbdp->format = U_SHORT;
      break;
    case 'U':
      mbdp->format = U_LONG;
      break;
    case 'f':
      mbdp->format = FLOAT_T;
      break;
    case 'b':
      mbdp->format = BINARY;
      break;
    case 'a':
      mbdp->format = ASCII;
      break;
    default:
      fprintf(stderr, "Invalid format type.\n");
      exit(EXIT_FAILURE);
  }

  /* any 32-bit formats need 2 regs per value */
  if (mbdp->num_registers % 2 && (mbdp->format == FLOAT_T ||
                                  mbdp->format == S_LONG ||
                                  mbdp->format == U_LONG)) {
      mbdp->num_registers++;
  }

  /* validate set parameters */
  validate_params(mbcp, mbdp);

  return 0;
}

/* do actual modbus polling */
int poll(struct modbus_comm_params *mbcp, struct modbus_db_params *mbdp,
         uint16_t *tab_reg)
{
  int mb_friendly_starting_reg = mbdp->starting_register - 40001;
  struct timeval response_timeout;
  int result;
  modbus_t *mb;

  /* zero response_timeout struct */
  memset(&response_timeout, 0, sizeof(response_timeout));

  /* create tcp connection */
  mb = modbus_new_tcp(mbcp->ip_address, mbcp->port);
  /* set slave address to 1 -- usually not needed for Modbus/TCP */
  modbus_set_slave(mb, mbcp->slave);
  if (modbus_connect(mb) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(mb);
    exit(EXIT_FAILURE);
  }

  /* set specified timeout */
  response_timeout.tv_sec = mbcp->response_timeout;
  modbus_set_response_timeout(mb, &response_timeout);

  /* read modbus registers on active tcp connection */
  /* returns -1 for error, otherwise number of registers read */
  result = modbus_read_registers(mb, mb_friendly_starting_reg,
                                 mbdp->num_registers, tab_reg);

  /* close and free mb connection */
  modbus_close(mb);
  modbus_free(mb);

  return result;
}


void print_results(struct modbus_db_params *mbdp,
                   uint16_t *tab_reg, int num_results)
{
  int idx;

  /* loop through results and print */
  for(idx = 0; idx < num_results; idx++) {
    printf("%u:\t", mbdp->starting_register + idx);

    switch (mbdp->format) {
      case S_SHORT:
        print_s_short(tab_reg[idx]); 
        break;
      case S_LONG:
        print_s_long(tab_reg[idx], tab_reg[idx + 1]); 
        idx++;
        break;
      case U_SHORT:
        print_u_short(tab_reg[idx]);
        break;
      case U_LONG:
        print_u_long(tab_reg[idx], tab_reg[idx + 1]);
        idx++;
        break;
      case BINARY:
        print_binary(tab_reg[idx]);
        break;
      case FLOAT_T:
        print_float(tab_reg[idx], tab_reg[idx + 1]);
        idx++;
        break;
      case ASCII:
        print_ascii(tab_reg[idx]);
        break;
      default:
        break;
    }
  }
}


void print_float(uint16_t value1, uint16_t value2)
{
  float *result;
  uint32_t int32 = (value1 << 16) | value2;

  result = (float *) &int32;
  printf("%.2f\n", *result);

  result = NULL;
}

/* print register as a binary string */
void print_binary(uint16_t value)
{
  int i;
  char binary_string[18] = { '\0' };
  char *ptr = binary_string;

  for (i = 32768; i > 0; i >>= 1) {
    if (i == 128) {
      *ptr++ = ' ';
    }
    *ptr++ = (value & i) ? '1' : '0';
  }

  printf("%s\n", binary_string);

  ptr = NULL;
}

void print_s_short(uint16_t value)
{
  printf("%d\n", (int16_t)value);
}

void print_s_long(uint16_t value1, uint16_t value2)
{
  printf("%d\n", (int32_t)((value1 << 16) | value2));
}

void print_u_short(uint16_t value)
{
  printf("%u\n", value);
}

void print_u_long(uint16_t value1, uint16_t value2)
{
  printf("%u\n", (uint32_t)((value1 << 16) | value2));
}

void print_ascii(uint16_t value)
{
  char *char_ptr = (char *) &value;

  printf("%c%c\n", isprint(char_ptr[0]) ? char_ptr[0] : '?',
                   isprint(char_ptr[1]) ? char_ptr[1] : '?');

  char_ptr = NULL;
}

