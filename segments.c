/*
        segments.c
        Ian Ryan (iryan01) and Caden Chang (cchang15)
        10/20/23
        HW#6
        Summary: implementation of segments module, includes functions that
                 maps, unmaps, loads/stores words, and free segements from 
                 memory              
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitpack.h>
#include "segments.h"
#include "atom.h"
#include "seq.h"
#include "assert.h"

/********** initialize ********
 * Purpose: creates a new instance of Segments struct, allocates memory on heap
            for Table, Seq, and a uint32_T
 * Inputs:  nothing
 * Return:  reference to initialzed instance of seg
 * Expects: 
 ************************/
struct Segments initialize()
{
        struct Segments seg;
        seg.mapped = calloc(1000000000, sizeof(UArray_T));
        seg.unmapped = Seq_new(10);

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
void update_zero_seg(struct Segments seg, UArray_T program)
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
uint32_t segment_map(struct Segments seg, uint32_t length)
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
                id = (uint32_t)(uintptr_t)Seq_remhi(seg.unmapped);
                (*seg.unmapped_len)--;
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
void segment_unmap(struct Segments seg, uint32_t id)
{
        /* checks for attempt to unmap zero segment */
        assert(id != 0); //
        UArray_T table_entry = get_segment(seg, id);
        
        /* unmap non-mapped or unmapped segment */
        assert(table_entry != NULL); 
        
        UArray_free(&table_entry);
        table_entry = NULL;
        
        seg.mapped[id] = NULL;
        Seq_addhi(seg.unmapped, (void *)(uintptr_t)id);
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
void word_store(struct Segments seg, uint32_t word, uint32_t id, 
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
uint32_t word_load(struct Segments seg, uint32_t id, uint32_t offset)
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
UArray_T get_segment(struct Segments seg, uint32_t id)
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
void free_segment(struct Segments seg, uint32_t id)
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
void free_all_segments(struct Segments seg)
{
        for (int i = 0; i < (int)(*seg.nextID); i++) {
                free_segment(seg, i);
        }
        free(seg.mapped);
        Seq_free(&seg.unmapped);
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

