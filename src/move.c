#include "move.h"
#include "attack.h"
#include "board.h"

/* ----------------------------------------------------------------------------
 * # LEGAL GENERATION
 * 
 * - Move generation if there is no check on the king.
 * - Include filtering moves for pinned pieces.
 * ------------------------------------------------------------------------- */

void gen_legal_pawn(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, PAWN);
    while (bb) {
        int sq = poplsb(&bb);
        // Pinned? king sq -> pinner sq ray : else full
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ? 
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t push = ((us == WHITE) ? (sq_bb(sq) << 8) : (sq_bb(sq) >> 8)) & ~pos->all;
        bb_t db_push = ((us == WHITE) ? ((push << 8) & RANK_4) : ((push >> 8) & RANK_5)) & ~pos->all;
        push &= pin_ray;
        db_push &= pin_ray;
        if (push & ((us == WHITE) ? RANK_8 : RANK_1))
        {
            int to_sq = poplsb(&push);
            ml_add(ml, sq, to_sq, PROM_N);
            ml_add(ml, sq, to_sq, PROM_B);
            ml_add(ml, sq, to_sq, PROM_R);
            ml_add(ml, sq, to_sq, PROM_Q);
        }
        // double push
        if (db_push) ml_add(ml, sq, poplsb(&db_push), DOUBLE_PAWN);
        if (push) ml_add(ml, sq, poplsb(&push), NO_FLAG);
        //TODO: improve?
        // captures
        bb_t captures = ATTACKS_PAWN[us][sq] & pos->occ[them] & pin_ray;
        if (captures & ((us == WHITE) ? RANK_8 : RANK_1)) 
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
        if (pos->ep != NO_ENPASSANT && (sq_bb(pos->ep) & ATTACKS_PAWN[us][sq] & pin_ray))
        {
            //TODO: TEST
            //TODO: pin_ray
            if (!(sq_bb(sq) & RANKS[pos->ks[us] >> 3])
                    || !(rook_attacks_fast(pos->ks[us], pos->all & ~(sq_bb(sq) 
                                | sq_bb(pos->ep + ((us == WHITE) ? (-8) : (8))))
                            & (BB(pos, them, ROOK) | BB(pos, them, QUEEN)))))
                ml_add(ml, sq, pos->ep, EN_PASSANT);
        }
    }
}

