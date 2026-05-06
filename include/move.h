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

// Print helper
#define PRINTML(ml) do {\
    for (size_t i = 0; i < (ml).count; i++)\
        print_move(ml.moves[i]);\
} while (0)

// Move encoding
static inline move_t mencode(int from, int to, flags_t f) {
    assert((unsigned char)from < SQ_COUNT);
    assert((unsigned char)to < SQ_COUNT);
    assert((unsigned char)f < 16);
    return (f << 12) | (to << 6) | from;
}
static inline int mfrom(move_t m) { return m & 0x3F; } 
static inline int mto(move_t m)   { return (m >> 6) & 0x3F; } 
static inline int mflag(move_t m) { return (m >> 12) & 0x0F; } 

// Flags
static inline int is_prom(flags_t f) { return f >= PROM_N; }
static inline int is_capture(flags_t f) { 
    return f == CAPTURE || f == EN_PASSANT || f >= PROM_CAP_N; 
}
static inline int is_castle(flags_t f) { return f == CASTLE_KING || f == CASTLE_QUEEN; }
static inline int prom_piece(flags_t f) { return KNIGHT + ((f - PROM_N) & 3); }

#define ITER_ML(ml, it) for (size_t (it) = 0; (it) < (ml).count; (it)++)

// * FUNCTIONS
void print_move(move_t m);
void _pseudo_legal_pawn(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_knight(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_king(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_rook(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_bishop(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_queen(pos_t *pos, ml_t *ml, color_t c);
void _all_pseudo_legal(pos_t *pos, ml_t *ml, color_t c);


void make_move(pos_t *pos, int from, int to, int flag, color_t c);
void unmake_move(pos_t *pos, int from, int to, color_t c);
void quick_make(pos_t *pos, int from, int to);
void gen_legal(pos_t *pos, color_t c, ml_t *ml_pseudo, ml_t *ml_legal);
