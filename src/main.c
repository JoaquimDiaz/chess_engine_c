#include <stdio.h>
#include "board.h"
#include "attack.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/ppp2ppp/2np4/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR w kq - 0 7";

int main(void)
{
    _init_attacks_all();
    // pos_t *pos = starting_pos();
    pos_t *pos = parse_fen(FEN_VIENNA);
    ml_t pseudo_ml = {0};
    ml_t legal_ml  = {0};
    // make_move(pos, e2, e4, NO_FLAG, pos->side);
    // make_move(pos, e7, e5, NO_FLAG, pos->side);
    // make_move(pos, c2, c4, NO_FLAG, pos->side);
    _all_pseudo_legal(pos, &pseudo_ml, pos->side);
    gen_legal(pos, pos->side, &pseudo_ml, &legal_ml);
    for (size_t i = 0; i < pseudo_ml.count; i++) {
        print_move(pseudo_ml.moves[i]);
    }
    // for (size_t i = 0; i < legal_ml.count; i++) {
    //     print_move(legal_ml.moves[i]);
    // }
    printf("pseudo count: %zu\n", pseudo_ml.count);
    printf("legal count: %zu\n", legal_ml.count);
    print_pos(pos);
    return 0;
}
