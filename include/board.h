#pragma once

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* ----------------------------------------------------------------------------
 * # TYPE & STRUCT DEFINITION
 * ------------------------------------------------------------------------- */

typedef uint64_t bb_t;

typedef enum {
    WHITE  = 0,
    BLACK  = 1,
    COLORS = 2
} color_t;

typedef enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    SQ_COUNT
} sq_t;

typedef enum {
    NO_PIECE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    PIECES
} piece_t;

#define PIECE_COUNT (PIECES-1)

typedef enum {
    NO_CASTLING  = 0x00,
    W_00  = 0x01,
    W_000 = 0x02,
    B_00  = 0x04,
    B_000 = 0x08,
    ALL_CASTLING = 0x0F,
} castling_rights_t;

#define NO_ENPASSANT SQ_COUNT

#define MAX_PLY 256

typedef struct {
    uint8_t  captured;
    uint8_t  castling;
    uint8_t  hm;
    uint16_t fm;
} saved_t;

typedef struct {
    bb_t     bb[COLORS][PIECE_COUNT];
    bb_t     occ[COLORS];
    bb_t     all;
    uint8_t  pl[SQ_COUNT];
    uint8_t  ks[COLORS];
    uint8_t  castling;
    uint8_t  side;
    uint8_t  ep;
    uint8_t  hm;
    uint16_t fm;
    saved_t  ply_stack[MAX_PLY];
    int      ply;
} pos_t;

/* ----------------------------------------------------------------------------
 * # TABLES & CONSTANTS
 * ------------------------------------------------------------------------- */

// RANKS
#define RANK_1 0x00000000000000FFull
#define RANK_2 0x000000000000FF00ull
#define RANK_3 0x0000000000FF0000ull
#define RANK_4 0x00000000FF000000ull
#define RANK_5 0x000000FF00000000ull
#define RANK_6 0x0000FF0000000000ull
#define RANK_7 0x00FF000000000000ull
#define RANK_8 0xFF00000000000000ull
// FILES
#define FILE_A 0x0101010101010101ull
#define FILE_B 0x0202020202020202ull
#define FILE_C 0x0404040404040404ull
#define FILE_D 0x0808080808080808ull
#define FILE_E 0x1010101010101010ull
#define FILE_F 0x2020202020202020ull
#define FILE_G 0x4040404040404040ull
#define FILE_H 0x8080808080808080ull

extern const bb_t RANKS[8];
extern const bb_t FILES[8];

extern uint8_t CASTLING_TABLE[SQ_COUNT];

/* ----------------------------------------------------------------------------
 * # UTILITY FUNCTIONS
 * ------------------------------------------------------------------------- */

// VALIDATORS
#define VALID_PIECE(piece) ((piece) >= PAWN && (piece) <= KING)
#define VALID_COLOR(color) ((color) == WHITE || (color) == BLACK)
#define VALID_RF(r, f)     ((unsigned char)(r) < 8 && (unsigned char)(f) < 8)
// ITERATORS
#define ITER_COLOR(c)  for (color_t (c) = WHITE; (c) < COLORS; (c)++)
#define ITER_PIECE(it) for (piece_t (it) = PAWN; (it) < PIECES; (it)++)
#define ITER_BB(c, p)  ITER_COLOR(c) ITER_PIECE(p)
#define ITER_SQ(sq)    for (sq_t sq = a1; sq < SQ_COUNT; sq++)

static inline void set_bb(pos_t *pos, int c, int piece, bb_t val) {
    assert(VALID_COLOR(c));
    assert(VALID_PIECE(piece));
    pos->bb[c][piece-1] = val;
}

// * Setting and accessing bitboard from `pos`
#define BB(pos, c, p) ((pos)->bb[c][(p)-1])

static inline bb_t bb(pos_t *pos, int c, int piece)
{
    assert(VALID_COLOR(c));
    assert(VALID_PIECE(piece));
    return pos->bb[c][piece-1];
}

static inline bb_t sq_bb(sq_t s)
{
    assert((unsigned char)s < SQ_COUNT);
    return (1ull << s);
}

// Piece list manipulation
static inline color_t piece_color(uint8_t p) { return p >> 3; }
static inline uint8_t piece_type(uint8_t p)  { return p & 7; }
static inline uint8_t make_piece(color_t c, uint8_t p)  { return (c << 3) | p; }

// validate and flip color
static inline void flip_side(pos_t *pos)
{
    assert(VALID_COLOR(pos->side));
    pos->side = pos->side^1;
}

// * PRINTING helpers
// print sq
static inline const char *sq_to_str(int sq)
{
    assert((unsigned char)sq < SQ_COUNT);
    static char buf[3];
    buf[0] = 'a' + (sq & 7);
    buf[1] = '1' + (sq >> 3);
    buf[2] = '\0';
    return buf;
}


/* ----------------------------------------------------------------------------
 * # FUNCTIONS
 * ------------------------------------------------------------------------- */

pos_t *starting_pos(void);

// * PRINTING
void print_bb(bb_t bb);
void print_pos(pos_t *pos);
void print_pos_info(pos_t *pos);

// * FEN PARSING
pos_t *parse_fen(char *fen);

// * BB UTILS
int popcount(bb_t bb);
int poplsb(bb_t *bb);
