#pragma once

#include "board.h"
#include "attack.h"

typedef uint32_t move_t;

#define MAX_MOVES 256

typedef struct {
    move_t moves[MAX_MOVES];
    size_t  count;
} ml_t;

typedef enum {
    NO_FLAG      = 0,
    DOUBLE_PAWN  = 1,
    CASTLE_KING  = 2,
    CASTLE_QUEEN = 3,
    CAPTURE      = 4,
    EN_PASSANT   = 5,
    PROM_N       = 6,
    PROM_B       = 7,
    PROM_R       = 8,
    PROM_Q       = 9,
    PROM_CAP_N   = 10,
    PROM_CAP_B   = 11,
    PROM_CAP_R   = 12,
    PROM_CAP_Q   = 13,
} flags_t;

// * MOVE HELPERS
static inline move_t mencode(int from, int to, int flags) {
    assert((unsigned char)from < SQ_COUNT);
    assert((unsigned char)to < SQ_COUNT);
    assert((unsigned char)flags < 16);
    return (flags << 12) | (to << 6) | from;
}
static inline int mfrom(move_t m) { return m & 0x3F; } 
static inline int mto(move_t m)   { return (m >> 6) & 0x3F; } 
static inline int mflag(move_t m) { return (m >> 12) & 0x0F; } 

// * FUNCTIONS
void print_move(move_t m);
void _pseudo_legal_pawn(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_knight(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_king(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_rook(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_bishop(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_queen(pos_t *pos, ml_t *ml, color_t c);
void _all_pseudo_legal(pos_t *pos, ml_t *ml, color_t c);


void make_move(pos_t *pos, move_t m);
void quick_make(pos_t *pos, int from, int to);
