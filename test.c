#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

void make_float(uint16_t reg1, uint16_t reg2);//uint16_t *reg_values);

int main(int argc, char **argv)
{
  uint16_t reg1 = (uint16_t)argv[1],
           reg2 = (uint16_t)argv[2];
  make_float(reg1, reg2);
  return 0;
}

void make_float(uint16_t reg1, uint16_t reg2)//*reg_values)
{
  uint16_t *reg_values;
  reg_values = (uint16_t *) malloc(sizeof(uint16_t) * 2);
  reg_values[0] = reg1;
  reg_values[1] = reg2;
   
  if (reg_values == NULL) {
    fprintf(stderr, "Null pointer in make_float()\n");
    exit(EXIT_FAILURE);
  }

  //uint16_t *int_ptr = &reg_values[0];
  float result = 0.0f;
  char *char_ptr;
  char_ptr = (char *)&reg_values[1]; 

  printf("char_ptr: %u\n", (unsigned int)*char_ptr);

  //result = (reg_values[0] << 16 & reg_values[1]);  
}

