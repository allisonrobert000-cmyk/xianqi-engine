#include "MoveValidator.h"
#include <cmath>


std::pair<int,int> findGeneral(const Position& position, Side side) {
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            Piece piece = position.board[row][col];
            if (side == RED && piece == RED_GENERAL) {
                return {row, col};
            } else if (side == BLACK && piece == BLACK_GENERAL) {
                return {row, col};
            }
        }
    }
    return {-1, -1};
}

bool isThreatenedByChariot(const Position& position, int row, int col, Side side) {
    for (int i = 0; i < 9; i++) {
        Piece squarePiece = position.board[row][i];
        if ((squarePiece == RED_CHARIOT || squarePiece == BLACK_CHARIOT) && (!belongsToSide(squarePiece, side))) {
            bool isEmpty = true;
            if (i < col) {
                for (int j = i + 1; j < col; j++) {
                    Piece betweenPiece = position.board[row][j];
                    if (betweenPiece != EMPTY) {
                        isEmpty = false;
                        break;
                    }
                }
                if (isEmpty) {
                    return true;
                }
            }
            if (i > col) {
                for (int j = col + 1; j < i; j++) {
                    Piece betweenPiece = position.board[row][j];
                    if (betweenPiece != EMPTY) {
                        isEmpty = false;
                        break;
                    }
                }
                if (isEmpty) {
                    return true;
                }
            }
        }
    }

    for (int i = 0; i < 10; i++) {
        Piece squarePiece = position.board[i][col];
        if ((squarePiece == RED_CHARIOT || squarePiece == BLACK_CHARIOT) && (!belongsToSide(squarePiece, side))) {
            bool isEmpty = true;
            if (i < row) {
                for (int j = i + 1; j < row; j++) {
                    Piece betweenPiece = position.board[j][col];
                    if (betweenPiece != EMPTY) {
                        isEmpty = false;
                        break;
                    }
                }
                if (isEmpty) {
                    return true;
                }
            }
            if (i > row) {
                for (int j = row + 1; j < i; j++) {
                    Piece betweenPiece = position.board[j][col];
                    if (betweenPiece != EMPTY) {
                        isEmpty = false;
                        break;
                    }
                }
                if (isEmpty) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isThreatenedByGeneral(const Position& position, int row, int col, Side side) {
    if (side == BLACK) {
        for (int i = row + 1; i < 10; i++) {
            Piece occPiece = position.board[i][col];
            if (occPiece == RED_GENERAL) {
                return true; 
            } else if (occPiece != EMPTY) {
                return false;
            }
        }
        return false;
    }
    if (side == RED) {
        for (int i = row - 1; i >= 0; i--) {
            Piece occPiece = position.board[i][col];
            if (occPiece == BLACK_GENERAL) {
                return true;
            } else if (occPiece != EMPTY) {
                return false;
            }
        }
        return false;
    }
    return false;
}

bool isThreatenedBySoldier(const Position& position, int row, int col, Side side) {
    if (side == BLACK) {
        Piece inFrontPiece = position.board[row + 1][col];
        Piece leftPiece = position.board[row][col - 1];
        Piece rightPiece = position.board[row][col + 1];
        if (inFrontPiece == RED_SOLDIER || leftPiece == RED_SOLDIER || rightPiece == RED_SOLDIER) {
            return true;
        } else {
            return false;
        }
    }
    if (side == RED) {
        Piece inFrontPiece = position.board[row - 1][col];
        Piece leftPiece = position.board[row][col -1 ];
        Piece rightPiece = position.board[row][col + 1];
        if (inFrontPiece == BLACK_SOLDIER || leftPiece == BLACK_SOLDIER || rightPiece == BLACK_SOLDIER) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool isThreatenedByHorse(const Position& position, int row, int col, Side side) {
    struct ThreatCheck {
        std::pair<int,int> candidateOffset;
        std::pair<int,int> legOffset;
    };

    ThreatCheck horseThreatChecks[8] = {
    { {2,1},   {1,1}   },
    { {2,-1},  {1,-1}  },
    { {-2,1},  {-1,1}  },
    { {-2,-1}, {-1,-1} },
    { {1,2},   {1,1}   },
    { {-1,2},  {-1,1}  },
    { {1,-2},  {1,-1}  },
    { {-1,-2}, {-1,-1} },
    };

    for (int i = 0; i < 8; i++) {
        int newRow = row + horseThreatChecks[i].candidateOffset.first;
        int newCol = col + horseThreatChecks[i].candidateOffset.second;
        int legRow = row + horseThreatChecks[i].legOffset.first;
        int legCol = col + horseThreatChecks[i].legOffset.second;
        if (isOnBoard(newRow, newCol)) {
            Piece occPiece = position.board[newRow][newCol];
            Piece legPiece = position.board[legRow][legCol];
            if (legPiece == EMPTY) {
                if (!belongsToSide(occPiece, side) && (occPiece == BLACK_HORSE || occPiece ==RED_HORSE)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isThreatenedByCannon(const Position& position, int row, int col, Side side) {
    std::pair<int,int> directions[4] = {
        {-1, 0}, // up
        {1,0}, //down
        {0,-1}, //left
        {0, 1} //right
    };

    for (int i = 0; i < 4; i++) {
        int steps = 1; 
        bool blocked = false;
        bool foundScreen = false;
        while (!blocked) {
            int newRow = row + (directions[i].first * steps);
            int newCol = col + (directions[i].second * steps);
            if (isOnBoard(newRow, newCol)) {
                Piece occPiece = position.board[newRow][newCol];
                if (!foundScreen) {
                    if (occPiece == EMPTY) {
                        steps++;
                    } else {
                        foundScreen = true;
                        steps++;
                    }
                } else {
                    if (occPiece == EMPTY) {
                        steps++;
                    } else {
                        if ((!belongsToSide(occPiece, side)) && (occPiece == RED_CANNON || occPiece == BLACK_CANNON)) {
                            return true;
                        } else {
                            blocked = true;
                        }
                    }
                }
            } else {
                blocked = true;
            }
        }
    }
    return false;
}

bool isInCheck(const Position& position, Side side) {
    std::pair<int,int> generalPos = findGeneral(position, side);
    int generalRow = generalPos.first;
    int generalCol = generalPos.second;

    if (isThreatenedByCannon(position, generalRow, generalCol, side)) {
        return true;
    }

    if (isThreatenedByChariot(position, generalRow, generalCol, side)) {
        return true;
    }

    if (isThreatenedByGeneral(position, generalRow, generalCol, side)) {
        return true;
    }

    if (isThreatenedByHorse(position, generalRow, generalCol, side)) {
        return true;
    }

    if (isThreatenedBySoldier(position, generalRow, generalCol, side)) {
        return true;
    }

    return false;
}

std::vector<Move> validateMoves(const Position& position, const std::vector<Move>& moves) {
    std::vector<Move> results; 

    for (int i = 0; i < moves.size(); i++) {
        Move currentMove = moves[i];
        Position copyPosition = position;
        Piece movePiece = copyPosition.board[currentMove.fromRow][currentMove.fromCol];
        copyPosition.board[currentMove.toRow][currentMove.toCol] = movePiece;
        copyPosition.board[currentMove.fromRow][currentMove.fromCol] = EMPTY;
        if (!isInCheck(copyPosition, copyPosition.sideToMove)) {
            results.push_back(currentMove);
        }
    }

    return results;
}

GameStatus isGameOver(const Position& position) {
    std::vector<Move> remainingMoves = generateMoves(position);
    remainingMoves = validateMoves(position, remainingMoves);

    if (remainingMoves.empty()) {
        if (isInCheck(position, position.sideToMove)) {
            return CHECKMATE;
        }
        else {
            return STALEMATE;
        }
    } else {
        return CONTINUE;
    }
}