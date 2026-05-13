#include <stdio.h>
#include "board.h"
#include "attack.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/ppp2ppp/2np4/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR w kq - 0 7";
char *FEN_PROM_CAP = "3q1q2/4P3/8/8/6k1/8/6K1/8 w - - 0 1";

//TODO: hm increment

int main(void)
{
    // !!!!! DONT FORGET TO INIT THE ENGINE !!!!!!!
    init_engine();
    print_bb(MASK_B00_SAFE);
    print_bb(MASK_B000_SAFE);
    print_bb(MASK_W00_SAFE);
    print_bb(MASK_W000_SAFE);
    // pos_t *pos = parse_fen("1k1n2QR/8/8/1R6/5Q2/4K1N1/8/8 b - - 0 1");
    // print_pos(pos);
    // print_pos_info(pos);
    // compute_pin(pos, pos->side);
    // compute_checkers(pos, pos->side);
    // print_bb(pos->pinned[pos->side]);
    // print_bb(pos->pinners[pos->side^1]);
    // print_bb(pos->checkers[pos->side]);
}
