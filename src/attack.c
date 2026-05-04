#include "attack.h"
#include "board.h"

/* ----------------------------------------------------------------------------
 * # ATTACKS TABLES - PAWN / KNIGHT / KING
 * ------------------------------------------------------------------------- */

bb_t ATTACKS_PAWN[COLORS][SQ_COUNT];
bb_t ATTACKS_KNIGHT[SQ_COUNT];
bb_t ATTACKS_KING[SQ_COUNT];

void _init_attacks_pawn()
{
    ITER_COLOR(c){
        ITER_SQ(sq){
            int r = sq >> 3;
            int f = sq & 7;
            r = (c == WHITE) ? r + 1 : r - 1;
            ATTACKS_PAWN[c][sq]  = 0ull; 
            if ((unsigned char)r > 7) continue;
            if ((unsigned char)(f + 1) < 8) 
                ATTACKS_PAWN[c][sq] |= sq_bb(r * 8 + f + 1); 
            if ((unsigned char)(f - 1) < 8) 
                ATTACKS_PAWN[c][sq] |= sq_bb(r * 8 + f - 1); 
        }
    }
}

void _init_attacks_knight()
{
    const int r_dir[] = {2,  2, 1,  1, -2, -2, -1, -1};
    const int f_dir[] = {1, -1, 2, -2,  1, -1,  2, -2};
    ITER_SQ(sq){
        int r = sq >> 3;
        int f = sq & 7;
        ATTACKS_KNIGHT[sq] = 0ull;
        for (int i = 0; i < 8; i++) {
            int rd = r + r_dir[i];
            int fd = f + f_dir[i];
            if (VALID_RF(rd, fd)) ATTACKS_KNIGHT[sq] |= sq_bb(rd * 8 + fd);
        }
    }
}

void _init_attacks_king()
{
    const int r_dir[] = { 1, 1, 1,  0, 0, -1, -1, -1};
    const int f_dir[] = {-1, 0, 1, -1, 1, -1,  0,  1};
    ITER_SQ(sq) {
        int r = sq >> 3;
        int f = sq & 7;
        ATTACKS_KING[sq] = 0ull;
        for (int i = 0; i < 8; i++) {
            int rd = r + r_dir[i];
            int fd = f + f_dir[i];
            if (VALID_RF(rd, fd)) ATTACKS_KING[sq] |= sq_bb(rd * 8 + fd);
        }
    }
}

/* ----------------------------------------------------------------------------
 * # ROOK / BISHOP - MASKS
 * ------------------------------------------------------------------------- */

bb_t MASKS_ROOK[SQ_COUNT];
bb_t MASKS_BISHOP[SQ_COUNT];

void _init_masks_rook()
{
    ITER_SQ(sq) {
        int r = sq >> 3;
        int f = sq & 7;
        MASKS_ROOK[sq] = ((RANKS[r] & ~(FILES[0] | FILES[7]))
                       | (FILES[f] & ~(RANKS[0] | RANKS[7])))
                       & ~sq_bb(sq);
    }
}

void _init_masks_bishop()
{
    const int r_dir[] = {1, 1, -1, -1};
    const int f_dir[] = {1, -1, 1, -1};
    ITER_SQ(sq) {
        int r = sq >> 3;
        int f = sq & 7;
        MASKS_BISHOP[sq] = 0ull;
        for (int i = 0; i < 4; i++) {
            int rd = r + r_dir[i];
            int fd = f + f_dir[i];
            while (VALID_RF(rd, fd)) {
                MASKS_BISHOP[sq] |= sq_bb(rd * 8 + fd);
                rd = rd + r_dir[i];
                fd = fd + f_dir[i];
            }
        }
        MASKS_BISHOP[sq] &= ~(RANKS[0] | RANKS[7] | FILES[0] | FILES[7]);
    }
}

bb_t attacks_rook_slow(int sq, bb_t occ) 
{
    const int r_dir[] = {1, -1, 0,  0};
    const int f_dir[] = {0,  0, 1, -1};
    int r = sq >> 3;
    int f = sq & 7;
    bb_t attacks = 0ull;
    for (int i = 0; i < 4; i++) {
        int rd = r + r_dir[i];
        int fd = f + f_dir[i];
        while (VALID_RF(rd, fd)) {
            attacks |= sq_bb(rd * 8 + fd);
            if (sq_bb(rd * 8 + fd) & occ) break;
            rd += r_dir[i];
            fd += f_dir[i];
        }
    }
    return attacks;
}

