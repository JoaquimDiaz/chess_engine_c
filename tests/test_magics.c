#include <criterion/criterion.h>
#include "board.h"
#include "attack.h"

void _init_tables(void)
{
    _init_masks_rook();
    _init_masks_bishop();
    _init_offset_table();
    _init_rb_attack_tables();
}

Test(Magics, rookAttack)
{
    _init_tables();
    cr_assert(rook_attacks_fast(a1, sq_bb(a2)) & sq_bb(a2));
    cr_assert(rook_attacks_fast(a1, sq_bb(a2)) & ~sq_bb(a3));
}

// TODO: more tests
