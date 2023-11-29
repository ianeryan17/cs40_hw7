/*
        um_driver.c
        Ian Ryan (iryan01) and Caden Chang (cchang15)
        10/20/23
        HW#6
        Summary: file contains driver function that runs the program and 
                 functions that execute each individual command from the 
                 program with the help of functions from segments.c
                 
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "um_driver.h"

static void execute_command(struct Segments seg_memory, uint32_t program,
                            uint32_t *registers, uint32_t *program_counter,
                            uint32_t program_length);
static inline void conditional_move(uint32_t *registers, uint32_t ra, 
                                    uint32_t rb, uint32_t rc);
static inline void segmented_load(struct Segments seg_memory, 
                                  uint32_t *registers, uint32_t ra, 
                                  uint32_t rb, uint32_t rc);
static inline void segmented_store(struct Segments seg_memory, 
                                   uint32_t *registers, uint32_t ra,
                                   uint32_t rb, uint32_t rc);
static inline void addition(uint32_t *registers, uint32_t ra, uint32_t rb,
                            uint32_t rc);
static inline void multiplication(uint32_t *registers, uint32_t ra,
                                  uint32_t rb, uint32_t rc);
static inline void division(uint32_t *registers, uint32_t ra, uint32_t rb,
                            uint32_t rc);
static inline void bitwise_nand(uint32_t *registers, uint32_t ra, uint32_t rb,
                                uint32_t rc);
static inline void halt(uint32_t *program_counter, uint32_t program_length);
static inline void map_seg(struct Segments seg_memory, uint32_t *registers,
                           uint32_t rb, uint32_t rc);
static inline void unmap_seg(struct Segments seg_memory, uint32_t *registers,
                             uint32_t rc);
static inline void output(uint32_t *registers, uint32_t rc);
static inline void input(uint32_t *registers, uint32_t rc);
static inline void load_program(struct Segments seg_memory, 
                                uint32_t *registers, uint32_t rb, uint32_t rc,
                                uint32_t *program_counter);
static inline void load_val(uint32_t *registers, uint32_t ra, uint32_t val);

static inline uint32_t get_opcode(uint32_t word);
static inline void get_registers(uint32_t word, uint32_t *ra, uint32_t *rb, 
                                 uint32_t *rc);
static inline void get_val(uint32_t word, uint32_t *ra, uint32_t *value);

/********** run ********
 * Purpose: receives a UArray_T that represents the program of the um and
            executes each word by calling execute_program
 * Inputs:  UArray_T program: array representing the words of the program       
 * Return:  void
 * Expects: program to be a properly initialized UArray 
 ************************/
void run(UArray_T program)
{
        uint32_t program_counter = 0;
        struct Segments seg_memory = initialize();
        update_zero_seg(seg_memory, program);
        uint32_t *registers = (uint32_t *) calloc(8, sizeof(uint32_t));
        
        /* runs until end of zero segment in case of no halt command */
        while (program_counter < 
               (uint32_t)UArray_length(get_segment(seg_memory, 0))) {
                uint32_t cur_command = 
                                word_load(seg_memory, 0, program_counter);
                execute_command(seg_memory, cur_command, registers,
                                &program_counter, 
                                (uint32_t)UArray_length(
                                get_segment(seg_memory, 0)));
        }

        free(registers);
        free_all_segments(seg_memory);
}

/********** execute_command ********
 * Purpose: receives all relevant runtime information about the UM including
            the current word to be executed and executes it by calling helper
            functions
 * Inputs:  Segments seg_memory: instance of segmented memory struct
            uint32_t program: 32-bit words representing the command to be 
                              executed
            uint32_t *registers: pointer to the registers array
            uint32_t *program_counter: reference to program_counter
            uint32_t program_length: represents the length of the program
 * Return:  void
 * Expects: seg_memory to be properly initialized 
 ************************/
static void execute_command(struct Segments seg_memory, uint32_t program,
                            uint32_t *registers, uint32_t *program_counter,
                            uint32_t program_length)
{
        uint32_t opcode = get_opcode(program);
        uint32_t ra, rb, rc, value;
        if (opcode == 13){
                get_val(program, &ra, &value);
                assert(ra < 8);
        } else {
                get_registers(program, &ra, &rb, &rc);
                assert(ra < 8);
                assert(rb < 8);
                assert(rc < 8);
        }

        (*program_counter)++;
        assert(opcode <= 13);

        switch(opcode) {
                case 0:
                        conditional_move(registers, ra, rb, rc);
                        break;
                case 1:
                        segmented_load(seg_memory, registers, ra, rb, rc);
                        break;
                case 2:
                        segmented_store(seg_memory, registers, ra, rb, rc);
                        break;
                case 3:
                        addition(registers, ra, rb, rc);
                        break;
                case 4:
                        multiplication(registers, ra, rb, rc);
                        break;
                case 5:
                        division(registers, ra, rb, rc);
                        break;
                case 6:
                        bitwise_nand(registers, ra, rb, rc);
                        break;
                case 7:
                        halt(program_counter, program_length);
                        break;
                case 8:
                        map_seg(seg_memory, registers, rb, rc);
                        break;
                case 9:
                        unmap_seg(seg_memory, registers, rc);
                        break;
                case 10:
                        output(registers, rc);
                        break;
                case 11:
                        input(registers, rc);
                        break;
                case 12:
                        load_program(seg_memory, registers, rb, rc, 
                                     program_counter);
                        break;
                case 13:
                        load_val(registers, ra, value);
                        break;
        }
}