bb_t attacks_bishop_slow(int sq, bb_t occ)
{
    const int r_dir[] = {1, 1, -1, -1};
    const int f_dir[] = {1, -1, 1, -1};
    int r = sq >> 3;
    int f = sq & 7;
    bb_t attacks = 0ull;
    for (int i = 0; i < 4; i++) {
        int rd = r + r_dir[i];
        int fd = f + f_dir[i];
        while (VALID_RF(rd, fd)) {
            attacks |= sq_bb(rd * 8 + fd);
            if (sq_bb(rd * 8 + fd) & occ) break;
            rd += r_dir[i];
            fd += f_dir[i];
        }
    }
    return attacks;
}

bb_t subset_from_index(int idx, int bits, bb_t mask) 
{
    bb_t subset = 0ull;
    for (int i = 0; i < bits; i++) 
    {
        bb_t sq = 1ull << poplsb(&mask);
        if ((1 << i) & idx) subset |= sq;
    }
    return subset;
}

/* ----------------------------------------------------------------------------
 * # ROOK / BISHOP / QUEEN - FAST
 * ------------------------------------------------------------------------- */

#ifndef GENERATING_MAGICS

const int ROOK_BITS[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

const int BISHOP_BITS[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

int ROOK_OFFSET[SQ_COUNT];
int BISHOP_OFFSET[SQ_COUNT];

int ROOK_ATTACK_MAX = 0;
int BISHOP_ATTACK_MAX = 0;

void _init_offset_table(void) 
{
    int r_offset = 0;
    int b_offset = 0;
    for (int sq = a1; sq < SQ_COUNT; sq++) {
        ROOK_OFFSET[sq] = r_offset;
        BISHOP_OFFSET[sq] = b_offset;

        r_offset += 1u << ROOK_BITS[sq];
        b_offset += 1u << BISHOP_BITS[sq];
    }
    ROOK_ATTACK_MAX = r_offset;
    BISHOP_ATTACK_MAX = b_offset;
}

bb_t *ROOK_ATTACKS = NULL;
bb_t *BISHOP_ATTACKS = NULL;

void _init_rb_attack_tables(void) 
{
    ROOK_ATTACKS = malloc(sizeof(bb_t) * ROOK_ATTACK_MAX);
    BISHOP_ATTACKS = malloc(sizeof(bb_t) * BISHOP_ATTACK_MAX);

    if (!ROOK_ATTACKS || !BISHOP_ATTACKS) {
        perror("malloc");
        exit(1);
    }

    for (int sq = a1; sq < SQ_COUNT; sq++) {
        int rook_size = 1 << ROOK_BITS[sq];
        for (int i = 0; i < rook_size; i++) {
            bb_t occ = subset_from_index(i, ROOK_BITS[sq], MASKS_ROOK[sq]);
            bb_t attacks = attacks_rook_slow(sq, occ);
            int idx = transform(occ, ROOK_MAGIC[sq], ROOK_BITS[sq]);
            // assert((unsigned)idx < (1u << ROOK_BITS[sq]));
            ROOK_ATTACKS[ROOK_OFFSET[sq] + idx] = attacks;
        }

        int bishop_size = 1 << BISHOP_BITS[sq];
        for (int i = 0; i < bishop_size; i++) 
        {
            bb_t occ = subset_from_index(i, BISHOP_BITS[sq], MASKS_BISHOP[sq]);
            bb_t attacks = attacks_bishop_slow(sq, occ);
            int idx = transform(occ, BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
            // assert((unsigned)idx < (1u << BISHOP_BITS[sq]));
            BISHOP_ATTACKS[BISHOP_OFFSET[sq] + idx] = attacks;
        }
    }
}

bb_t rook_attacks_fast(int sq, bb_t occ) 
{
    occ &= MASKS_ROOK[sq];
    int idx = transform(occ, ROOK_MAGIC[sq], ROOK_BITS[sq]);
    return ROOK_ATTACKS[ROOK_OFFSET[sq] + idx];
}

bb_t bishop_attacks_fast(int sq, bb_t occ) 
{
    occ &= MASKS_BISHOP[sq];
    int idx = transform(occ, BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
    return BISHOP_ATTACKS[BISHOP_OFFSET[sq] + idx];
}

bb_t queen_attacks_fast(int sq, bb_t occ) 
{
    bb_t attacks = 0ULL;
    attacks |= rook_attacks_fast(sq, occ);
    attacks |= bishop_attacks_fast(sq, occ);
    return attacks;
}

void _init_attacks_all(void)
{
    _init_attacks_pawn();
    _init_attacks_knight();
    _init_attacks_king();
    _init_masks_rook();
    _init_masks_bishop();
    _init_offset_table();
    _init_rb_attack_tables();
}

#endif
