#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "seq.h"
#include "assert.h"
#include <stdint.h>
#include <bitpack.h>
#include "um_driver.h"
#include "word_extractor.h"
#include "segments.h"
#include "atom.h"
#include "uarray.h"

void test_registers();
void test_loadvalue();
void test_seg_initialize();
void test_get_segment();
void test_update_0seg();
void test_mapped_case1();
void test_mapped_case2();
void test_mapped_case3(); //tests successful unmap as well
void test_unmap_fail_non_mapped();
void test_unmap_fail_0seg();
void test_load_word();
void test_load_word_fail_offset();
void test_load_word_fail_id();
void test_word_store();
void test_word_store_fail_offset();
void test_word_store_fail_id();
void test_full();
void test_size();

int main(int argc, char *argv[])
{
        (void) argc;
        (void) argv;
        // test_registers();
        // test_loadvalue();
        //test_seg_initialize();
        // test_get_segment();
        // test_update_0seg();
        // fprintf(stderr, "0seg done\n");
        // test_mapped_case1();
        // fprintf(stderr, "case1 done\n");
        test_mapped_case2();
        fprintf(stderr, "case2 done\n");
        // test_mapped_case3();
        // test_unmap_fail_non_mapped();
        // test_unmap_fail_0seg();
        //test_load_word();
        // test_load_word_fail_offset();
        // test_load_word_fail_id();
        //test_word_store();
        // test_word_store_fail_offset();
        // test_word_store_fail_id();
        // test_full();
        //test_size();
}


// tested before making into static inline functions
void test_registers()
{
        uint32_t test_word = 805306577;
        uint32_t result = get_opcode(test_word);
        assert(result = 3);
        unsigned ra;
        unsigned rb;
        unsigned rc;
        get_registers(test_word, &ra, &rb, &rc);
        assert(ra == 3);
        assert(rb == 2);
        assert(rc == 1);
}

void test_loadvalue()
{
        uint32_t test_word = 3724541969;
        assert(get_opcode(test_word) == 13);
        unsigned ra, value;
        get_val(test_word, &ra, &value);
        assert(ra == 7);
        assert(value == 17);
}

void test_seg_initialize()
{
        struct Segments tester = initialize();
        assert(Table_length(tester.mapped) == 0);
        assert(Seq_length(tester.unmapped) == 0);
        free_all_segments(tester);
}

// void test_update_0seg() 
// {
//         struct Segments tester = initialize();
//         Seq_T temp = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t four = 4;
//                 Seq_addhi(temp, (void *)(uintptr_t)four);
//         }
//         update_zero_seg(tester, temp);
//         assert(Table_length(tester.mapped) == 1);
//         assert(Seq_length(tester.unmapped) == 0);
//         Seq_T holder = get_segment(tester, 0);
//         assert(Seq_length(holder) == 10);
//         for (int i = 0; i < Seq_length(holder); i++) {
//                 assert((uint32_t)(uintptr_t)Seq_get(holder, i) == 4);
//         }
//         free_all_segments(tester);
// }

// //maps 4 segments, tests with table and sequence length
// void test_mapped_case1() {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);

//         Seq_T mapped_ids = Seq_new(4);
//         for (int i = 0; i < 4; i++) {
//                 assert(*(tester.nextID) == (uint32_t)(i+1));
//                 uint32_t id = segment_map(tester, 10);
//                 Seq_addhi(mapped_ids, (void *)(uintptr_t)id);
//         }
        
//         assert(Table_length(tester.mapped) == 5);
//         assert(Seq_length(tester.unmapped) == 0);
//         assert(*(tester.nextID) == (uint32_t)5);
        
//         Seq_free(&mapped_ids);
//         free_all_segments(tester);
// }

