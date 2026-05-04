#include <criterion/criterion.h>
#include "board.h"


Test(ParseFen, startingPosition)
{
    // # Starting position
    pos_t *pos = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    cr_assert(pos->castling == ALL_CASTLING);
    cr_assert(pos->side == WHITE);
    cr_assert(pos->ep == NO_ENPASSANT);
    cr_assert(pos->hm == 0);
    cr_assert(pos->fm == 1);
    free(pos);
}

Test(ParseFen, noCastling)
{
    // # No castling rights
    pos_t *pos = parse_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w - - 0 1");
    cr_assert(pos->castling == NO_CASTLING);
    cr_assert(pos->side == WHITE);
    cr_assert(pos->ep == NO_ENPASSANT);
    cr_assert(pos->hm == 0);
    cr_assert(pos->fm == 1);
    free(pos);
}

Test(ParseFen, blackKingCastle)
{
    // # Only black kingside castling
    pos_t *pos = parse_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w k - 0 1");
    cr_assert(pos->castling & B_00);
    cr_assert((pos->castling & B_000) == 0);
    cr_assert(pos->side == WHITE);
    cr_assert(pos->ep == NO_ENPASSANT);
    cr_assert(pos->hm == 0);
    cr_assert(pos->fm == 1);
    free(pos);
}

Test(ParseFen, enPassantSQ)
{
    // # En passant available on e6
    pos_t *pos = parse_fen("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    cr_assert(pos->castling == ALL_CASTLING);
    cr_assert(pos->side == WHITE);
    cr_assert(pos->ep == d6);
    cr_assert(pos->hm == 0);
    cr_assert(pos->fm == 2);
    free(pos);
}

Test(ParseFen, endGame)
{
    // # Endgame — kings and pawns only
    pos_t *pos = parse_fen("8/4k3/8/3p4/3P4/8/4K3/8 w - - 32 22");
    cr_assert(pos->castling == NO_CASTLING);
    cr_assert(pos->side == WHITE);
    cr_assert(pos->ep == NO_ENPASSANT);
    cr_assert(pos->hm == 32);
    cr_assert(pos->fm == 22);
    free(pos);
}

Test(ParseFen, invalidRow)
{
    pos_t *pos = parse_fen("9/4k3/8/3p4/3P4/8/4K3/8 w - - 32 22");
    cr_assert(pos == NULL);
    free(pos);
}

Test(ParseFen, invalidPiece)
{
    pos_t *pos = parse_fen("8/4f3/8/3p4/3P4/8/4K3/8 w - - 32 22");
    cr_assert(pos == NULL);
    free(pos);
}

Test(ParseFen, invalidCastle)
{
    pos_t *pos = parse_fen("8/4k3/8/3p4/3P4/8/4K3/8 w kQkK - 32 22");
    cr_assert(pos == NULL);
    free(pos);
}

Test(ParseFen, invalidEnPassant)
{
    pos_t *pos = parse_fen("8/4k3/8/3p4/3P4/8/4K3/8 w kqQK z9 32 22");
    cr_assert(pos == NULL);
    free(pos);
}
