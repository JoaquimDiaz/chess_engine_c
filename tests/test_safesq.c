#include <criterion/criterion.h>

#include "board.h"
#include "move.h"

Test(SafeSQ, isAttackedBishop)
{
    _init_attacks_all();
    _init_castling_table();
    pos_t *pos = parse_fen("rnbqkbnr/pppp1ppp/8/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1");
    cr_assert(is_square_attacked(pos, d5, WHITE), "d5 should be attacked");
    cr_assert(is_square_attacked(pos, f7, WHITE), "f7 should be attacked");
    cr_assert(!is_square_attacked(pos, g8, WHITE), "g8 should not be attacked");
    free(pos);
}

Test(SafeSQ, isAttackedQueen)
{
    _init_attacks_all();
    _init_castling_table();
    pos_t *pos = parse_fen("rnb1k1nr/pppp1ppp/8/2b1p3/2B1P1Qq/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
    cr_assert(!is_square_attacked(pos, c8, WHITE), "c8 should not be attacked");
    cr_assert(!is_square_attacked(pos, g8, WHITE), "g8 should not be attacked");
    cr_assert(!is_square_attacked(pos, e1, BLACK), "e1 should not be attacked");
    cr_assert(is_square_attacked(pos, g7, WHITE), "g7 should be attacked");
    cr_assert(is_square_attacked(pos, f2, BLACK), "f2 should be attacked");
    cr_assert(is_square_attacked(pos, h2, BLACK), "h2 should be attacked");
    free(pos);
}

