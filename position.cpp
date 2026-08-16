#include "position.h"
#include "pieces.h"
#include <cassert>
#include <iostream>

static const Piece STARTING_POSITION[10][9] = 
{
    {BLACK_CHARIOT, BLACK_HORSE, BLACK_ELEPHANT, BLACK_ADVISOR, BLACK_GENERAL, BLACK_ADVISOR, BLACK_ELEPHANT, BLACK_HORSE, BLACK_CHARIOT},
    {EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},
    {EMPTY,BLACK_CANNON,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,BLACK_CANNON,EMPTY},
    {BLACK_SOLDIER,EMPTY,BLACK_SOLDIER,EMPTY,BLACK_SOLDIER,EMPTY,BLACK_SOLDIER,EMPTY,BLACK_SOLDIER},
    {EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},
    {EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},
    {RED_SOLDIER,EMPTY,RED_SOLDIER,EMPTY,RED_SOLDIER,EMPTY,RED_SOLDIER,EMPTY,RED_SOLDIER},
    {EMPTY,RED_CANNON,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,RED_CANNON,EMPTY},
    {EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},
    {RED_CHARIOT,RED_HORSE,RED_ELEPHANT,RED_ADVISOR,RED_GENERAL,RED_ADVISOR,RED_ELEPHANT,RED_HORSE,RED_CHARIOT}
};

Position::Position() {
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            board[row][col] = STARTING_POSITION[row][col];
        }
    }

    sideToMove = RED;
}

bool isInPalace(int row, int col, Side side) {
    switch (side) {
        case RED:
        return (row >= 7 && row <= 9 && col >= 3 && col <= 5);
        case BLACK:
        return (row >= 0 && row <= 2 && col >= 3 && col <= 5);
        default:
            std::cerr << "Invalid side value passed to inInPalace" << std::endl;
            assert(false);
    }
}

bool isOnBoard(int row, int col) {
    return ((row <= 9 && row >= 0) && (col <= 8 && col >= 0));
}

Position makeMove(const Position& position, Move move) {
    Position copyPosition = position;
    Piece movePiece = copyPosition.board[move.fromRow][move.fromCol];
    copyPosition.board[move.toRow][move.toCol] = movePiece;
    copyPosition.board[move.fromRow][move.fromCol] = EMPTY;

    copyPosition.sideToMove = (position.sideToMove == RED) ? BLACK : RED;

    return copyPosition;
}