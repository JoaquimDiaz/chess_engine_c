#include "board.h"
#include "move.h"

int main(void)
{
    init_engine();
    pos_t *pos = starting_pos();
    perft_root(pos, 7);
    free(pos);
    return 0;
}
