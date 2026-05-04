#include <criterion/criterion.h>
#include "board.h"

Test(Board, square)
{
    cr_assert(SQ_COUNT == 64);
    cr_assert(a1 == 0);
    cr_assert(h8 == 63);
}

Test(Board, startingPosition)
{
    pos_t *pos = starting_pos();
    // KING SQ
    cr_assert(BB(pos, BLACK, KING) & sq_bb(pos->ks[BLACK]) & sq_bb(e8));
    cr_assert(BB(pos, WHITE, KING) & sq_bb(pos->ks[WHITE]) & sq_bb(e1));
    free(pos);
}