// // maps and unmaps segments, checks to make sure size of table and sequence is
// // correct
void test_mapped_case2() {
        struct Segments tester = initialize();
        UArray_T test_program = UArray_new(10, sizeof(uint32_t));
        for (int i = 0; i < 10; i++) {
                uint32_t zero = 0;
                *((uint32_t *)UArray_at(test_program, i)) = zero;
        }
        update_zero_seg(tester, test_program);

        Seq_T mapped_ids = Seq_new(4);
        for (int i = 0; i < 4; i++) {
                assert(*(tester.nextID) == (uint32_t)(i+1));
                uint32_t id = segment_map(tester, 10);
                Seq_addhi(mapped_ids, (void *)(uintptr_t)id);
        }
        
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 0);
        
        segment_unmap(tester, 3);
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 1);

        uint32_t new_id = segment_map(tester, 10);
        assert(new_id == 3);
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 0);
        assert(*(tester.nextID) == (uint32_t)5);

        segment_unmap(tester, 3);
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 1);
        assert(*(tester.nextID) == (uint32_t)5);

        segment_unmap(tester, 1);
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 2);
        assert(*(tester.nextID) == (uint32_t)5);

        uint32_t new_id1 = segment_map(tester, 10);
        assert(new_id1 == 1);
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 1);
        assert(*(tester.nextID) == (uint32_t)5);

        uint32_t new_id2 = segment_map(tester, 10);
        assert(new_id2 == 3);
        assert(Table_length(tester.mapped) == 5);
        assert(Seq_length(tester.unmapped) == 0);
        assert(*(tester.nextID) == (uint32_t)5);

        uint32_t new_id3 = segment_map(tester, 10);
        assert(new_id3 == 5);
        assert(Table_length(tester.mapped) == 6);
        assert(Seq_length(tester.unmapped) == 0);
        assert(*(tester.nextID) == (uint32_t)6);

        Seq_free(&mapped_ids);
        free_all_segments(tester);
}

// //tests many seg_maps and seg_unmaps
// void test_mapped_case3() {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);

//         Seq_T mapped_ids = Seq_new(1000);
//         for (int i = 0; i < 1000; i++) {
//                 assert(*(tester.nextID) == (uint32_t)(i+1));
//                 uint32_t id = segment_map(tester, 10);
//                 Seq_addhi(mapped_ids, (void *)(uintptr_t)id);
//         }

//         assert(Table_length(tester.mapped) == 1001);
//         assert(Seq_length(tester.unmapped) == 0);

//         for (int i = 1; i < 1001; i++) {
//                 segment_unmap(tester, i);
//                 assert(Seq_length(tester.unmapped) == i);
//         }

//         assert(Table_length(tester.mapped) == 1001);
//         assert(Seq_length(tester.unmapped) == 1000);

//         Seq_free(&mapped_ids);
//         free_all_segments(tester);
// }

// void test_unmap_fail_non_mapped()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);   

//         segment_unmap(tester, 4);
// }

// void test_unmap_fail_0seg()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);   

//         segment_unmap(tester, 0);
// }

// void test_load_word()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);   
//         segment_map(tester, 3, 10);
//         uint32_t result = word_load(tester, 3, 7);
//         assert(result == 0);
//         // fprintf(stderr, "test_load_word, word: %d\n", result);
//         free_all_segments(tester);

// }

// //currently, word_load fails with assert as it attempts to seq_get from out of bounds, which is a Hanson exception
// void test_load_word_fail_offset()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);   
//         segment_map(tester, 3, 10);
//         // uint32_t result = word_load(tester, 3, 10);
//         // fprintf(stderr, "test_load_word_fail_offset, word: %d\n", result);
// }

// void test_load_word_fail_id()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);   
//         segment_map(tester, 3, 10);
//         // uint32_t result = word_load(tester, 10, 1);
//         // fprintf(stderr, "test_load_word_fail_id, word: %d\n", result);
// }

// void test_word_store()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);
        
//         segment_map(tester, 3, 10);
//         word_store(tester, 5, 3, 6);
//         uint32_t result = word_load(tester, 3, 6);
//         assert(result == 5);
//         free_all_segments(tester);

//         // fprintf(stderr, "test_word_store, word: %d\n", result);
// }


