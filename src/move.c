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

// Check if square attacked by given color
int is_square_attacked(pos_t *pos, int sq, color_t c)
{
    assert((unsigned char)sq < SQ_COUNT);
    return (ATTACKS_KNIGHT[sq] & BB(pos, c, KNIGHT)
            || bishop_attacks_fast(sq, pos->all) & (BB(pos, c, BISHOP) | BB(pos, c, QUEEN))
            || rook_attacks_fast(sq, pos->all) & (BB(pos, c, ROOK) | BB(pos, c, QUEEN))
            || ATTACKS_PAWN[c][sq] & BB(pos, c, PAWN)
            || ATTACKS_KING[sq] & BB(pos, c, KING));
}

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
            ml_add(ml, sq, to_sq, PROM_N);
            ml_add(ml, sq, to_sq, PROM_B);
            ml_add(ml, sq, to_sq, PROM_R);
            ml_add(ml, sq, to_sq, PROM_Q);
        }
        // double push
        bb_t db_push = (c == WHITE) ? ((push << 8) & RANK_4) : ((push >> 8) & RANK_5);
        db_push &= ~pos->all;
        if (db_push) ml_add(ml, sq, poplsb(&db_push), DOUBLE_PAWN);
        if (push) ml_add(ml, sq, poplsb(&push), NO_FLAG);
        //TODO: improve?
        // captures
        bb_t captures = ATTACKS_PAWN[c][sq] & pos->occ[c^1];
        if (captures & ((c == WHITE) ? RANK_8 : RANK_1)) 
            while (captures) 
            {
                int to_sq = poplsb(&captures);
                ml_add(ml, sq, to_sq, PROM_CAP_N);
                ml_add(ml, sq, to_sq, PROM_CAP_B);
                ml_add(ml, sq, to_sq, PROM_CAP_R);
                ml_add(ml, sq, to_sq, PROM_CAP_Q);
            }
        else 
            while (captures) ml_add(ml, sq, poplsb(&captures), CAPTURE);
        if (pos->ep != NO_ENPASSANT && (sq_bb(pos->ep) & ATTACKS_PAWN[c][sq]))
            ml_add(ml, sq, pos->ep, EN_PASSANT);
    }
}

