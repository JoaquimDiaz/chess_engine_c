#include "move.h"
#include "attack.h"
#include "board.h"

/* ----------------------------------------------------------------------------
 * # HELPERS
 * ------------------------------------------------------------------------- */

static char *_decode_flag(uint32_t flag)
{
    switch (flag) {
        case NO_FLAG:       return "NO_FLAG";
        case DOUBLE_PAWN:   return "DOUBLE_PAWN";
        case CASTLE_KING:   return "CASTLE_KING";
        case CASTLE_QUEEN:  return "CASTLE_QUEEN";
        case CAPTURE:       return "CAPTURE";
        case EN_PASSANT:    return "EN_PASSANT";
        case PROM_N:        return "PROM_N";
        case PROM_B:        return "PROM_B";
        case PROM_R:        return "PROM_R";
        case PROM_Q:        return "PROM_Q";
        case PROM_CAP_N:    return "PROM_CAP_N";
        case PROM_CAP_B:    return "PROM_CAP_B";
        case PROM_CAP_R:    return "PROM_CAP_R";
        case PROM_CAP_Q:    return "PROM_CAP_Q";
        default: return "NO_FLAG";
    }
}

void print_move(move_t m)
{
    printf("from: '%s' | ", sq_to_str(mfrom(m)));
    printf("to: '%s' |", sq_to_str(mto(m)));
    printf("flag: '%s'\n", _decode_flag(mflag(m)));
}

/* ----------------------------------------------------------------------------
 * # PSEUDO LEGAL GENERATION
 * ------------------------------------------------------------------------- */

void _pseudo_legal_pawn(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, PAWN);
    while (bb) {
        int sq = poplsb(&bb);
        // single push
        bb_t push = (c == WHITE) ? (sq_bb(sq) << 8) : (sq_bb(sq) >> 8);
        push &= ~pos->all;
        // TODO: PROMOTIONS
        // double push
        bb_t db_push = (c == WHITE) ? ((push << 8) & RANK_4) : ((push >> 8) & RANK_5);
        db_push &= ~pos->all;
        if (db_push) ml->moves[ml->count++] = mencode(sq, poplsb(&db_push), DOUBLE_PAWN);
        if (push) ml->moves[ml->count++] = mencode(sq, poplsb(&push), NO_FLAG);
        // captures
        bb_t captures = ATTACKS_PAWN[c][sq] & pos->occ[c^1];
        while (captures) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&captures), CAPTURE);
        }
    }
}

void _pseudo_legal_knight(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, KNIGHT);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t captures = ATTACKS_KNIGHT[sq] & pos->occ[c^1];
        while (captures) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&captures), CAPTURE);
        }
        bb_t quiet_moves = ATTACKS_KNIGHT[sq] & ~pos->all;
        while (quiet_moves) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _pseudo_legal_king(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, KING);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t captures = ATTACKS_KING[sq] & pos->occ[c^1];
        while (captures) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&captures), CAPTURE);
        }
        bb_t quiet_moves = ATTACKS_KING[sq] & ~pos->all;
        while (quiet_moves) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _pseudo_legal_rook(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, ROOK);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t quiet_moves = rook_attacks_fast(sq, pos->all);
        bb_t captures = quiet_moves & pos->occ[c^1];
        while (captures) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _pseudo_legal_bishop(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, ROOK);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t quiet_moves = bishop_attacks_fast(sq, pos->all);
        bb_t captures = quiet_moves & pos->occ[c^1];
        while (captures) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _pseudo_legal_queen(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, QUEEN);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t quiet_moves = bishop_attacks_fast(sq, pos->all) | rook_attacks_fast(sq, pos->all);
        bb_t captures = quiet_moves & pos->occ[c^1];
        while (captures) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml->moves[ml->count++] = mencode(sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _all_pseudo_legal(pos_t *pos, ml_t *ml, color_t c)
{
    _pseudo_legal_pawn(pos, ml, c);
    _pseudo_legal_knight(pos, ml, c);
    _pseudo_legal_king(pos, ml, c);
    _pseudo_legal_rook(pos, ml, c);
    _pseudo_legal_bishop(pos, ml, c);
    _pseudo_legal_queen(pos, ml, c);
}

/* ----------------------------------------------------------------------------
 * # MAKE / UNMAKE / LEGAL
 * ------------------------------------------------------------------------- */

void quick_make(pos_t *pos, int from, int to)
{
    int flag = NO_FLAG;
    if (pos->pl[to] != NO_PIECE) flag = CAPTURE;
    make_move(pos, mencode(from, to, flag));
}

void make_move(pos_t *pos, move_t m)
{
    int from = mfrom(m);
    int to   = mto(m);
    if (piece_type(pos->pl[from]) == KING)
        pos->ks[pos->side] = to;
    // bitboard
    BB(pos, pos->side, pos->pl[from]) &= ~sq_bb(from);
    BB(pos, pos->side, pos->pl[from]) |= sq_bb(to);
    // occupancy
    pos->occ[pos->side] &= ~sq_bb(from);
    pos->occ[pos->side] |= sq_bb(to);
    pos->all &= ~sq_bb(from);
    if (mflag(m) == CAPTURE) {
        pos->occ[pos->side^1] &= ~sq_bb(to);
        BB(pos, pos->side^1, pos->pl[to]) &= ~sq_bb(to);
    } else {
        pos->all |= sq_bb(to);
    }
    // piece list
    pos->pl[to]   = pos->pl[from];
    pos->pl[from] = NO_PIECE;
    // pos info
    //TODO: CASTLING &
    if (pos->side == BLACK) pos->fm++;
    flip_side(pos);
}

//TODO: UNMAKE

int is_king_safe(pos_t *pos, color_t c)
{
    int sq = pos->ks[c];
    assert((unsigned char)sq < 64);
    return !((ATTACKS_PAWN[c^1][sq] & BB(pos, c^1, PAWN))
            | (ATTACKS_KNIGHT[sq]   & BB(pos, c^1, KNIGHT))
            | (ATTACKS_KING[sq]     & BB(pos, c^1, KING))
            | (rook_attacks_fast(sq, pos->all)   & (BB(pos, c^1, ROOK)   | BB(pos, c^1, QUEEN)))
            | (bishop_attacks_fast(sq, pos->all) & (BB(pos, c^1, BISHOP) | BB(pos, c^1, QUEEN))));
    //TODO: CASTLING
}

void unmake_move(pos_t *pos)
{
    // pl[from] = pl[to]
    // pl[to]   = saved->captured
    // 
}

int gen_legal(pos_t *pos, color_t c, ml_t *ml_pseudo, ml_t *ml_legal)
{
    move_t *p = ml_legal->moves;
    for (size_t i = 0; i < ml_pseudo->count; i++) {
        make_move(pos, ml_pseudo->moves[i]);
        if (is_king_safe(pos, c)) {
            *p++ = ml_pseudo->moves[i];
            ml_legal->count++;
        }
        // unmake_move()
    }
}
