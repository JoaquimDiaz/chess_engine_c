# pragma once

#include "board.h"
#include "magic.h"

extern bb_t ATTACKS_PAWN[COLORS][SQ_COUNT];
extern bb_t ATTACKS_KNIGHT[SQ_COUNT];
extern bb_t ATTACKS_KING[SQ_COUNT];
void _init_attacks_pawn();
void _init_attacks_knight();
void _init_attacks_king();

extern bb_t MASKS_ROOK[SQ_COUNT];
extern bb_t MASKS_BISHOP[SQ_COUNT];
void _init_masks_rook();
void _init_masks_bishop();

bb_t attacks_rook_slow(int sq, bb_t occ);
bb_t attacks_bishop_slow(int sq, bb_t occ);
bb_t subset_from_index(int idx, int bits, bb_t mask);

static inline int transform(bb_t blocker, bb_t magic, int bits) {
    return (int)((blocker * magic) >> (64 - bits));
}

// * Magic tables & functions
void _init_offset_table(void);
void _init_rb_attack_tables(void);
void _init_attacks_all(void);

bb_t rook_attacks_fast(int sq, bb_t occ);
bb_t bishop_attacks_fast(int sq, bb_t occ);
bb_t queen_attacks_fast(int sq, bb_t occ);
