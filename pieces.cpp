#include "pieces.h"

bool isRed(Piece piece) {
    switch (piece) {
        case RED_GENERAL:
        case RED_ADVISOR:
        case RED_CANNON:
        case RED_CHARIOT:
        case RED_ELEPHANT:
        case RED_HORSE:
        case RED_SOLDIER:
            return true;
        case BLACK_ADVISOR:
        case BLACK_CANNON:
        case BLACK_CHARIOT:
        case BLACK_ELEPHANT:
        case BLACK_GENERAL:
        case BLACK_HORSE:
        case BLACK_SOLDIER:
            return false;
        case EMPTY: return false;
        default: return false;
    }
}

bool isBlack(Piece piece) {
    return !isRed(piece) && piece != EMPTY;
}

bool belongsToSide(Piece piece, Side side) {
    return ((side == RED && isRed(piece)) || (side == BLACK && isBlack(piece)));
}