// //word store doesn't fail with an assert currently, just returns NULL and continues
// void test_word_store_fail_offset()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);
//         segment_map(tester, 3, 10);
//         word_store(tester, 5, 3, 11);
//         // uint32_t result = word_load(tester, 3, 6);
//         // fprintf(stderr, "test_word_store_fail_offset, word: %d\n", result);
// }

// void test_word_store_fail_id()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);
//         segment_map(tester, 3, 10);
//         segment_unmap(tester, 3);
//         word_store(tester, 5, 3, 4);
//         // uint32_t result = word_load(tester, 3, 4);
//         // fprintf(stderr, "test_word_store_fail_id, word: %d\n", result);
        
// }

// void test_full()
// {
//         struct Segments tester = initialize();
//         Seq_T test_program = Seq_new(10);
//         for (int i = 0; i < 10; i++) {
//                 uint32_t zero = 0;
//                 Seq_addhi(test_program, (void *)(uintptr_t)zero);
//         }
//         update_zero_seg(tester, test_program);
//         segment_map(tester, 1034);
//         segment_map(tester, 430);
//         segment_map(tester, 10);
//         segment_map(tester, 10);
//         segment_map(tester, 0);
//         segment_map(tester, 1);
//         assert(Table_length(tester.mapped) == 7);

//         word_store(tester, 1, 12, 9);
//         assert(word_load(tester, 12, 9) == 1);
//         word_store(tester, 33, 325, 0);
//         assert(word_load(tester, 325, 0) == 33);

//         segment_unmap(tester, 789);
//         segment_map(tester, 11);
        
//         word_store(tester, 1, 789, 10);

//         segment_map(tester, 5);
//         word_store(tester, 0, 1, 0);
//         word_store(tester, 1, 1, 1);
//         word_store(tester, 2, 1, 2);
//         word_store(tester, 3, 1, 3);
//         word_store(tester, 4, 1, 4);

//         //load program prototype start
//         Seq_T new_program = get_segment(tester, 1);
//         Seq_T copy = Seq_new(5);
//         for (int i = 0; i < 5; i++)  {
//                 uint32_t int_copy = (uint32_t)(uintptr_t)Seq_get(new_program, i);
//                 Seq_addhi(copy, (void *)(uintptr_t)int_copy);
//         }
//         free_segment(tester, 0);
//         update_zero_seg(tester, copy);
//         //load program prototype end
        
//         word_store(tester, 1, 1, 0);

//         assert(word_load(tester, 0, 0) == 0);
//         assert(word_load(tester, 1, 0) == 1);

//         free_all_segments(tester);
// }


//old tests
// void test_get_segment()
// {
//         struct Segments tester = initialize(); //makes table and sequence
//         uint32_t num1 = 4;
//         uint32_t num2 = 5;
//         uint32_t num3 = 7;
//         uint32_t num4 = 8;
//         uint32_t num5 = 9;
//         uint32_t num6 = 10;
//         Seq_T test = Seq_seq((void *)(uintptr_t) num1,
//                              (void *)(uintptr_t) num2,
//                              (void *)(uintptr_t) num3, 
//                              (void *)(uintptr_t) num4,
//                              (void *)(uintptr_t) num5,
//                              (void *)(uintptr_t) num6, NULL); //makes sequence
//         Seq_free(&test);
//         // fprintf(stderr, "original table size: %d\n", Seq_length(test));
//         segment_map(tester, 1);
//         segment_map(tester, 2);
//         segment_map(tester, 3);
//         segment_map(tester, 4);
//         // fprintf(stderr, "original seq length: %d\n", Seq_length(test));

//         // segment_map(tester, 5, 5);
        
//         free_all_segments(tester);

//         // Seq_T result = get_segment(tester, 12);
//         // Seq_free(&result);
//         // Table_free(&(tester.mapped));
//         // Set_free(&(tester.unmapped));
//         // int length = Seq_length(result);
//         // fprintf(stderr, "seq length: %d\n", length);
//         // for (int i = 0; i < length; i++){
//         //         // fprintf(stderr, "element %u\n", (uint32_t)(uintptr_t)Seq_get(result, i));
//         // }
// }


