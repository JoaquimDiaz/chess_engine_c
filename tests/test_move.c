#include <criterion/criterion.h>

#include "board.h"
#include "move.h"

Test(Move, EncodeDecodeFromTo)
{
    ITER_SQ(sq) {
        move_t m1 = mencode(a1, sq, NO_FLAG);
        move_t m2 = mencode(sq, a1, NO_FLAG);
        cr_assert((sq_t)mto(m1) == sq);
        cr_assert((sq_t)mfrom(m2) == sq);
    }
}

Test(Move, decodePromo)
{
    //MOVE
    move_t m1 = mencode(a1, a1, PROM_N);
    move_t m2 = mencode(a1, a1, PROM_B);
    move_t m3 = mencode(a1, a1, PROM_R);
    move_t m4 = mencode(a1, a1, PROM_Q);
    move_t m5 = mencode(a1, a1, PROM_CAP_N);
    move_t m6 = mencode(a1, a1, PROM_CAP_B);
    move_t m7 = mencode(a1, a1, PROM_CAP_R);
    move_t m8 = mencode(a1, a1, PROM_CAP_Q);
    //ISPROMO
    cr_assert(is_prom(mflag(m1)));
    cr_assert(is_prom(mflag(m2)));
    cr_assert(is_prom(mflag(m3)));
    cr_assert(is_prom(mflag(m4)));
    cr_assert(is_prom(mflag(m5)));
    cr_assert(is_prom(mflag(m6)));
    cr_assert(is_prom(mflag(m7)));
    cr_assert(is_prom(mflag(m8)));
    //PROMPIECE
    cr_assert(prom_piece(mflag(m1)) == KNIGHT); 
    cr_assert(prom_piece(mflag(m2)) == BISHOP);
    cr_assert(prom_piece(mflag(m3)) == ROOK);
    cr_assert(prom_piece(mflag(m4)) == QUEEN);
    cr_assert(prom_piece(mflag(m5)) == KNIGHT);
    cr_assert(prom_piece(mflag(m6)) == BISHOP);
    cr_assert(prom_piece(mflag(m7)) == ROOK);
    cr_assert(prom_piece(mflag(m8)) == QUEEN);
}

Test(Move, isCastle)
{
    move_t m1 = mencode(a1, a1, CASTLE_KING);
    move_t m2 = mencode(a1, a1, CASTLE_QUEEN);
    cr_assert(is_castle(mflag(m1)));
    cr_assert(is_castle(mflag(m2)));
}