/********** conditional_move ********
 * Purpose: sets value of register a to register b if register c is not 0
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers to be properly initialized 
 ************************/
static inline void conditional_move(uint32_t *registers, uint32_t ra, 
                                    uint32_t rb, uint32_t rc)
{
        if (registers[rc] != 0){
                uint32_t rb_val = registers[rb];
                registers[ra] = rb_val;
        }
}

/********** segmented_load ********
 * Purpose: sets value of r[a] to the word located at segment r[b], 
            offset r[c]
 * Inputs:  Segments seg_memory: reference to segmented memory
            uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers to be properly initialized 
 ************************/
static inline void segmented_load(struct Segments seg_memory,
                                  uint32_t *registers, uint32_t ra,
                                  uint32_t rb, uint32_t rc)
{
        uint32_t id = registers[rb];
        uint32_t offset = registers[rc];
        uint32_t loaded_word = word_load(seg_memory, id, offset);
        registers[ra] = loaded_word;
}

/********** segmented_store ********
 * Purpose: sets word located at segment r[b] and offset r[b] to r[a]
 * Inputs:  Segments seg_memory: reference to segmented memory
            uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers array to be properly initialized 
 ************************/
static inline void segmented_store(struct Segments seg_memory, 
                                   uint32_t *registers, uint32_t ra,
                                   uint32_t rb, uint32_t rc)
{
        uint32_t word = registers[rc];
        uint32_t id = registers[ra];
        uint32_t offset = registers[rb];
        word_store(seg_memory, word, id, offset);
}

/********** addition ********
 * Purpose: sets r[a] to the value r[b] + r[c]
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers array to be properly initialized 
 ************************/
static inline void addition(uint32_t *registers, uint32_t ra, uint32_t rb,
                            uint32_t rc)
{
        uint32_t result = registers[rb] + registers[rc];
        registers[ra] = result;
}

/********** multiplication ********
 * Purpose: sets r[a] to the value r[b] * r[c]
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers array to be properly initialized 
 ************************/
static inline void multiplication(uint32_t *registers, uint32_t ra,
                                  uint32_t rb, uint32_t rc)
{
        uint32_t result = registers[rb] * registers[rc];
        registers[ra] = result;
}

/********** division ********
 * Purpose: sets r[a] to the value r[b] / r[c]
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers array to be properly initialized 
 ************************/
static inline void division(uint32_t *registers, uint32_t ra, uint32_t rb,
                            uint32_t rc)
{
        assert(registers[rc] != 0);
        uint32_t result = registers[rb] / registers[rc];
        registers[ra] = result;
}

/********** bitwise_nand ********
 * Purpose: sets r[a] to the value ~(r[b] & r[c]) (bitwise &)
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t ra, rb, rc: indices of the desired registers
 * Return:  void
 * Expects: registers array to be properly initialized 
 ************************/
static inline void bitwise_nand(uint32_t *registers, uint32_t ra, uint32_t rb,
                                uint32_t rc)
{
        uint32_t result = registers[rb] & registers[rc];
        result = ~result;
        registers[ra] = result;
}

/********** halt ********
 * Purpose: sets the program counter to the length of the program, effectively
            ending the program
 * Inputs:  uint32_t *program_counter: reference to the program_counter
            uint32_t program_length: length of the array in segment 0
 * Return:  void
 * Expects:  
 ************************/
static inline void halt(uint32_t *program_counter, uint32_t program_length)
{
        *program_counter = program_length;
}

/********** map_seg ********
 * Purpose: maps a new segment into memory and stores its id in r[b]
 * Inputs:  Segments seg_memory: reference to segmented memory
            uint32_t *registers: pointer to the registers array
            uint32_t rb, rc: indices of the desired registers
 * Return:  void
 * Expects: seg_memory and registers to be properly initialized
 ************************/
