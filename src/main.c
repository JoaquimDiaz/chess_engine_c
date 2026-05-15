#include "attack.h"
#include "board.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/ppp2ppp/2np4/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR w kq - 0 7";
char *FEN_PROM_CAP = "3q1q2/4P3/8/8/6k1/8/6K1/8 w - - 0 1";

//TODO: hm increment

int main(void)
{
    // !!!!! DONT FORGET TO INIT THE ENGINE !!!!!!!
    init_engine();
    ITER_SQ(sq) {
        print_bb(BLOCKERS[a1][sq]);
        getchar();
    }
    // pos_t * pos = starting_pos();
    // while (1)
    // {
    //     make_random(pos);
    //     print_pos(pos);
    //     print_pos_info(pos);
    //     getchar();
    // }
    // make_move(pos, e2, e4, DOUBLE_PAWN, pos->side);
    // make_move(pos, d7, d5, DOUBLE_PAWN, pos->side);
    // make_move(pos, e4, e5, NO_FLAG, pos->side);
    // make_move(pos, f7, f5, DOUBLE_PAWN, pos->side);
    // make_move(pos, e5, f6, EN_PASSANT, pos->side);
    // unmake_move(pos, e5, f6, EN_PASSANT);
    // ITER_BB(c, p) {
    //     printf("Piece: '%i', Color: %s\n", p, (c==WHITE)?"WHITE":"BLACK");
    //     print_bb(BB(pos, c, p));
    // }
    // print_bb(pos->occ[WHITE]);
    // print_bb(pos->occ[BLACK]);
    // print_bb(pos->all);
    // make_move(pos, e7, e6, NO_FLAG, pos->side);
    // make_move(pos, f6, f6, NO_FLAG, pos->side);
    // print_pos(pos);
    // print_pos_info(pos);
    // compute_pin(pos, pos->side);
    // compute_checkers(pos, pos->side);
    // print_bb(pos->pinned[pos->side]);
    // print_bb(pos->pinners[pos->side^1]);
    // print_bb(pos->checkers[pos->side]);
}
