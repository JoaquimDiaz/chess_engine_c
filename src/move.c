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
        if (push & ((c == WHITE) ? RANK_8 : RANK_1))
        {
            int to_sq = poplsb(&push);
            ml->moves[ml->count++] = mencode(sq, to_sq, PROM_N);
            ml->moves[ml->count++] = mencode(sq, to_sq, PROM_B);
            ml->moves[ml->count++] = mencode(sq, to_sq, PROM_R);
            ml->moves[ml->count++] = mencode(sq, to_sq, PROM_Q);
        }
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
    make_move(pos, from, to, flag, pos->side);
}

void make_move(pos_t *pos, int from, int to, int flag, color_t c)
{
    pos->ply_stack[pos->ply++] = (saved_t){pos->pl[to], pos->castling, pos->ep, pos->hm};
    if (piece_type(pos->pl[from]) == KING)
        pos->ks[c] = to;
    // bitboard
    BB(pos, c, piece_type(pos->pl[from])) &= ~sq_bb(from);
    BB(pos, c, piece_type(pos->pl[from])) |= sq_bb(to);
    pos->occ[c] &= ~sq_bb(from);
    pos->occ[c] |= sq_bb(to);
    pos->all &= ~sq_bb(from);
    if (flag == CAPTURE) {
        pos->occ[c^1] &= ~sq_bb(to);
        BB(pos, c^1, piece_type(pos->pl[to])) &= ~sq_bb(to);
    } else {
        pos->all |= sq_bb(to);
    }
    // piece list
    pos->pl[to]   = pos->pl[from];
    pos->pl[from] = NO_PIECE;
    // pos info
    //TODO: FLAGS
    if (c == BLACK) pos->fm++;
    flip_side(pos);
}

//TODO: UNMAKE

int is_king_safe(pos_t *pos, color_t c)
{
    int sq = pos->ks[c];
    assert((unsigned char)sq < 64);
    return !((ATTACKS_PAWN[c][sq]   & BB(pos, c^1, PAWN))
            | (ATTACKS_KNIGHT[sq]   & BB(pos, c^1, KNIGHT))
            | (ATTACKS_KING[sq]     & BB(pos, c^1, KING))
            | (rook_attacks_fast(sq, pos->all)   & (BB(pos, c^1, ROOK)   | BB(pos, c^1, QUEEN)))
            | (bishop_attacks_fast(sq, pos->all) & (BB(pos, c^1, BISHOP) | BB(pos, c^1, QUEEN))));
    //TODO: CASTLING
}

void unmake_move(pos_t *pos, int from, int to, color_t c)
{
    saved_t s = pos->ply_stack[pos->ply--];
    BB(pos, c, piece_type(pos->pl[to])) &= ~sq_bb(to);
    BB(pos, c, piece_type(pos->pl[to])) |=  sq_bb(from);
    pos->occ[c] |=  sq_bb(from);
    pos->occ[c] &= ~sq_bb(to);
    pos->pl[from] = pos->pl[to];
    if (piece_type(pos->pl[to]) == KING) pos->ks[c] = from;
    if (s.captured) {
        pos->pl[to] = s.captured;
        BB(pos, c^1, piece_type(s.captured)) |= sq_bb(to);
    } else {
        pos->pl[to] = NO_PIECE;
        pos->all &= ~sq_bb(to);
    }
    pos->castling = s.castling;
    pos->ep       = s.ep;
    pos->hm       = s.hm;
    if (pos->side == WHITE) pos->fm--;
    flip_side(pos);
}

void gen_legal(pos_t *pos, color_t c, ml_t *ml_pseudo, ml_t *ml_legal)
{
    move_t *p = ml_legal->moves;
    for (size_t i = 0; i < ml_pseudo->count; i++) {
        move_t m = ml_pseudo->moves[i];
        int from = mfrom(m);
        int to   = mto(m);
        int flag = mflag(m);
        make_move(pos, from, to, flag, c);
        if (is_king_safe(pos, c)) {
            *p++ = ml_pseudo->moves[i];
            ml_legal->count++;
        }
        unmake_move(pos, from, to, c);
    }
}