void _pseudo_legal_knight(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, KNIGHT);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t captures = ATTACKS_KNIGHT[sq] & pos->occ[c^1];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        bb_t quiet_moves = ATTACKS_KNIGHT[sq] & ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _pseudo_legal_king(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, KING);
    int sq = poplsb(&bb);
    bb_t captures = ATTACKS_KING[sq] & pos->occ[c^1];
    while (captures) {
        ml_add(ml, sq, poplsb(&captures), CAPTURE);
    }
    bb_t quiet_moves = ATTACKS_KING[sq] & ~pos->all;
    while (quiet_moves) {
        ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
    }
    if (c == WHITE) {
        if ((pos->castling & W_00) 
                && !(pos->all & MASK_W00)
                && !is_square_attacked(pos, f1, BLACK)
                && !is_square_attacked(pos, g1, BLACK)
                && !is_square_attacked(pos, e1, BLACK))
            ml_add(ml, sq, g1, CASTLE_KING);
        if ((pos->castling & W_000) && !(pos->all & MASK_W000)
                && !is_square_attacked(pos, c1, BLACK)
                && !is_square_attacked(pos, d1, BLACK)
                && !is_square_attacked(pos, e1, BLACK))
            ml_add(ml, sq, c1, CASTLE_QUEEN);
    } else {
        if ((pos->castling & B_00) && !(pos->all & MASK_B00)
                && !is_square_attacked(pos, f8, WHITE)
                && !is_square_attacked(pos, g8, WHITE)
                && !is_square_attacked(pos, e8, WHITE))
            ml_add(ml, sq, g8, CASTLE_KING);
        if ((pos->castling & B_000) && !(pos->all & MASK_B000)
                && !is_square_attacked(pos, c8, WHITE)
                && !is_square_attacked(pos, d8, WHITE)
                && !is_square_attacked(pos, e8, WHITE))
            ml_add(ml, sq, c8, CASTLE_QUEEN);
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
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void _pseudo_legal_bishop(pos_t *pos, ml_t *ml, color_t c)
{
    bb_t bb = BB(pos, c, BISHOP);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t quiet_moves = bishop_attacks_fast(sq, pos->all);
        bb_t captures = quiet_moves & pos->occ[c^1];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
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
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
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
    // save board state
    pos->ply_stack[++pos->ply] = (saved_t){ pos->pl[to], pos->castling, pos->ep, pos->hm };
    if (piece_type(pos->pl[from]) == KING) pos->ks[c] = to;
    switch(flag) {
        case DOUBLE_PAWN: 
            pos->ep = (c == WHITE) ? (to - 8) : (to + 8); 
            break;
        case EN_PASSANT: 
            int sq_ep = (c == WHITE) ? pos->ep - 8 : pos->ep + 8;
            BB(pos, c^1, PAWN) &= ~sq_bb(sq_ep);
            pos->ply_stack[pos->ply].captured = pos->pl[sq_ep];
            pos->pl[sq_ep] = NO_PIECE;
            // pos->occ[c]   |= sq_bb(sq_ep); 
            pos->occ[c^1] &= ~sq_bb(sq_ep); 
            pos->ep = NO_ENPASSANT;
            break;
        case CASTLE_KING: 
            // Would it be better to declare sq_r outside?
            int sq_r = (c == WHITE) ? h1 : h8;
            BB(pos, c, ROOK) &= ~sq_bb(sq_r); 
            BB(pos, c, ROOK) |= sq_bb(sq_r - 2); 
            pos->pl[sq_r - 2] = pos->pl[sq_r];
            pos->pl[sq_r] = NO_PIECE;
            pos->occ[c] &= ~sq_bb(sq_r);
            pos->occ[c] |= sq_bb(sq_r - 2);
            pos->ep = NO_ENPASSANT;
            break;
        case CASTLE_QUEEN: 
            sq_r = (c == WHITE) ? a1 : a8;
            BB(pos, c, ROOK) &= ~sq_bb(sq_r); 
            BB(pos, c, ROOK) |= sq_bb(sq_r + 3); 
            pos->pl[sq_r + 3] = pos->pl[sq_r];
            pos->pl[sq_r] = NO_PIECE;
            pos->occ[c] &= ~sq_bb(sq_r);
            pos->occ[c] |= sq_bb(sq_r + 3);
            pos->ep = NO_ENPASSANT;
            break;
        default: 
            pos->ep = NO_ENPASSANT;
            break;
    }
    // from
    BB(pos, c, piece_type(pos->pl[from])) &= ~sq_bb(from);
    pos->occ[c] &= ~sq_bb(from);
    if (is_capture(flag)) {
        BB(pos, c^1, piece_type(pos->pl[to])) &= ~sq_bb(to);
        pos->occ[c^1] &= ~sq_bb(to);
    } 
    if (is_prom(flag)) {
        int p = prom_piece(flag);
        BB(pos, c, p) |= sq_bb(to);
        pos->pl[to] = make_piece(c, p);
    } else {
        BB(pos, c, piece_type(pos->pl[from])) |= sq_bb(to);
        pos->occ[c] |= sq_bb(to);
    }
    pos->pl[to]   = pos->pl[from];
    pos->pl[from] = NO_PIECE;
    pos->all = pos->occ[c] | pos->occ[c^1];
    //pos info
    //TODO: half move
    pos->castling &= (CASTLING_TABLE[from] & CASTLING_TABLE[to]);
    if (pos->side == BLACK) pos->fm++;
    flip_side(pos);
}

void unmake_move(pos_t *pos, int from, int to, int flag)
{
    color_t c = pos->side^1;
    saved_t s = pos->ply_stack[pos->ply--];
    if (piece_type(pos->pl[to]) == KING) pos->ks[c] = from;
    switch(flag) {
        case EN_PASSANT: 
            int sq_ep = (c == WHITE) ? s.ep - 8 : s.ep + 8;
            BB(pos, c^1, PAWN) |= sq_bb(sq_ep);
            pos->pl[sq_ep] = make_piece(c^1, PAWN);
            pos->occ[c^1] |= sq_bb(sq_ep); 
            break;
        case CASTLE_KING: 
            // Would it be better to declare sq_r outside?
            int sq_r = (c == WHITE) ? h1 : h8;
            BB(pos, c, ROOK) |= sq_bb(sq_r); 
            BB(pos, c, ROOK) &= ~sq_bb(sq_r - 2); 
            pos->pl[sq_r]     = pos->pl[sq_r - 2];
            pos->pl[sq_r - 2] = NO_PIECE;
            pos->occ[c] |= sq_bb(sq_r);
            pos->occ[c] &= ~sq_bb(sq_r - 2);
            break;
        case CASTLE_QUEEN: 
            sq_r = (c == WHITE) ? a1 : a8;
            BB(pos, c, ROOK) |= sq_bb(sq_r); 
            BB(pos, c, ROOK) &= ~sq_bb(sq_r + 3); 
            pos->pl[sq_r] = pos->pl[sq_r + 3];
            pos->pl[sq_r + 3] = NO_PIECE;
            pos->occ[c] |= sq_bb(sq_r);
            pos->occ[c] &= ~sq_bb(sq_r + 3);
            break;
        default: 
            break;
    }
    // from
    BB(pos, c, piece_type(pos->pl[to])) |= sq_bb(from);
    pos->occ[c] |= sq_bb(from);
    pos->occ[c] &= ~sq_bb(to);
    pos->pl[from] = pos->pl[to];
    if (is_capture(flag)) {
        BB(pos, c, piece_type(pos->pl[to])) &= ~sq_bb(to);
        BB(pos, c^1, piece_type(s.captured)) |= sq_bb(to);
        pos->pl[to] = s.captured;
        pos->occ[c^1] |= sq_bb(to);
    } else {
        BB(pos, c, piece_type(pos->pl[to])) &= ~sq_bb(to);
        pos->pl[to] = NO_PIECE;
    }
    if (is_prom(flag))
    {
        int p = prom_piece(flag);
        BB(pos, c, p) &= ~sq_bb(to);
        BB(pos, c, PAWN) |= sq_bb(from);
        pos->pl[to] = NO_PIECE;
        pos->pl[from] = make_piece(c, PAWN);
    }
    pos->all = pos->occ[c] | pos->occ[c^1];
    //pos info
    pos->castling = s.castling;
    pos->ep       = s.ep;
    pos->hm       = s.hm;
    if (pos->side == WHITE) pos->fm--;
    flip_side(pos);
}

int is_king_safe(pos_t *pos, color_t c)
{
    int sq = pos->ks[c];
    assert((unsigned char)sq < 64);
    return !((ATTACKS_PAWN[c][sq]   & BB(pos, c^1, PAWN))
            | (ATTACKS_KNIGHT[sq]   & BB(pos, c^1, KNIGHT))
            | (ATTACKS_KING[sq]     & BB(pos, c^1, KING))
            | (rook_attacks_fast(sq, pos->all)   & (BB(pos, c^1, ROOK)   | BB(pos, c^1, QUEEN)))
            | (bishop_attacks_fast(sq, pos->all) & (BB(pos, c^1, BISHOP) | BB(pos, c^1, QUEEN))));
}

void gen_legal(pos_t *pos, color_t c, ml_t *ml_legal)
{
    ml_t ml_pseudo = {0};
    _all_pseudo_legal(pos, &ml_pseudo, c);
    move_t *p = ml_legal->moves;
    for (size_t i = 0; i < ml_pseudo.count; i++) {
        move_t m = ml_pseudo.moves[i];
        int from = mfrom(m);
        int to   = mto(m);
        int flag = mflag(m);
        make_move(pos, from, to, flag, c);
        if (is_king_safe(pos, c)) {
            *p++ = ml_pseudo.moves[i];
            ml_legal->count++;
        }
        unmake_move(pos, from, to, flag);
    }
}

/* ----------------------------------------------------------------------------
 * # PERFT
 * ------------------------------------------------------------------------- */

uint64_t perft(pos_t *pos, int depth)
{
    if (depth == 0) 
      return 1ULL;

    ml_t move_list = {0};
    uint64_t nodes = 0ull;

    gen_legal(pos, pos->side, &move_list);
    if (depth == 1)
        return move_list.count;

    for (size_t i = 0; i < move_list.count; i++) {
        move_t m = move_list.moves[i];
        int from = mfrom(m);
        int to   = mto(m);
        int flag = mflag(m);
        make_move(pos, from, to, flag, pos->side);
        nodes += perft(pos, depth - 1);
        unmake_move(pos, from, to, flag);
    }
    return nodes;
}
