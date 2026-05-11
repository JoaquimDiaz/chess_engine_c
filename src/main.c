#include <stdio.h>
#include "board.h"
#include "attack.h"
#include "move.h"

char *FEN_VIENNA = "r1b1k1nr/ppp2ppp/2np4/2bNp3/2B1P1Q1/8/PPPP1qPP/R1BK2NR w kq - 0 7";
char *FEN_PROM_CAP = "3q1q2/4P3/8/8/6k1/8/6K1/8 w - - 0 1";

int main(void)
{
    _init_attacks_all();
    _init_castling_table();

    // pos_t *pos = starting_pos();
    // make_move(pos, b1, c3, NO_FLAG, pos->side);
    // print_pos(pos);
    // print_pos_info(pos);
    // print_bb(BB(pos, WHITE, KNIGHT));
    // putchar('\n');
    // print_bb(pos->occ[WHITE]);

pos_t *pos = NULL;
    for (size_t depth = 0; depth < 11; depth++)
    {
        pos = starting_pos();
        uint64_t n = perft(pos, depth);
        printf("depth: %zu, perft: %lu\n", depth, n);
        // print_pos(pos);
        // print_pos_info(pos);
        // getchar();
    }
    free(pos);

}
