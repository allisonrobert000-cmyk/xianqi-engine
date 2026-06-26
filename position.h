//defines the position struct which stores the board array and whose turn it is
#pragma once
#include "pieces.h"


// board[row][col]
// row 0 = Black home rank
// row 9 = Red home rank
struct Position {
    Piece board[10][9];
    Side sideToMove;

    Position();
};


