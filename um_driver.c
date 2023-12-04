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
        //fprintf(stderr, "before while\n");
        /* runs until end of zero segment in case of no halt command */
        while (program_counter < (*seg_memory.program_length)) {
                uint32_t cur_command = word_load(seg_memory, 0, program_counter);

                //execute_command(seg_memory, cur_command, registers, &program_counter);

                
                
                uint32_t opcode = ((uint64_t)(cur_command) << 32) >> 60; 
        // fprintf(stderr, "opcode: %u\n", opcode);

                uint32_t ra, rb, rc, value;
                value = 0;
                ra = 0;
                rb = 0;
                rc = 0;
                if (opcode == 13){
                        
                        
                        ra = ((uint64_t)(cur_command) << 36) >> 61; 
                        value = ((uint64_t)(cur_command) << 39) >> 39; 
                        assert(ra < 8);
                        // fprintf(stderr, "loading value: %u into %u\n", value, ra);
                } else {
                        
                        ra = ((uint64_t)(cur_command) << 55) >> 61; 
                        rb = ((uint64_t)(cur_command) << 58) >> 61; 
                        rc = ((uint64_t)(cur_command) << 61) >> 61; 
                        assert(ra < 8);
                        assert(rb < 8);
                        assert(rc < 8);
                        // fprintf(stderr, "ra, rb, rc: %u, %u, %u\n", ra, rb, rc);
                }
                uint32_t id, offset, word, result, num_words, new_counter;
                int x;
                char c;
                
                (program_counter)++; 
                assert(opcode <= 13);

                switch(opcode) {
                        case 0:
                                if (registers[rc] != 0) {
                                        uint32_t rb_val = registers[rb];
                                        registers[ra] = rb_val;
                                }
                                break;
                        case 1:
                                id = registers[rb];
                                offset = registers[rc];
                                word = word_load(seg_memory, id, offset);
                                registers[ra] = word;
                                break;
                        case 2:
                                word = registers[rc];
                                id = registers[ra];
                                offset = registers[rb];
                                word_store(seg_memory, word, id, offset);
                                break;
                        case 3:
                                result = registers[rb] + registers[rc];
                                registers[ra] = result;
                                break;
                        case 4:
                                result = registers[rb] * registers[rc];
                                registers[ra] = result;
                                break;
                        case 5:
                                assert(registers[rc] != 0);
                                result = registers[rb] / registers[rc];
                                registers[ra] = result;
                                break;
                        case 6:
                                result = registers[rb] & registers[rc];
                                result = ~result;
                                registers[ra] = result;
                                break;
                        case 7:
                                program_counter = *seg_memory.program_length;
                                break;
                        case 8:
                                num_words = registers[rc];
                                id = segment_map(seg_memory, num_words);
                                registers[rb] = id;
                                break;
                        case 9:
                                id = registers[rc];
                                segment_unmap(seg_memory, id);
                                break;
                        case 10:
                                result = registers[rc];
                                assert(result <= 255);
                                x = result;
                                c = x;
                                fprintf(stdout, "%c", c);
                                break;
                        case 11:
                                c = fgetc(stdin);
                                result = 0;
                                if (c == EOF) {
                                        result = ~result;
                                } else {
                                        result = c;
                                }
                                registers[rc] = result;
                                break;
                        case 12:

                                id = registers[rb];
                                new_counter = registers[rc];
                                if (id != 0) {
                                        UArray_T target_program = get_segment(seg_memory, id);
                                
                                        /* catch attempt to load unmapped/not-mapped seg */
                                        assert(target_program != NULL); 
                                        /* catch out of bounds program counter */
                                        assert(new_counter < (uint32_t)UArray_length(target_program)); 
                                        
                                        UArray_T copy = UArray_copy(target_program, 
                                                                UArray_length(target_program));
                                        free_segment(seg_memory, 0);
                                        update_zero_seg(seg_memory, copy);

                                }
                                program_counter = new_counter;
                                break;
                        case 13:
                                registers[ra] = value;
                                break;
                }

        }
        //fprintf(stderr, "before freeing reg\n");
        free(registers);
        //fprintf(stderr, "after freeing reg\n");
        free_all_segments(seg_memory);
        //fprintf(stderr, "after freeing segs\n");
}





