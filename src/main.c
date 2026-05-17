#include "attack.h"
#include "board.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/ppp2ppp/2np4/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR w kq - 0 7";
char *FEN_PROM_CAP = "3q1q2/4P3/8/8/6k1/8/6K1/8 w - - 0 1";
char *PINNED_BISHOP = "r2qk1nr/ppp2bpp/2b5/4b2Q/B7/3P4/PPP1RPPP/RNBQK1NR b KQkq - 0 4";
char *CHECK_BLOCK = "8/4k1b1/1q6/8/5n2/8/4R3/2K5 b - - 0 1";
char *PAWN_PIN = "3k4/2ppp3/1B3Q2/8/8/8/2PPP3/3K4 b - - 0 1";
char *DB_BLOCK = "7k/2ppp3/1B6/8/8/2Q5/2PPP3/3K4 b - - 0 1";
char *DB_CHECK = "8/2ppp1k1/1B6/8/8/2Q2RR1/2PPP3/3K4 b - - 0 1";
char *EP_CHECK = "3k4/2p1p3/8/3pP3/4K3/8/2PP4/8 w - d6 0 2";

//TODO: hm increment

int main(void)
{
    // !!!!! DONT FORGET TO INIT THE ENGINE !!!!!!!
    init_engine();
    // !!!!!!!!
    // pos_t *pos = parse_fen(EP_CHECK);
    pos_t *pos = starting_pos();
    quick_make(pos, d2, d4);
    quick_make(pos, e7, e5);
    quick_make(pos, e1, d2);
    quick_make(pos, e5, e4);
    print_pos(pos);
    print_pos_info(pos);
    divide(pos, 1);
    // for (int i = 0; i < 6; i++) make_random(pos);
    // print_pos(pos);
    // ml_t ml = {0};
    // gen_legal(pos, &ml, pos->side);
    // PRINTML2(pos, ml);
    // pos->checkers[pos->side] = compute_checkers(pos, pos->side);
    // pos->pinned[pos->side] = compute_pin(pos, pos->side);
    // pos->attacked[pos->side] = attacked_squares(pos, pos->side); 
    // quick_make(pos, e2, e4);
    // quick_make(pos, d7, d5);
    // quick_make(pos, e4, e5);
    // quick_make(pos, f7, f5);
    // print_pos(pos);
    // print_pos_info(pos);
    // make_move(pos, e5, f6, EN_PASSANT, pos->side);
    // unmake_move(pos, e5, f6, EN_PASSANT);
    // gen_legal(pos, &ml, pos->side);
    // PRINTML(ml);
    // ITER_ML(ml, i) {
    //     if (mfrom(ml.moves[i]) == e5) print_move(ml.moves[i]);
    //     print_move(ml.moves[i]);
    // }

    // pos_t * pos = starting_pos();
    // while (1)
    // {
    //     make_random(pos);
    //     print_pos(pos);
    //     print_pos_info(pos);
    //     getchar();
    // }
}
