/*
        segments.c
        Ian Ryan (iryan01) and Caden Chang (cchang15)
        10/20/23
        HW#6
        Summary: calls program_reader function to get program words from file,
                 and gives program to um with run()
                 
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "seq.h"
#include "assert.h"
#include <stdint.h>
#include <bitpack.h>
#include "um_driver.h"
#include "uarray.h"
#include <sys/stat.h>


int main(int argc, char *argv[])
{
        assert(argc == 2);
        char *program = argv[1];
        

        FILE *program_ptr = fopen(program, "r");
        if (program_ptr == NULL){
                fprintf(stderr, "The given file %s was unable to be", program);
                fprintf(stderr, " read as it is not in the proper format.\n");
                fprintf(stderr, "Please try again with a proper .um file.\n");
                return EXIT_FAILURE;
        }

        
        /* use stat to find size of um program to size UArray */
        struct stat st;
        stat(program, &st);
        int num_words = (st.st_size)/4;
        //UArray_T um_program = UArray_new(num_words, sizeof(uint32_t));
        uint32_t *um_program = calloc(num_words, sizeof(uint32_t));
        
        char curr_char;
        int counter = 0;
        uint32_t word_holder = 0;
        int index = 0;
        while (fscanf(program_ptr, "%c", &curr_char) == 1) {
                counter++;
                int c_int = curr_char;
                assert(c_int < 256);
                word_holder = (uint32_t)Bitpack_news((uint64_t)word_holder, 8,
                                                      32 - (8 * counter), 
                                                      c_int);
                if (counter == 4){
                        uint32_t new_val = word_holder;
                        um_program[index] = new_val;
                        counter = 0;
                        word_holder = 0;
                        index++;
                }
        }

        fclose(program_ptr);
        
        //run(um_program);

        uint32_t program_counter = 0;

        //initialize
        
        uint32_t **mapped = calloc(1000000, sizeof(uint32_t));
        assert(mapped != NULL);

        uint32_t *unmapped = calloc(1000000, sizeof(uint32_t));
        assert(unmapped != NULL);

        uint32_t *segment_lengths = calloc(1000000, sizeof(uint32_t));
        assert(segment_lengths != NULL);

        uint32_t nextID = 1;
        uint32_t mapped_len = 0;
        uint32_t unmapped_len = 0;
        uint32_t program_length = 0;

        //update zero seg
        mapped[0] = um_program;
        (mapped_len)++;
        (program_length) = num_words;
        segment_lengths[0] = program_length;


        uint32_t *registers = (uint32_t *) calloc(8, sizeof(uint32_t));
        assert(registers != NULL);
        /* runs until end of zero segment in case of no halt command */
        while (program_counter < (program_length)) {

                //word load
                assert(0 < mapped_len);
                uint32_t *program_seg = mapped[0];
                

                assert(program_seg != NULL); 
                assert(program_length > program_counter); 
                uint32_t cur_command = program_seg[program_counter];
                
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
                uint32_t *target_segment = NULL;
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
                                assert(id < mapped_len);
                                target_segment = mapped[id];

                                assert(target_segment != NULL); 
                                assert(segment_lengths[id] > offset); 
                                word = target_segment[offset];
                                registers[ra] = word;
                                break;
                        case 2:
                                word = registers[rc];
                                id = registers[ra];
                                offset = registers[rb];

                                assert(id < mapped_len);
                                target_segment = mapped[id];

                                assert(target_segment != NULL); 
                                assert(segment_lengths[id] > offset); 

                                target_segment[offset] = word;
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
                                program_counter = program_length;
                                break;
                        case 8:
                                num_words = registers[rc];

                                id = 0;
                                if ((unmapped_len) == 0){ /* new id case */
                                        id = (nextID);
                                        uint32_t *new_seg = calloc(num_words, sizeof(uint32_t));
                                        (nextID)++;
                                        mapped[id] = new_seg;
                                        segment_lengths[id] = num_words;
                                        (mapped_len)++;
                                        assert((nextID) == (uint32_t)(mapped_len));
                                } else { /* unmapped id case */
                                        (unmapped_len)--;
                                        uint32_t index = (unmapped_len);
                                        id = unmapped[index];
                                        unmapped[index] = -1;
                                        
                                        uint32_t *new_seg = calloc(num_words, sizeof(uint32_t));
                                        mapped[id] = new_seg;
                                        segment_lengths[id] = num_words;
                                }

                                registers[rb] = id;
                                break;
                        case 9:
                                id = registers[rc];

                                /* checks for attempt to unmap zero segment */
                                assert(id != 0); //
                                assert(id < mapped_len);
                                uint32_t *table_entry = mapped[id];
                                
                                /* unmap non-mapped or unmapped segment */
                                assert(table_entry != NULL); 
                                
                                free(table_entry);
                                mapped[id] = NULL;
                                table_entry = NULL;
                                
                                mapped[id] = NULL;
                                segment_lengths[id] = -1;
                                uint32_t index = (unmapped_len);
                                unmapped[index] = id;
                                (unmapped_len)++;

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
                                        assert(id < mapped_len); 
                                        uint32_t *target_program = mapped[id];
                                
                                        /* catch attempt to load unmapped/not-mapped seg */
                                        assert(target_program != NULL); 
                                        /* catch out of bounds program counter */
                                        assert(new_counter < segment_lengths[id]); 
                                        

                                        uint32_t *copy = calloc(segment_lengths[id], sizeof(uint32_t));
                                        memcpy(copy, target_program, segment_lengths[id] * sizeof(uint32_t));
                                        assert(0 < mapped_len);

                                        //freeing old zero seg
                                        uint32_t *table_entry = mapped[0];
                                        if (table_entry != NULL) {
                                                free(table_entry);
                                                mapped[0] = NULL;
                                                table_entry = NULL;
                                        }

                                        if ((mapped_len) == 0) {
                                                mapped[0] = copy;
                                                (mapped_len)++;

                                        } else {
                                                mapped[0] = copy;
                                        }
                                        (program_length) = segment_lengths[id];
                                        segment_lengths[0] = program_length;

                                }
                                program_counter = new_counter;
                                break;
                        case 13:
                                registers[ra] = value;
                                break;
                }

        }
        
        free(registers);

        for (int i = 0; i < (int)(nextID); i++) {
                assert((uint32_t)i < mapped_len);
                uint32_t *table_entry = mapped[i];
                if (table_entry != NULL) {
                        free(table_entry);
                        table_entry = NULL;
                        mapped[i] = NULL;
                }
        }
        free(mapped);
        free(unmapped);
        free(segment_lengths);
        mapped = NULL;
        unmapped = NULL;
        segment_lengths = NULL;
        
        return EXIT_SUCCESS;
}

