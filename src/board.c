#include "board.h"

/* ----------------------------------------------------------------------------
 * # TABLES & CONSTANTS
 * ------------------------------------------------------------------------- */

const bb_t RANKS[8] = { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };
const bb_t FILES[8] = { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };

uint8_t CASTLING_TABLE[SQ_COUNT];

void _init_castling_table(void)
{
    memset(CASTLING_TABLE, 0xFF, SQ_COUNT);
    CASTLING_TABLE[a1] ^= W_000;
    CASTLING_TABLE[a8] ^= B_000;
    CASTLING_TABLE[h1] ^= W_00;
    CASTLING_TABLE[h8] ^= B_00;
    CASTLING_TABLE[e1] ^= (W_00 | W_000);
    CASTLING_TABLE[e8] ^= (B_00 | B_000);
}


/* ----------------------------------------------------------------------------
 * # STARTING POSITION
 * ------------------------------------------------------------------------- */

static void _set_starting_pieces(pos_t *pos)
{
    // WHITE PIECES
    set_bb(pos, WHITE, PAWN,   0x000000000000FF00ull);
    set_bb(pos, WHITE, KNIGHT, 0x0000000000000042ull);
    set_bb(pos, WHITE, BISHOP, 0x0000000000000024ull);
    set_bb(pos, WHITE, ROOK,   0x0000000000000081ull);
    set_bb(pos, WHITE, QUEEN,  0x0000000000000008ull);
    set_bb(pos, WHITE, KING,   0x0000000000000010ull);
    // BLACK PIECES
    set_bb(pos, BLACK, PAWN,   0x00FF000000000000ull);
    set_bb(pos, BLACK, KNIGHT, 0x4200000000000000ull);
    set_bb(pos, BLACK, BISHOP, 0x2400000000000000ull);
    set_bb(pos, BLACK, ROOK,   0x8100000000000000ull);
    set_bb(pos, BLACK, QUEEN,  0x0800000000000000ull);
    set_bb(pos, BLACK, KING,   0x1000000000000000ull);
}

static void _set_occupancy(pos_t *pos)
{
    pos->occ[WHITE] = 0ull;
    pos->occ[BLACK] = 0ull;
    ITER_BB(c, p) { pos->occ[c] |= BB(pos, c, p); }
    pos->all = pos->occ[WHITE] | pos->occ[BLACK];
}

static void _set_piecelist(pos_t *pos)
{
    ITER_SQ(sq) {
        if (!(sq_bb(sq) & pos->all)) {
            pos->pl[sq] = NO_PIECE;
            goto next_sq;
        }
        ITER_COLOR(c) {
            if (!(sq_bb(sq) & pos->occ[c])) continue;
            ITER_PIECE(p) {
                if (sq_bb(sq) & BB(pos, c, p)){
                    pos->pl[sq] = make_piece(c, p);
                    goto next_sq;
                }
            }
        }
        next_sq:;
    }
}

// Return an initialized starting position
pos_t *starting_pos(void)
{
    pos_t *pos = malloc(sizeof(pos_t));
    if (!pos) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    _set_starting_pieces(pos);
    _set_occupancy(pos);
    _set_piecelist(pos);
    pos->side = WHITE;
    // KING SQUARES
    pos->ks[WHITE] = e1;
    pos->ks[BLACK] = e8;
    // CASTLING RIGHTS / EN PASSANT / MOVE COUNT
    pos->castling = ALL_CASTLING;
    pos->ep = NO_ENPASSANT;
    pos->hm = 0;
    pos->fm = 0;
    pos->ply = 0;
    return pos;
}

/* ----------------------------------------------------------------------------
 * # PRINTING
 * ------------------------------------------------------------------------- */

void print_bb(bb_t bb)
{
    for (int r = 7; r >= 0; r--) {
        for (int f = 0; f < 8; f++) {
            printf("%i ", (sq_bb(r * 8 + f) & bb) ? 1 : 0);
        }
        putchar('\n');
    }
}

static char _get_piece_char(uint8_t p)
{
    char c = (piece_color(p) == BLACK) ? 'a' - 'A' : 0;
    switch (piece_type(p)){
        case NO_PIECE: return     '.';
        case PAWN:     return c + 'P';
        case KNIGHT:   return c + 'N';
        case BISHOP:   return c + 'B';
        case ROOK:     return c + 'R';
        case QUEEN:    return c + 'Q';
        case KING:     return c + 'K';
        default:       return '.';
    }
}

void print_pos(pos_t *pos)
{
    printf("   | a b c d e f g h |\n");
    printf(" ---------------------\n");
    for (int r = 7; r >= 0; r--) {
        printf(" %i | ", r + 1);
        for (int f = 0; f < 8; f++) {
            printf("%c ", _get_piece_char(pos->pl[r * 8 + f]));
        }
        printf("|\n");
    }
    printf(" ---------------------\n");
}

