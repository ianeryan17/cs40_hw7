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

static inline struct Segments initialize();
static inline void update_zero_seg(struct Segments seg, UArray_T program);
static inline uint32_t segment_map(struct Segments seg, uint32_t length);
static inline void segment_unmap(struct Segments seg, uint32_t id);
static inline void word_store(struct Segments seg, uint32_t word, uint32_t id, 
                       uint32_t offset);
static inline uint32_t word_load(struct Segments seg, uint32_t id, uint32_t offset);
static inline void free_segment(struct Segments seg, uint32_t id);
static inline void free_all_segments(struct Segments seg);
static inline UArray_T get_segment(struct Segments seg, uint32_t id);


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
        assert(registers != NULL);
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


static inline struct Segments initialize()
{
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
        return seg;
}
// ttest
/********** update_zero_seg ********
 * Purpose: sets 0 segment in segments table to given UArray_T (program)
 * Inputs:  Segments seg: reference to segments struct
            UArray_T program: reference to program to be loaded in
 * Return:  reference to initialzed instance of seg
 * Expects: value in the table that represents 0 seg to be empty
 ************************/
static inline void update_zero_seg(struct Segments seg, UArray_T program)
{
        if ((*seg.mapped_len) == 0) {
                seg.mapped[0] = (void *)program;
                (*seg.mapped_len)++;

        } else {
                seg.mapped[0] = program;
        }
        (*seg.program_length) = (uint32_t)UArray_length(program);
        //fprintf(stderr, "length should be %u but it is %u\n", (*seg.nextID), (uint32_t)Seq_length(seg.mapped));

}

/********** segment_map ********
 * Purpose: initializes a new key value pair in the table and sets the value
            to a UArray_T of the given length of all zeros
 * Inputs:  Segments seg: reference to segments struct
            uint32_t length: length of the newly mapped segment
 * Return:  segment ID of the newly mapped segment 
 * Expects: seg to be initialized properly
 ************************/
static inline uint32_t segment_map(struct Segments seg, uint32_t length)
{
        uint32_t id = 0;
        if ((*seg.unmapped_len) == 0){ /* new id case */
                id = (*seg.nextID);
                UArray_T new_seg = UArray_new(length, sizeof(uint32_t));
                (*seg.nextID)++;
                seg.mapped[id] = new_seg;
                (*seg.mapped_len)++;
                // fprintf(stderr, "length should be %u but it is %u\n", (*seg.nextID), (uint32_t)Seq_length(seg.mapped));
                assert((*seg.nextID) == (uint32_t)(*seg.mapped_len));
        } else { /* unmapped id case */
                //id = (uint32_t)(uintptr_t)Seq_remhi(seg.unmapped);
                (*seg.unmapped_len)--;
                uint32_t index = (*seg.unmapped_len);
                id = seg.unmapped[index];
                seg.unmapped[index] = -1;
                
                UArray_T new_seg = UArray_new(length, sizeof(uint32_t));
                seg.mapped[id] = new_seg;
        }
        //fprintf(stderr, "SEGMENT %u created with length %u\n", id, length);
        return id;
}

/********** segment_unmap ********
 * Purpose: sets the element in the table corresponding to the given key (id)
            to null, while freeing the previous UArray_T
 * Inputs:  Segments seg: reference to segments struct
            uint32_t id: seg id of segment to be unmapped
 * Return:  void 
 * Expects: seg to be initialized properly
 ************************/
static inline void segment_unmap(struct Segments seg, uint32_t id)
{
        /* checks for attempt to unmap zero segment */
        assert(id != 0); //
        UArray_T table_entry = get_segment(seg, id);
        
        /* unmap non-mapped or unmapped segment */
        assert(table_entry != NULL); 
        
        UArray_free(&table_entry);
        table_entry = NULL;
        
        seg.mapped[id] = NULL;
        //Seq_addhi(seg.unmapped, (void *)(uintptr_t)id);
        uint32_t index = (*seg.unmapped_len);
        seg.unmapped[index] = id;
        (*seg.unmapped_len)++;
}

/********** word_store ********
 * Purpose: stores given word in the seg memory address given (id and offset)
 * Inputs:  Segments seg: reference to segments struct
            uint32_t word: value to be stored in memory
            uint32_t id: seg id of desired segment
            uint32_t offset: index of UArray of the given segment
 * Return:  void 
 * Expects: seg to be initialized properly
 ************************/
static inline void word_store(struct Segments seg, uint32_t word, uint32_t id, 
                       uint32_t offset)
{
        UArray_T target_segment = get_segment(seg, id);
        
        /* catches unmapped or non-mapped segment */
        assert(target_segment != NULL); 
        /* ensures offset is within segment length */
        // fprintf(stderr, "trying to store word in id: %u, length: %u and offset: %u\n", id, (uint32_t)UArray_length(target_segment), offset);
        assert((uint32_t)UArray_length(target_segment) > offset); 

        *((uint32_t *)UArray_at(target_segment, offset)) = word;
}

/********** word_load ********
 * Purpose: gets word from given address of segmented memory
 * Inputs:  Segments seg: reference to segments struct
            uint32_t id: seg id of desired segment
            uint32_t offset: index of UArray of the given segment
 * Return:  word located at given address 
 * Expects: seg to be initialized properly
 ************************/
static inline uint32_t word_load(struct Segments seg, uint32_t id, uint32_t offset)
{
        UArray_T target_segment = get_segment(seg, id);
        assert(target_segment != NULL); 
        assert((uint32_t)UArray_length(target_segment) > offset); 
        return (*(uint32_t *)UArray_at(target_segment, offset));
}

/********** get_segment ********
 * Purpose: retrieves reference of given segment id from table
 * Inputs:  Segments seg: reference to segments struct
            uint32_t id: seg id of desired segment
 * Return:  reference to segment at given id
 * Expects: seg to be initialized properly
 ************************/
static inline UArray_T get_segment(struct Segments seg, uint32_t id)
{
        assert(id < *seg.mapped_len);
        return seg.mapped[id];
}

/********** free_segment ********
 * Purpose: frees given segment at given seg id from memory, leaves key in
            table but sets value to NULL
 * Inputs:  Segments seg: reference to segments struct
            uint32_t id: seg id of desired segment
 * Return:  void
 * Expects: seg to be initialized properly
 ************************/
static inline void free_segment(struct Segments seg, uint32_t id)
{
        UArray_T table_entry = get_segment(seg, id);
        if (table_entry != NULL) {
                UArray_free(&table_entry);
                table_entry = NULL;
        }
}

/********** free_all_segments ********
 * Purpose: frees all allocated memory corresponding to segmented memory, calls
            mapping helper function to free each segment individually
 * Inputs:  Segments seg: reference to segments struct
 * Return:  void
 * Expects: seg to be initialized properly
 ************************/
static inline void free_all_segments(struct Segments seg)
{
        for (int i = 0; i < (int)(*seg.nextID); i++) {
                free_segment(seg, i);
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


