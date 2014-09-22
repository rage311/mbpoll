#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

void make_float(uint16_t reg1, uint16_t reg2);//uint16_t *reg_values);

int main(int argc, char **argv)
{
  uint16_t reg1 = (uint16_t)atoi(argv[1]),
           reg2 = (uint16_t)atoi(argv[2]);
  printf("reg1, reg2: %u, %u\n", reg1, reg2);

  make_float(reg1, reg2);
  return 0;
}

void make_float(uint16_t reg1, uint16_t reg2)
{
  float *result;
  int reg32 = ((reg1 << 16) | reg2);
  printf("reg32: %d\n", reg32);

  result = (float *)(&reg32);
  printf("result: %.2f\n", *result);
   
  if (reg_values == NULL) {
    fprintf(stderr, "Null pointer in make_float()\n");
    exit(EXIT_FAILURE);
  }
  
}

