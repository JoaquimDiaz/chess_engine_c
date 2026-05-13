#include <criterion/criterion.h>

#include "board.h"
#include "move.h"

Test(PawnMove, WhitePawnProm)
{
    init_engine();
    pos_t *pos = parse_fen("3q1q2/4P3/8/8/6k1/8/6K1/8 w - - 0 1");
    ml_t mlp = {0};
    ml_t mll = {0};
    gen_legal(pos, pos->side, &mlp, &mll);

    move_t to_find[] = {
        mencode(e7, e8, PROM_N),   mencode(e7, e8, PROM_B),
        mencode(e7, e8, PROM_R),   mencode(e7, e8, PROM_Q),
        mencode(e7, d8, PROM_CAP_N), mencode(e7, d8, PROM_CAP_B),
        mencode(e7, d8, PROM_CAP_R), mencode(e7, d8, PROM_CAP_Q),
        mencode(e7, f8, PROM_CAP_N), mencode(e7, f8, PROM_CAP_B),
        mencode(e7, f8, PROM_CAP_R), mencode(e7, f8, PROM_CAP_Q),
    };
    int found[12] = {0};
    // LEGAL
    for (size_t i = 0; i < 12; i++) found[i] = 0;
    for (size_t i = 0; i < mll.count; i++)
    {
        move_t m = mll.moves[i];
        for (size_t j = 0; j < 12; j++)
            if (m == to_find[j]) { found[j]++; break; }
    }
    for (size_t i = 0; i < 12; i++)
        cr_assert(found[i], "move %zu not found", i);
}

Test(PawnMove, BlackPawnProm)
{
    init_engine();
    pos_t *pos = parse_fen("8/1k6/8/1K6/8/8/3p4/2R1B3 b - - 0 1");
    ml_t mlp = {0};
    ml_t mll = {0};
    gen_legal(pos, pos->side, &mlp, &mll);
    move_t to_find[] = {
        mencode(d2, d1, PROM_N), mencode(d2, d1, PROM_B),
        mencode(d2, d1, PROM_R), mencode(d2, d1, PROM_Q),
        mencode(d2, c1, PROM_CAP_N), mencode(d2, c1, PROM_CAP_B),
        mencode(d2, c1, PROM_CAP_R), mencode(d2, c1, PROM_CAP_Q),
        mencode(d2, e1, PROM_CAP_N), mencode(d2, e1, PROM_CAP_B),
        mencode(d2, e1, PROM_CAP_R), mencode(d2, e1, PROM_CAP_Q)
    };
    int found[12] = {0};
    // LEGAL
    for (size_t i = 0; i < 12; i++) found[i] = 0;
    for (size_t i = 0; i < mll.count; i++){
        move_t m = mll.moves[i];
        for (size_t j = 0; j < 12; j++)
            if (m == to_find[j]) { found[j]++; break; }
    }
    for (size_t i = 0; i < 12; i++)
        cr_assert(found[i], "move %zu not found", i);
}
