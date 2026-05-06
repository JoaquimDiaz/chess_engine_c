#include <stdio.h>
#include "board.h"
#include "attack.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/ppp2ppp/2np4/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR w kq - 0 7";
char *FEN_PROM_CAP = "3q1q2/4P3/8/8/6k1/8/6K1/8 w - - 0 1";

int main(void)
{
    
    _init_castling_table();
    _init_attacks_all();
    // pos_t *pos = starting_pos();
    pos_t *pos = parse_fen(FEN_PROM_CAP);
    ml_t pseudo_ml = {0};
    ml_t legal_ml  = {0};
    _all_pseudo_legal(pos, &pseudo_ml, pos->side);
    // gen_legal(pos, pos->side, &pseudo_ml, &legal_ml);
    PRINTML(pseudo_ml);
    // print_pos_info(pos);
    return 0;
}