static inline void map_seg(struct Segments seg_memory, uint32_t *registers,
                           uint32_t rb, uint32_t rc)
{
        uint32_t num_words = registers[rc];
        uint32_t id = segment_map(seg_memory, num_words);
        registers[rb] = id;
}

/********** unmap_seg ********
 * Purpose: unmaps segment with id r[c]
 * Inputs:  Segments seg_memory: reference to segmented memory
            uint32_t *registers: pointer to the registers array
            uint32_t rc: indices of the desired registers
 * Return:  void
 * Expects: seg_memory and registers to be properly initialized
 ************************/
static inline void unmap_seg(struct Segments seg_memory, uint32_t *registers,
                             uint32_t rc)
{
        uint32_t id = registers[rc];
        segment_unmap(seg_memory, id);
}

/********** output ********
 * Purpose: outputs value r[c] to stdout 
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t rc: index of register
 * Return:  void
 * Expects: registers array to be properly initialized
 ************************/
static inline void output(uint32_t *registers, uint32_t rc)
{
        uint32_t result = registers[rc];
        assert(result <= 255);
        int x = result;
        char c = x;
        fprintf(stdout, "%c", c);
}

/********** input ********
 * Purpose: receives value from stdin and places it in r[c] 
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t rc: index of register
 * Return:  void
 * Expects: registers array to be properly initialized
 ************************/
static inline void input(uint32_t *registers, uint32_t rc)
{
        char c = fgetc(stdin);
        uint32_t result = 0;
        if (c == EOF) {
                result = ~result;
        } else {
                result = c;
        }
        
        registers[rc] = result;
}

/********** load_program ********
 * Purpose: makes copy of segment with id r[b], frees 0 segment, and loads the
            copy into 0 segment; sets program counter to r[c]
 * Inputs:  Segments seg_memory: reference to segmented memory
            uint32_t *registers: pointer to the registers array
            uint32_t rb, rc: indices of registers
            uint32_t *program_counter: reference to program counter
 * Return:  void
 * Expects: seg_memory and registers array to be properly initialized
 ************************/
static inline void load_program(struct Segments seg_memory, 
                                uint32_t *registers, uint32_t rb, uint32_t rc,
                                uint32_t *program_counter)
{
        uint32_t id = registers[rb];
        uint32_t new_counter = registers[rc];
        UArray_T target_program = get_segment(seg_memory, id);
        
        /* catch attempt to load unmapped/not-mapped seg */
        assert(target_program != NULL); 
        /* catch out of bounds program counter */
        assert(new_counter < (uint32_t)UArray_length(target_program)); 
        
        UArray_T copy = UArray_copy(target_program, 
                                    UArray_length(target_program));
        free_segment(seg_memory, 0);
        update_zero_seg(seg_memory, copy);

        *program_counter = new_counter;
}

/********** load_val ********
 * Purpose: sets r[a] to given value
 * Inputs:  uint32_t *registers: pointer to the registers array
            uint32_t ra: index of register
            uint32_t val: value to be stored in registers array
 * Return:  void
 * Expects: registers array to be properly initialized
 ************************/
static inline void load_val(uint32_t *registers, uint32_t ra, uint32_t val)
{
        registers[ra] = val;
}

/********** get_opcode ********
 * Purpose: gets the opcode of an instruction
 * Inputs:  uint32_t word: word to be bitpacked
 * Return:  first 4 bits of given word representing the opcode
 * Expects: 
 ************************/
static inline uint32_t get_opcode(uint32_t word)
{
        return (uint32_t)Bitpack_getu(word, 4, 28);
}

/********** get_registers ********
 * Purpose: sets values of ra, rb, and rc to correct values by reference from
            given word
 * Inputs:  uint32_t word: word to be bitpacked
            uint32_t *ra, *rb, *rc: references to values to be set
 * Return:  nothing
 * Expects: 
 ************************/
static inline void get_registers(uint32_t word, uint32_t *ra, uint32_t *rb,
                                 uint32_t *rc)
{
        *ra = (uint32_t)Bitpack_getu(word, 3, 6);
        *rb = (uint32_t)Bitpack_getu(word, 3, 3);
        *rc = (uint32_t)Bitpack_getu(word, 3, 0);
}

/********** get_val ********
 * Purpose: sets values of ra and value to correct values by reference from
            given word
 * Inputs:  uint32_t word: word to be bitpacked
            uint32_t *ra, *value: references to values to be set
 * Return:  nothing
 * Expects: 
 ************************/
static inline void get_val(uint32_t word, uint32_t *ra, uint32_t *value)
{
        *ra = (uint32_t)Bitpack_getu(word, 3, 25); 
        *value = (uint32_t)Bitpack_getu(word, 25, 0); 
}