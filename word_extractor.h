#ifndef WORDEXTRACTOR_INCLUDED
#define WORDEXTRACTOR_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitpack.h>

extern uint32_t get_opcode(uint32_t word);
extern void get_registers(uint32_t word, uint32_t *ra, uint32_t *rb, uint32_t *rc);
extern void get_val(uint32_t word, uint32_t *ra, uint32_t *value);

#endif