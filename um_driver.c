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

struct Segments {
    UArray_T *mapped;
    uint32_t *unmapped;
    uint32_t *nextID;
    uint32_t *mapped_len;
    uint32_t *unmapped_len;
    uint32_t *program_length;
};

typedef struct Segments *Segments;


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

        //initialize
        struct Segments seg;
        seg.mapped = calloc(1000000, sizeof(UArray_T));
        assert(seg.mapped != NULL);

        seg.unmapped = calloc(1000000, sizeof(uint32_t));
        assert(seg.unmapped != NULL);

        seg.nextID = malloc(sizeof(seg.nextID));
        assert(seg.nextID != NULL);
        (*seg.nextID) = 1;

        seg.mapped_len = malloc(sizeof(seg.mapped_len));
        assert(seg.mapped_len != NULL);
        (*seg.mapped_len) = 0;

        seg.unmapped_len = malloc(sizeof(seg.unmapped_len));
        assert(seg.unmapped_len != NULL);
        (*seg.unmapped_len) = 0;

        seg.program_length = malloc(sizeof(seg.program_length));
        assert(seg.program_length != NULL);
        (*seg.program_length) = 0;

        //update zero seg
        seg.mapped[0] = (void *)program;
        (*seg.mapped_len)++;
        (*seg.program_length) = (uint32_t)UArray_length(program);


        uint32_t *registers = (uint32_t *) calloc(8, sizeof(uint32_t));
        assert(registers != NULL);
        /* runs until end of zero segment in case of no halt command */
        while (program_counter < (*seg.program_length)) {

                //word load
                assert(0 < *seg.mapped_len);
                UArray_T program_seg = seg.mapped[0];
                

                assert(program_seg != NULL); 
                assert((uint32_t)UArray_length(program_seg) > program_counter); 
                uint32_t cur_command = (*(uint32_t *)UArray_at(program_seg, program_counter));
                
                uint32_t opcode = ((uint64_t)(cur_command) << 32) >> 60; 

                uint32_t ra, rb, rc, value;
                value = 0;
                ra = 0;
                rb = 0;
                rc = 0;
                if (opcode == 13){
                        
                        ra = ((uint64_t)(cur_command) << 36) >> 61; 
                        value = ((uint64_t)(cur_command) << 39) >> 39; 
                        assert(ra < 8);
                } else {
                        
                        ra = ((uint64_t)(cur_command) << 55) >> 61; 
                        rb = ((uint64_t)(cur_command) << 58) >> 61; 
                        rc = ((uint64_t)(cur_command) << 61) >> 61; 
                        assert(ra < 8);
                        assert(rb < 8);
                        assert(rc < 8);
                }
                uint32_t id, offset, word, result, num_words, new_counter;
                int x;
                char c;
                UArray_T target_segment = NULL;
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
                                assert(id < *seg.mapped_len);
                                target_segment = seg.mapped[id];

                                assert(target_segment != NULL); 
                                assert((uint32_t)UArray_length(target_segment) > offset); 
                                word = (*(uint32_t *)UArray_at(target_segment, offset));
                                registers[ra] = word;
                                break;
                        case 2:
                                word = registers[rc];
                                id = registers[ra];
                                offset = registers[rb];

                                assert(id < *seg.mapped_len);
                                target_segment = seg.mapped[id];

                                assert(target_segment != NULL); 
                                assert((uint32_t)UArray_length(target_segment) > offset); 

                                *((uint32_t *)UArray_at(target_segment, offset)) = word;
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
                                program_counter = *seg.program_length;
                                break;
                        case 8:
                                num_words = registers[rc];

                                id = 0;
                                if ((*seg.unmapped_len) == 0){ /* new id case */
                                        id = (*seg.nextID);
                                        UArray_T new_seg = UArray_new(num_words, sizeof(uint32_t));
                                        (*seg.nextID)++;
                                        seg.mapped[id] = new_seg;
                                        (*seg.mapped_len)++;
                                        assert((*seg.nextID) == (uint32_t)(*seg.mapped_len));
                                } else { /* unmapped id case */
                                        (*seg.unmapped_len)--;
                                        uint32_t index = (*seg.unmapped_len);
                                        id = seg.unmapped[index];
                                        seg.unmapped[index] = -1;
                                        
                                        UArray_T new_seg = UArray_new(num_words, sizeof(uint32_t));
                                        seg.mapped[id] = new_seg;
                                }

                                registers[rb] = id;
                                break;
                        case 9:
                                id = registers[rc];

                                /* checks for attempt to unmap zero segment */
                                assert(id != 0); //
                                assert(id < *seg.mapped_len);
                                UArray_T table_entry = seg.mapped[id];
                                
                                /* unmap non-mapped or unmapped segment */
                                assert(table_entry != NULL); 
                                
                                UArray_free(&table_entry);
                                table_entry = NULL;
                                
                                seg.mapped[id] = NULL;
                                uint32_t index = (*seg.unmapped_len);
                                seg.unmapped[index] = id;
                                (*seg.unmapped_len)++;

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
                                        assert(id < *seg.mapped_len); 
                                        UArray_T target_program = seg.mapped[id];
                                
                                        /* catch attempt to load unmapped/not-mapped seg */
                                        assert(target_program != NULL); 
                                        /* catch out of bounds program counter */
                                        assert(new_counter < (uint32_t)UArray_length(target_program)); 
                                        
                                        UArray_T copy = UArray_copy(target_program, 
                                                                UArray_length(target_program));
                                        assert(0 < *seg.mapped_len);
                                        UArray_T table_entry = seg.mapped[0];
                                        if (table_entry != NULL) {
                                                UArray_free(&table_entry);
                                                table_entry = NULL;
                                        }

                                        if ((*seg.mapped_len) == 0) {
                                                seg.mapped[0] = (void *)copy;
                                                (*seg.mapped_len)++;

                                        } else {
                                                seg.mapped[0] = copy;
                                        }
                                        (*seg.program_length) = (uint32_t)UArray_length(copy);

                                }
                                program_counter = new_counter;
                                break;
                        case 13:
                                registers[ra] = value;
                                break;
                }

        }
        
        free(registers);

        for (int i = 0; i < (int)(*seg.nextID); i++) {
                assert((uint32_t)i < *seg.mapped_len);
                UArray_T table_entry = seg.mapped[i];
                if (table_entry != NULL) {
                        UArray_free(&table_entry);
                        table_entry = NULL;
                }
        }
        free(seg.mapped);
        free(seg.unmapped);
        free(seg.nextID);
        free(seg.mapped_len);
        free(seg.unmapped_len);
        free(seg.program_length);
        seg.nextID = NULL;
        seg.mapped = NULL;
        seg.unmapped = NULL;
        seg.mapped_len = NULL;
        seg.unmapped_len = NULL;
}



