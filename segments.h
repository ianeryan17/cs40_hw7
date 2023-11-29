#ifndef SEGMENTS_INCLUDED
#define SEGMENTS_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitpack.h>
#include "seq.h"
#include "table.h"
#include "set.h"
#include "uarray.h"

struct Segments {
    Table_T mapped;
    Seq_T unmapped;
    uint32_t *nextID;
};

typedef struct Segments *Segments;

extern struct Segments initialize();
extern void update_zero_seg(struct Segments seg, UArray_T program);
extern uint32_t segment_map(struct Segments seg, uint32_t length);
extern void segment_unmap(struct Segments seg, uint32_t id);
extern void word_store(struct Segments seg, uint32_t word, uint32_t id, 
                       uint32_t offset);
extern uint32_t word_load(struct Segments seg, uint32_t id, uint32_t offset);
extern void free_segment(struct Segments seg, uint32_t id);
extern void free_all_segments(struct Segments seg);
extern UArray_T get_segment(struct Segments seg, uint32_t id);

#endif