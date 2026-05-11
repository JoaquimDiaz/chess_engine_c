#include <criterion/criterion.h>

#include "board.h"
#include "attack.h"
#include "move.h"

Test(EnPassant, Black)
{
    _init_attacks_all();
    _init_castling_table();
    pos_t *pos = parse_fen("4k3/ppppp1pp/8/8/4Pp2/5P2/PPPP2PP/4K3 b - e3 0 1");
    ml_t ml = {0};
    _pseudo_legal_pawn(pos, &ml, pos->side);
    int found = 0;
    for (size_t i = 0; i < ml.count; i++) {
        move_t m = ml.moves[i];
        if (mfrom(m) == f4 && mto(m) == e3 && mflag(m) == EN_PASSANT) found++;
    }
    cr_assert(found, "En passant move not found");
    free(pos);
}

Test(EnPassant, White)
{
    _init_attacks_all();
    _init_castling_table();
    pos_t *pos = parse_fen("4k3/ppp1p1pp/8/3pP3/8/5P2/PPPP2PP/4K3 w - d6 0 2");
    ml_t ml = {0};
    _pseudo_legal_pawn(pos, &ml, pos->side);
    int found = 0;
    for (size_t i = 0; i < ml.count; i++) {
        move_t m = ml.moves[i];
        if (mfrom(m) == e5 && mto(m) == d6 && mflag(m) == EN_PASSANT) found++;
    }
    cr_assert(found, "En passant move not found");
    free(pos);
}