void gen_legal_knight(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, KNIGHT);
    while (bb) {
        int sq = poplsb(&bb);
        // skip move generation if knight is pinned (can't move if pinned)
        if (pos->pinned[us] & sq_bb(sq)) continue;
        bb_t captures = ATTACKS_KNIGHT[sq] & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        bb_t quiet_moves = ATTACKS_KNIGHT[sq] & ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_legal_bishop(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, BISHOP);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ? 
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t quiet_moves = bishop_attacks_fast(sq, pos->all) & pin_ray;
        bb_t captures = quiet_moves & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_legal_rook(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, ROOK);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ?
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t quiet_moves = rook_attacks_fast(sq, pos->all) & pin_ray;
        bb_t captures = quiet_moves & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_legal_queen(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, QUEEN);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ?
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t quiet_moves = (bishop_attacks_fast(sq, pos->all) 
                | rook_attacks_fast(sq, pos->all)) & pin_ray;
        bb_t captures = quiet_moves & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_legal_king(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    int k_sq = pos->ks[us];
    int to_sq;
    bb_t occ_nok = pos->all & ~sq_bb(k_sq);
    // captures
    bb_t captures = ATTACKS_KING[k_sq] & pos->occ[them];
    while (captures) {
        to_sq = poplsb(&captures);
        if (!is_square_attacked(pos, to_sq, them, occ_nok))
            ml_add(ml, k_sq, to_sq, CAPTURE);
    }
    // quiet moves
    bb_t quiet_moves = ATTACKS_KING[k_sq] & ~pos->all;
    while (quiet_moves) {
        to_sq = poplsb(&quiet_moves);
        if (!is_square_attacked(pos, to_sq, them, occ_nok))
            ml_add(ml, k_sq, to_sq, NO_FLAG);
    }
    // castling
    if (us == WHITE) {
        if ((pos->castling & W_00) 
                && !(pos->all & MASK_W00)
                // && !(pos->attacked[us] & MASK_W00_SAFE))
                && !is_square_attacked(pos, f1, them, occ_nok)
                && !is_square_attacked(pos, g1, them, occ_nok))
            ml_add(ml, k_sq, g1, CASTLE_KING);
        if ((pos->castling & W_000) 
                && !(pos->all & MASK_W000)
                // && !(pos->attacked[us] & MASK_W000_SAFE))
                && !is_square_attacked(pos, d1, them, occ_nok)
                && !is_square_attacked(pos, c1, them, occ_nok))
            ml_add(ml, k_sq, c1, CASTLE_QUEEN);
    } else {
        if ((pos->castling & B_00) 
                && !(pos->all & MASK_B00)
                // && !(pos->attacked[us] & MASK_B00_SAFE))
                && !is_square_attacked(pos, f8, them, occ_nok)
                && !is_square_attacked(pos, g8, them, occ_nok))
            ml_add(ml, k_sq, g8, CASTLE_KING);
        if ((pos->castling & B_000) 
                && !(pos->all & MASK_B000)
                // && !(pos->attacked[us] & MASK_B000_SAFE))
                && !is_square_attacked(pos, d8, them, occ_nok)
                && !is_square_attacked(pos, c8, them, occ_nok))
            ml_add(ml, k_sq, c8, CASTLE_QUEEN);
    }
}

// * ALL
void gen_all_moves(pos_t *pos, ml_t *ml, int us)
{
    gen_legal_pawn(pos, ml, us);
    gen_legal_knight(pos, ml, us);
    gen_legal_bishop(pos, ml, us);
    gen_legal_rook(pos, ml, us);
    gen_legal_queen(pos, ml, us);
    gen_legal_king(pos, ml, us);
}


/* ----------------------------------------------------------------------------
 * # KING IN CHECK GENERATION
 *
 * - Move generation if king in check once
 * - legal generation 
 *      + filtering blocker ray 
 *      - castling moves
 * ------------------------------------------------------------------------- */

void gen_blockers_pawn(pos_t *pos, ml_t *ml, bb_t checker_ray, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, PAWN);
    while (bb) {
        int sq = poplsb(&bb);
        // Pinned? king sq -> pinner sq ray : else full
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ? 
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        // single push
        bb_t push = ((us == WHITE) ? (sq_bb(sq) << 8) : (sq_bb(sq) >> 8)) & ~pos->all;
        bb_t db_push = ((us == WHITE) ? 
                ((push << 8) & RANK_4) 
                : ((push >> 8) & RANK_5)) 
            & ~pos->all;
        push &= pin_ray & checker_ray;
        db_push &= pin_ray & checker_ray;
        if (push & ((us == WHITE) ? RANK_8 : RANK_1))
        {
            int to_sq = poplsb(&push);
            ml_add(ml, sq, to_sq, PROM_N);
            ml_add(ml, sq, to_sq, PROM_B);
            ml_add(ml, sq, to_sq, PROM_R);
            ml_add(ml, sq, to_sq, PROM_Q);
        }
        if (db_push) ml_add(ml, sq, poplsb(&db_push), DOUBLE_PAWN);
        if (push) ml_add(ml, sq, poplsb(&push), NO_FLAG);
        // captures
        bb_t captures = ATTACKS_PAWN[us][sq] & pos->occ[them] & pin_ray & checker_ray;
        if (captures & ((us == WHITE) ? RANK_8 : RANK_1)) 
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
        // en passant capture
        if (pos->ep != NO_ENPASSANT && (sq_bb(pos->ep) 
                    & ATTACKS_PAWN[us][sq] & pin_ray))
        {
            int cap_sq = pos->ep + ((us == WHITE) ? -8 : 8);
            if ((sq_bb(pos->ep) & checker_ray) || (sq_bb(cap_sq) & checker_ray))
            {
                if (!(sq_bb(sq) & RANKS[pos->ks[us] >> 3])
                        || !(rook_attacks_fast(pos->ks[us], pos->all & ~(sq_bb(sq) 
                                    | sq_bb(cap_sq))
                                & (BB(pos, them, ROOK) | BB(pos, them, QUEEN)))))
                    ml_add(ml, sq, pos->ep, EN_PASSANT);
            }
        }
    }
}

