#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitpack.h>
#include "word_extractor.h"

uint32_t get_opcode(uint32_t word)
{
        return (uint32_t)Bitpack_getu(word, 4, 28);
}

void get_registers(uint32_t word, uint32_t *ra, uint32_t *rb, uint32_t *rc)
{
        *ra = (uint32_t)Bitpack_getu(word, 3, 6);
        *rb = (uint32_t)Bitpack_getu(word, 3, 3);
        *rc = (uint32_t)Bitpack_getu(word, 3, 0);
}

void get_val(uint32_t word, uint32_t *ra, uint32_t *value)
{
        *ra = (uint32_t)Bitpack_getu(word, 3, 25); 
        *value = (uint32_t)Bitpack_getu(word, 25, 0); 
}
