#include <stdio.h>
#include "board.h"
#include "attack.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/pppp1ppp/2n5/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR b kq - 1 6";

int main(void)
{
    _init_attacks_all();
    pos_t *pos = starting_pos();
    ml_t   ml  = {0};
    _all_pseudo_legal(pos, &ml, pos->side);
    if (ml.count == 0) printf("NO MOVES\n");
    for (size_t i = 0; i < ml.count; i++) {
        print_move(ml.moves[i]);
    }
    return 0;
}