void gen_blockers_knight(pos_t *pos, ml_t *ml, bb_t checker_ray, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, KNIGHT);
    while (bb) {
        int sq = poplsb(&bb);
        // skip move generation if knight is pinned (can't move if pinned)
        if (pos->pinned[us] & sq_bb(sq)) continue;
        bb_t captures = ATTACKS_KNIGHT[sq] & pos->occ[them] & checker_ray;
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        bb_t quiet_moves = ATTACKS_KNIGHT[sq] & ~pos->all & checker_ray;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_blockers_bishop(pos_t *pos, ml_t *ml, bb_t checker_ray, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, BISHOP);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ? 
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t quiet_moves = bishop_attacks_fast(sq, pos->all) & pin_ray & checker_ray;
        bb_t captures = quiet_moves & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_blockers_rook(pos_t *pos, ml_t *ml, bb_t checker_ray, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, ROOK);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ?
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t quiet_moves = rook_attacks_fast(sq, pos->all) & pin_ray & checker_ray;
        bb_t captures = quiet_moves & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_blockers_queen(pos_t *pos, ml_t *ml, bb_t checker_ray, int us)
{
    int them = us^1;
    bb_t bb = BB(pos, us, QUEEN);
    while (bb) {
        int sq = poplsb(&bb);
        bb_t pin_ray = (pos->pinned[us] & sq_bb(sq)) ?
            BLOCKERS[pos->ks[us]][pinner_sq(pos, sq, us)]
            : FULL_BB;
        bb_t quiet_moves = (bishop_attacks_fast(sq, pos->all) 
                | rook_attacks_fast(sq, pos->all)) & pin_ray & checker_ray;
        bb_t captures = quiet_moves & pos->occ[them];
        while (captures) {
            ml_add(ml, sq, poplsb(&captures), CAPTURE);
        }
        quiet_moves &= ~pos->all;
        while (quiet_moves) {
            ml_add(ml, sq, poplsb(&quiet_moves), NO_FLAG);
        }
    }
}

void gen_king_incheck(pos_t *pos, ml_t *ml, int us)
{
    int them = us^1;
    int k_sq = pos->ks[us];
    int to_sq;
    bb_t occ_nok = pos->all & ~sq_bb(k_sq);
    bb_t captures = ATTACKS_KING[k_sq] & pos->occ[them];
    while (captures) {
        to_sq = poplsb(&captures);
        if (!is_square_attacked(pos, to_sq, them, occ_nok))
            ml_add(ml, k_sq, to_sq, CAPTURE);
    }
    bb_t quiet_moves = ATTACKS_KING[k_sq] & ~pos->all;
    while (quiet_moves) {
        to_sq = poplsb(&quiet_moves);
        if (!is_square_attacked(pos, to_sq, them, occ_nok))
            ml_add(ml, k_sq, to_sq, NO_FLAG);
    }
}

// * ALL
void gen_all_blockers(pos_t *pos, ml_t *ml, int us)
{
    bb_t checker_ray = get_checker_mask(pos, us);
    gen_blockers_pawn(pos, ml, checker_ray, us);
    gen_blockers_knight(pos, ml, checker_ray, us);
    gen_blockers_bishop(pos, ml, checker_ray, us);
    gen_blockers_rook(pos, ml, checker_ray, us);
    gen_blockers_queen(pos, ml, checker_ray, us);
    gen_king_incheck(pos, ml, us);
}


/* ----------------------------------------------------------------------------
 * # MAKE & UNMAKE 
 * ------------------------------------------------------------------------- */

void make_move(pos_t *pos, int from, int to, int flag, color_t c)
{
    // save board state
    pos->ply_stack[++pos->ply] = (saved_t){ 
        pos->pinned[c],
        pos->checkers[c],
        pos->pl[to], 
        pos->castling, 
        pos->ep, 
        pos->hm 
    };
    if (piece_type(pos->pl[from]) == KING) pos->ks[c] = to;
    int sq_r, sq_ep;
    switch(flag) {
        case DOUBLE_PAWN: 
            pos->ep = (c == WHITE) ? (to - 8) : (to + 8); 
            break;
        case EN_PASSANT: 
            sq_ep = (c == WHITE) ? pos->ep - 8 : pos->ep + 8;
            BB(pos, c^1, PAWN) &= ~sq_bb(sq_ep);
            pos->ply_stack[pos->ply].captured = pos->pl[sq_ep];
            pos->pl[sq_ep] = NO_PIECE;
            // pos->occ[c]   |= sq_bb(sq_ep); 
            pos->occ[c^1] &= ~sq_bb(sq_ep); 
            pos->ep = NO_ENPASSANT;
            break;
        case CASTLE_KING: 
            sq_r = (c == WHITE) ? h1 : h8;
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
    int sq_ep, sq_r;
    switch(flag) {
        case EN_PASSANT: 
            sq_ep = (c == WHITE) ? s.ep - 8 : s.ep + 8;
            BB(pos, c^1, PAWN) |= sq_bb(sq_ep);
            pos->pl[sq_ep] = make_piece(c^1, PAWN);
            pos->occ[c^1] |= sq_bb(sq_ep); 
            break;
        case CASTLE_KING: 
            // Would it be better to declare sq_r outside?
            sq_r = (c == WHITE) ? h1 : h8;
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
    pos->pinned[c]   = s.pinned;
    pos->checkers[c] = s.checkers;
    pos->castling  = s.castling;
    pos->ep        = s.ep;
    pos->hm        = s.hm;
    if (pos->side == WHITE) pos->fm--;
    flip_side(pos);
}

// for testing, avoid side & flag
void quick_make(pos_t *pos, int from, int to)
{
    int flag = NO_FLAG;
    if (pos->pl[to] != NO_PIECE) flag = CAPTURE;
    if (to == pos->ep)           flag = EN_PASSANT;
    if (piece_type(pos->pl[from]) == PAWN && abs(from - to) == 16)
        flag = DOUBLE_PAWN;
    //TODO: en passant
    //TODO: promo
    make_move(pos, from, to, flag, pos->side);
}

/* ----------------------------------------------------------------------------
 * # SAFETY & PINS FUNCTIONS
 * ------------------------------------------------------------------------- */

// Check if square attacked by given color
int is_square_attacked(pos_t *pos, int sq, int them, bb_t occ_nok)
{
    assert((unsigned char)sq < SQ_COUNT);
    return (ATTACKS_KNIGHT[sq]               & BB(pos, them, KNIGHT))
        || (bishop_attacks_fast(sq, occ_nok) & (BB(pos, them, BISHOP) | BB(pos, them, QUEEN)))
        || (rook_attacks_fast(sq, occ_nok)   & (BB(pos, them, ROOK)   | BB(pos, them, QUEEN)))
        || (ATTACKS_PAWN[them^1][sq]             & BB(pos, them, PAWN))
        || (ATTACKS_KING[sq]                 & BB(pos, them, KING));
}

// Compute attacked squares into a bitboard
// TODO: change to compute_attacked + change pos
bb_t attacked_squares(pos_t *pos, int us)
{
    int them = us^1;
    bb_t attacked = 0ull;
    bb_t pawns   = BB(pos, them, PAWN);
    bb_t knights = BB(pos, them, KNIGHT);
    bb_t bishops = BB(pos, them, BISHOP);
    bb_t rooks   = BB(pos, them, ROOK);
    bb_t queens  = BB(pos, them, QUEEN);
    bb_t king    = BB(pos, them, KING);
    bb_t occ_nok = pos->all & ~sq_bb(pos->ks[us]);
    while (pawns)   attacked |= ATTACKS_PAWN[them][poplsb(&pawns)];
    while (knights) attacked |= ATTACKS_KNIGHT[poplsb(&knights)];
    while (bishops) attacked |= bishop_attacks_fast(poplsb(&bishops), occ_nok);
    while (rooks)   attacked |= rook_attacks_fast(poplsb(&rooks), occ_nok);
    while (queens)  attacked |= queen_attacks_fast(poplsb(&queens), occ_nok);
    attacked |= ATTACKS_KING[poplsb(&king)];
    return attacked;
}


// compute a pinners(them) & pinned(us) bb
// TODO: change pos in place
bb_t compute_pin(pos_t *pos, int us)
{
    int them = us^1;
    int k_sq = pos->ks[us];
    bb_t pinned = 0ull;
    // TODO: improve reset pinners
    pos->pinners[us] = 0ull;
    // ROOK PINNERS
    bb_t rook_rf_bb = MASK_RF[k_sq];
    bb_t potential_pin = rook_attacks_fast(k_sq, pos->all) & pos->occ[us];
    while (potential_pin)
    {
        int sq = poplsb(&potential_pin);
        bb_t pinners_bb = rook_attacks_fast(sq, pos->all) 
            & rook_rf_bb & pos->occ[them]
            & (BB(pos, them, ROOK) | BB(pos, them, QUEEN));
        if (pinners_bb)
        {
            //TODO: Weird assign and return
            // just assign no return
            pos->pinners[us] |= pinners_bb;
            pinned |= sq_bb(sq);
        }
    }
    // BISHOP PINNERS
    bb_t diag_bb = MASK_DIAG[k_sq];
    potential_pin = bishop_attacks_fast(k_sq, pos->all) & pos->occ[us];
    while (potential_pin)
    {
        int sq = poplsb(&potential_pin);
        bb_t pinners_bb = bishop_attacks_fast(sq, pos->all) 
            & diag_bb & pos->occ[them]
            & (BB(pos, them, BISHOP) | BB(pos, them, QUEEN));
        if (pinners_bb)
        {
            pos->pinners[us] |= pinners_bb;
            pinned |= sq_bb(sq);
        }
    }
    return pinned;
}

// compute checkers(them) bb
// TODO: change pos in place
bb_t compute_checkers(pos_t *pos, int us)
{
    int them = us^1;
    int k_sq = pos->ks[us];
    return (
          (ATTACKS_KNIGHT[k_sq]   & BB(pos, them, KNIGHT))
        | (ATTACKS_PAWN[us][k_sq] & BB(pos, them, PAWN))
        | (rook_attacks_fast(k_sq, pos->all)   & (BB(pos, them, ROOK)   | BB(pos, them, QUEEN)))
        | (bishop_attacks_fast(k_sq, pos->all) & (BB(pos, them, BISHOP) | BB(pos, them, QUEEN)))
        );
}

// determine if king(us) is safe in position.
// USELESS? was used before in make/safe?/unmake
int is_king_safe(pos_t *pos, int us)
{
    int them = us^1;
    int sq   = pos->ks[us];
    assert((unsigned char)sq < 64);
    return !((ATTACKS_PAWN[us][sq]   & BB(pos, them, PAWN))
        | (ATTACKS_KNIGHT[sq]        & BB(pos, them, KNIGHT))
        | (ATTACKS_KING[sq]          & BB(pos, them, KING))
        | (rook_attacks_fast(sq, pos->all)   & (BB(pos, them, ROOK)   | BB(pos, them, QUEEN)))
        | (bishop_attacks_fast(sq, pos->all) & (BB(pos, them, BISHOP) | BB(pos, them, QUEEN))));
}

// generate legal moves in current position for given color(us)
// compute attacked/pinned/pinners/checkers before
void gen_legal(pos_t *pos, ml_t *ml, int us)
{
    // pos->attacked[us] = attacked_squares(pos, us);
    pos->checkers[us] = compute_checkers(pos, us);
    pos->pinned[us]   = compute_pin(pos, us);
    int checkers_count = popcount(pos->checkers[us]);
    assert(checkers_count < 3);
    switch (checkers_count) {
        case 0: gen_all_moves(pos, ml, us); break;
        case 1: gen_all_blockers(pos, ml, us); break;
        case 2: gen_king_incheck(pos, ml, us); break;
        // TODO: add handling for impossible case? or just gen_king_incheck?
        default: break;
    }
}

/* ----------------------------------------------------------------------------
 * # PERFT
 * ------------------------------------------------------------------------- */

ml_t ml_list[MAX_DEPTH];

uint64_t perft(pos_t *pos, int depth, int ply)
{
    if (depth == 0) return 1ULL;

    ml_t *ml = &ml_list[ply];
    ml->count = 0;

    uint64_t nodes = 0;

    gen_legal(pos, ml, pos->side);
    if (depth == 1) return (uint64_t)ml->count;

    for (size_t i = 0; i < ml->count; i++) {
        move_t m = ml->moves[i];
        int from = mfrom(m);
        int to   = mto(m);
        int flag = mflag(m);
        make_move(pos, from, to, flag, pos->side);
        nodes += perft(pos, depth - 1, ply + 1);
        unmake_move(pos, from, to, flag);
    }
    return nodes;
}

void perft_root(pos_t *pos, int max_depth)
{
    for (int d = 0; d <= max_depth; d++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);

        uint64_t nodes = perft(pos, d, 0);

        clock_gettime(CLOCK_MONOTONIC, &t1);
        double ms = (t1.tv_sec - t0.tv_sec) * 1000.0
                  + (t1.tv_nsec - t0.tv_nsec) / 1e6;

        printf("depth: %d, perft: %lu, time: %.1f ms\n", d, nodes, ms);
    }
}

void divide(pos_t *pos, int depth)
{
    ml_t ml = {0};
    gen_legal(pos, &ml, pos->side);
    uint64_t total = 0;
    for (size_t i = 0; i < ml.count; i++) {
        move_t m = ml.moves[i];
        int from = mfrom(m), to = mto(m), flag = mflag(m);
        make_move(pos, from, to, flag, pos->side);
        uint64_t nodes = perft(pos, depth - 1, 0);
        unmake_move(pos, from, to, flag);
        printf("%s", sq_to_str(from));
        printf("%s: %lu\n", sq_to_str(to), nodes);
        total += nodes;
    }
    printf("total: %lu\n", total);
}

/* ----------------------------------------------------------------------------
 * # INITIALISATION & RANDOM MOVE
 * ------------------------------------------------------------------------- */

// initialize all attacks & masks tables
void init_engine(void)
{
    srand(time(NULL));
    _init_attacks_all();
    _init_castling_table();
    _init_mask_rf();
    _init_mask_diag();
    _init_mask_blockers_pin();
}

// make a random legal move for current side in position
void make_random(pos_t *pos)
{
    ml_t ml = {0};
    gen_legal(pos, &ml, pos->side);
    size_t i = rand() % ml.count;
    move_t m = ml.moves[i];
    make_move(pos, mfrom(m), mto(m), mflag(m), pos->side);
}