void print_pos_info(pos_t *pos)
{
    printf("KS BLACK: '%i', WHITE: '%i'\n", pos->ks[BLACK], pos->ks[WHITE]);
    printf("Side to play: '%s'\n", (pos->side) ? "BLACK" : "WHITE");
    printf("W_00: '%i' | W_000: '%i' | B_00: '%i' | B_000: '%i'\n", 
            (pos->castling & W_00)  ? 1 : 0,
            (pos->castling & W_000) ? 1 : 0,
            (pos->castling & B_00)  ? 1 : 0,
            (pos->castling & B_000) ? 1 : 0);
    printf("EN PASSANT: '%s'\n", 
            (pos->ep == NO_ENPASSANT) ? "NO_ENPASSANT" : sq_to_str(pos->ep));
    printf("HM: '%i'\n", pos->hm);
    printf("FM: '%i'\n", pos->fm);
}

/* ----------------------------------------------------------------------------
 * # FEN PARSING
 * ------------------------------------------------------------------------- */

static const int CHAR_TO_PIECE[128] = {
    ['p'] = -PAWN, ['n'] = -KNIGHT, ['b'] = -BISHOP, 
    ['r'] = -ROOK, ['q'] = -QUEEN,  ['k'] = -KING, 
    ['P'] =  PAWN, ['N'] =  KNIGHT, ['B'] =  BISHOP, 
    ['R'] =  ROOK, ['Q'] =  QUEEN,  ['K'] =  KING, 
};

pos_t *parse_fen(char *fen)
{
    pos_t *pos = calloc(1, sizeof(pos_t));
    // bitboard & piece list setup
    int r = 7;
    int f = 0;
    for (; *fen != ' '; fen++)
    {
        // Empty squares
        if (*fen >= '1' && *fen <= '8') {
            f += (*fen - '0');
            continue;
        }
        // End of rank
        else if (*fen == '/') {
            r--;
            f = 0;
            continue;
        }
        // Else add piece to `pos`
        assert(VALID_RF(r, f));
        int8_t piece = CHAR_TO_PIECE[(uint8_t)*fen];
        if (!piece) { free(pos); return NULL; }
        int c = (piece < 0) ?  BLACK : WHITE;
        piece = (piece < 0) ? -piece : piece;
        BB(pos, c, piece) |= sq_bb(r * 8 + f);
        pos->pl[r * 8 + f] = make_piece(c, piece);
        if (piece == KING) pos->ks[c] = r * 8 + f;
        f++;
    }
    fen++;
    // side to play
    switch (*fen) {
        case 'w': pos->side = WHITE; break;
        case 'b': pos->side = BLACK; break;
        default:  free(pos); return NULL;
    }
    fen += 2;
    // Castling rights
    uint8_t guard = 0x00;
    for (; *fen != ' '; fen++)
    {
        switch (*fen) {
            case 'k': 
                if (guard & B_00) { free(pos); return NULL; } 
                pos->castling |= B_00;  
                guard |= B_00;
                break;
            case 'q':
                if (guard & B_000) { free(pos); return NULL; } 
                pos->castling |= B_000;  
                guard |= B_000;
                break;
            case 'K':
                if (guard & W_00) { free(pos); return NULL; } 
                pos->castling |= W_00;  
                guard |= W_00;
                break;
            case 'Q':
                if (guard & W_000) { free(pos); return NULL; } 
                pos->castling |= W_000;  
                guard |= W_000;
                break;
            case '-': break;
            default: free(pos); return NULL;
        }
    }
    fen++;
    // En passant
    if (*fen == '-'){
        pos->ep = NO_ENPASSANT;
    } else {
        int f = *fen - 'a';
        int r = *(fen + 1) - '1';
        if (!VALID_RF(r, f)){ free(pos); return NULL; }
        pos->ep = r * 8 + f;
    }
    fen += 2;
    // Half move & full move
    char *end;
    pos->hm = strtol(fen, &end, 10);
    if (end == fen) { free(pos); return NULL; }
    fen = end + 1;
    pos->fm = strtol(fen, &end, 10);
    if (end == fen) { free(pos); return NULL; }
    // Board occupancy
    _set_occupancy(pos);
    pos->ply = 0;
    return pos;
}

/* ----------------------------------------------------------------------------
 * # BIT UTILS
 * ------------------------------------------------------------------------- */

// returns the number of bits set in a bitboard
int popcount(bb_t bb)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bb);
#else
    int count;
    for (count = 0; bb; count++, bb &= bb - 1);
    return count;
#endif
}

// removes lsb from bitboard and returns its index
int poplsb(bb_t *bb)
{
    assert(*bb != 0);
    bb_t b = *bb;
    int index = __builtin_ctzll(b);
    *bb = b & (b - 1);
    return index;
}
