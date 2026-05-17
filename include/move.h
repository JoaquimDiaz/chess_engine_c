#pragma once

#include <time.h>

#include "board.h"
#include "attack.h"

typedef uint32_t move_t;

#define MAX_MOVES 256
#define MAX_DEPTH 64

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

#define FULL_BB 0xFFFFFFFFFFFFFFFF

// Castling masks
#define MASK_W00  0x60
#define MASK_W000 0x0E
#define MASK_B00  0x6000000000000000
#define MASK_B000 0x0E00000000000000
// Safe castling masks
#define MASK_W00_SAFE  0x70
#define MASK_W000_SAFE 0x1C
#define MASK_B00_SAFE  0x7000000000000000
#define MASK_B000_SAFE 0x1C00000000000000

// * MOVE HELPERS

// Print helper
#define PRINTML(ml) do {\
    for (size_t i = 0; i < (ml).count; i++)\
        print_move(ml.moves[i]);\
} while (0)

#define PRINTML2(pos, ml) do {\
    for (size_t i = 0; i < (ml).count; i++){\
        move_t m = (ml).moves[i];\
        int p = piece_type((pos)->pl[mfrom(m)]);\
        printf("%s: ", p_to_str(p));\
        print_move((ml).moves[i]);}\
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
    return f == CAPTURE || f >= PROM_CAP_N; 
}
static inline int is_castle(flags_t f) { return f == CASTLE_KING || f == CASTLE_QUEEN; }
static inline int prom_piece(flags_t f) { return KNIGHT + ((f - PROM_N) & 3); }

static inline void ml_add(ml_t *ml, int from, int to, flags_t f) {
    assert(ml->count <= MAX_MOVES);
    ml->moves[ml->count++] = mencode(from, to, f);
}

//USELESS
static inline bb_t compute_pin_ray(pos_t *pos, int sq, int us) {
    return (pos->pinned[us] & sq_bb(sq)) ? (MASK_RF[pos->ks[us]] | MASK_DIAG[pos->ks[us]]) : 0ull;
}

static inline int pinner_sq(pos_t *pos, int pinned_sq, int us) {
    return __builtin_ctzll(MASK_PIN[pos->ks[us]][pinned_sq] & pos->pinners[us]);
}

static inline bb_t get_checker_mask(pos_t *pos, int us) {
    assert(popcount(pos->checkers[us]) == 1);
    int checker_sq = __builtin_ctzll(pos->checkers[us]);
    return BLOCKERS[pos->ks[us]][checker_sq] | sq_bb(checker_sq);
}

#define ITER_ML(ml, it) for (size_t (it) = 0; (it) < (ml).count; (it)++)

// * FUNCTIONS
void print_move(move_t m);
int is_square_attacked(pos_t *pos, int sq, int them);
void _pseudo_legal_pawn(pos_t *pos, ml_t *ml, int us);
void _pseudo_legal_knight(pos_t *pos, ml_t *ml, int us);
void _pseudo_legal_king(pos_t *pos, ml_t *ml, int us);
// void _pseudo_legal_king(pos_t *pos, ml_t *ml, color_t c);
void _pseudo_legal_rook(pos_t *pos, ml_t *ml, int us);
void _pseudo_legal_bishop(pos_t *pos, ml_t *ml, int us);
void _pseudo_legal_queen(pos_t *pos, ml_t *ml, int us);
void gen_pseudo_legal(pos_t *pos, ml_t *ml, int us);

bb_t attacked_squares(pos_t *pos, int us);

// Legal move gen
void gen_legal_pawn(pos_t *pos, ml_t *ml, int us);
void gen_legal_knight(pos_t *pos, ml_t *ml, int us);
void gen_legal_bishop(pos_t *pos, ml_t *ml, int us);
void gen_legal_rook(pos_t *pos, ml_t *ml, int us);
void gen_legal_queen(pos_t *pos, ml_t *ml, int us);
void gen_legal_king(pos_t *pos, ml_t *ml, int us);

// Blockers move gen
void gen_blockers_pawn(pos_t *pos, ml_t *ml, int us);
void gen_blockers_knight(pos_t *pos, ml_t *ml, int us);
void gen_blockers_bishop(pos_t *pos, ml_t *ml, int us);
void gen_blockers_rook(pos_t *pos, ml_t *ml, int us);
void gen_blockers_queen(pos_t *pos, ml_t *ml, int us);
void gen_king_incheck(pos_t *pos, ml_t *ml, int us);
void gen_all_blockers(pos_t *pos, ml_t *ml, int us);
void gen_all_moves(pos_t *pos, ml_t *ml, int us);

bb_t compute_pin(pos_t *pos, int us);
bb_t compute_checkers(pos_t *pos, int us);

void make_move(pos_t *pos, int from, int to, int flag, color_t c);
void unmake_move(pos_t *pos, int from, int to, int flag);
void quick_make(pos_t *pos, int from, int to);
void gen_legal(pos_t *pos, ml_t *ml, int us);
// void gen_legal(pos_t *pos, int us, ml_t *ml_pseudo, ml_t *ml_legal);

//
uint64_t perft(pos_t *pos, int depth, int ply);
void perft_root(pos_t *pos, int max_depth);
void init_engine(void);
void make_random(pos_t *pos);
