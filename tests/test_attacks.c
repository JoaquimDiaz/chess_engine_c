#include <criterion/criterion.h>

#include "board.h"
#include "attack.h"

Test(Attacks, BishopAttacks)
{
    _init_attacks_all();
    pos_t *pos = parse_fen("rnbqk1nr/pppp1ppp/8/2b1p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3");
    bb_t wbb = bishop_attacks_fast(c4, pos->all);
    bb_t bbb = bishop_attacks_fast(c5, pos->all);

    cr_assert(sq_bb(f7) & wbb, "white bishop should attack f7");
    cr_assert(!(sq_bb(g8) & wbb), "white bishop should not attack f8");
    cr_assert(sq_bb(f2) & bbb, "black bishop should attack f2");
    cr_assert(!(sq_bb(g1) & bbb), "black bishop should not attack f8");
}

Test(Attacks, RookAttacks)
{
    _init_attacks_all();
    pos_t *pos = parse_fen("4k3/8/8/8/N3R2n/8/8/r3K3 w - - 0 1");
    bb_t wbb = rook_attacks_fast(e4, pos->all);
    bb_t bbb = rook_attacks_fast(a1, pos->all);
    cr_assert(sq_bb(e8) & wbb, "white rook should attack e8");
    cr_assert(sq_bb(e1) & bbb, "black rook should attack e1");
    cr_assert(!(sq_bb(a5) & bbb), "black rook should not attack a5");
}
