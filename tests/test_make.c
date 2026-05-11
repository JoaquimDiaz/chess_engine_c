#include <criterion/criterion.h>

#include "board.h"
#include "move.h"

Test(MakeUnmake, restoreState)
{
    pos_t *pos = starting_pos();
    make_move(pos, e2, e4, DOUBLE_PAWN, pos->side);
    make_move(pos, e7, e5, DOUBLE_PAWN, pos->side);
    unmake_move(pos, e7, e5, pos->side^1);
    cr_assert(pos->ep == e3);
    cr_assert(pos->side == BLACK);
    free(pos);
}

Test(MakeUnamke, makemakemake)
{
    _init_attacks_all();
    _init_castling_table();
    pos_t *pos = starting_pos();
    make_move(pos, e2, e4, DOUBLE_PAWN, pos->side);
    make_move(pos, e7, e5, DOUBLE_PAWN, pos->side);
    make_move(pos, f1, c4, NO_FLAG, pos->side);
    make_move(pos, f8, c5, NO_FLAG, pos->side);
    make_move(pos, g1, f3, NO_FLAG, pos->side);
    make_move(pos, g8, f6, NO_FLAG, pos->side);
    make_move(pos, e1, g1, CASTLE_KING, pos->side);
    make_move(pos, e8, g8, CASTLE_KING, pos->side);
    // make_move(pos, e1, e2, NO_FLAG, pos->side);
    // make_move(pos, e8, e7, NO_FLAG, pos->side);
    free(pos);
}

Test(MakeUnmake, EnPassant)
{
    pos_t *pos = starting_pos();
    make_move(pos, e2, e4, DOUBLE_PAWN, pos->side);
    make_move(pos, d7, d5, DOUBLE_PAWN, pos->side);
    make_move(pos, e4, d5, CAPTURE, pos->side);
    make_move(pos, c7, c5, DOUBLE_PAWN, pos->side);
    make_move(pos, d5, c5, EN_PASSANT, pos->side);
    free(pos);
}
