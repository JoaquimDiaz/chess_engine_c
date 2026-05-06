#include <criterion/criterion.h>

#include "board.h"
#include "move.h"

Test(MakeUnmake, restoreState)
{
    pos_t *pos = starting_pos();
    make_move(pos, e2, e4, DOUBLE_PAWN, pos->side);
    make_move(pos, e7, e5, DOUBLE_PAWN, pos->side);
    unmake_move(pos, e7, e5, pos->side^1);
    // cr_assert(pos->ep == e3);
    cr_assert(pos->side == BLACK);
}


