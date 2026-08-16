#include "MoveGenerator.h"
#include <cassert>
#include <iostream>

using namespace std;

static void generateGeneralMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
    std::pair<int,int> directions[4] = {
        {-1, 0}, // up
        {1,0}, //down
        {0,-1}, //left
        {0, 1} //right
    };

    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i].first;
        int newCol = col + directions[i].second;
        if (isOnBoard(newRow,newCol)) {
            Piece occPiece = position.board[newRow][newCol];
            if ((isInPalace(newRow, newCol, position.sideToMove)) && (!belongsToSide(occPiece, position.sideToMove))) {
                moves.push_back({row, col, newRow, newCol});
            }
        }
    }
}

static void generateAdvisorMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
    std::pair<int,int> directions[4] = {
        {-1, -1}, // up left
        {-1,1}, //up right
        {1,1}, //down right
        {1,-1} //down left
    };

    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i].first;
        int newCol = col + directions[i].second;
        if (isOnBoard(newRow, newCol)) {
            Piece occPiece = position.board[newRow][newCol];
            if ((isInPalace(newRow, newCol, position.sideToMove)) && (!belongsToSide(occPiece, position.sideToMove))) {
                moves.push_back({row, col, newRow, newCol});
            }
        }
    }
}

static void generateSoldierMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
    std::vector<std::pair<int,int>> directions;
    if (position.sideToMove == BLACK) {
        if (row <= 4 && row >= 0) {
            directions.push_back({1,0});
        } else if (row > 4 && row <= 9) {
            directions.push_back({1,0}); //down
            directions.push_back({0,-1}); //left
            directions.push_back({0,1}); //right
        } else {
            std::cerr << "Invalid row value passed to generateSoldierMoves" << std::endl;
        }
    } else if (position.sideToMove == RED) {
        if (row > 4 && row <= 9) {
            directions.push_back({-1,0});
        } else if (row <= 4 && row >= 0) {
            directions.push_back({-1,0}); //down
            directions.push_back({0,-1}); //left
            directions.push_back({0,1}); //right
        } else {
            std::cerr << "Invalid row value passed to generateSoldierMoves" << std::endl;
        }
    } else {
        std::cerr << "Invalid Side value passed to generateSoldierMoves";
    }

    for (int i = 0; i < directions.size(); i++) {
        int newRow = row + directions[i].first;
        int newCol = col + directions[i].second;
        if (isOnBoard(newRow, newCol)) {
            Piece occPiece = position.board[newRow][newCol];
            if (!belongsToSide(occPiece, position.sideToMove)) {
                moves.push_back({row, col, newRow, newCol});
            }
        }
    }
}

static void generateElephantMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
        std::pair<int,int> directions[4] = {
        {-2, -2}, // up left
        {-2,2}, //up right
        {2,2}, //down right
        {2,-2} //down left
    };

    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i].first;
        int newCol = col + directions[i].second;
        int eyeRow = row + directions[i].first / 2;
        int eyeCol = col + directions[i].second /2;
        if (isOnBoard(newRow, newCol)) {
            Piece occPiece = position.board[newRow][newCol];
            Piece eyePiece = position.board[eyeRow][eyeCol];
            if (!belongsToSide(occPiece, position.sideToMove)) {
                if (eyePiece == EMPTY) {
                    if ((position.sideToMove == BLACK && newRow <= 4 && newRow >= 0) || (position.sideToMove == RED && newRow > 4 && newRow <= 9)) {
                        moves.push_back({row, col, newRow, newCol});
                    }
                }
                
            }
        }
    }   
}

static void generateHorseMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
    struct HorseMove {
        std::pair<int,int> destination;
        std::pair<int,int> leg;
    };

    HorseMove horseMoves[8] = {
        { {-2,-1}, {-1, 0} },
        { {-2,1}, {-1, 0} },
        { {2,-1}, {1, 0} },
        { {2,1}, {1, 0} },
        { {-1,-2}, {0, -1} },
        { {1,-2}, {0, -1} },
        { {-1,2}, {0, 1} },
        { {1,2}, {0, 1} }
    };

    for (int i = 0; i < 8; i++) {
        int newRow = row + horseMoves[i].destination.first;
        int newCol = col + horseMoves[i].destination.second;
        int legRow = row + horseMoves[i].leg.first;
        int legCol = col + horseMoves[i].leg.second;
        if (isOnBoard(newRow, newCol)) {
            Piece occPiece = position.board[newRow][newCol];
            Piece legPiece = position.board[legRow][legCol];
            if (legPiece == EMPTY) {
                if (!belongsToSide(occPiece, position.sideToMove)) {
                    moves.push_back({row, col, newRow, newCol});
                }
            }
        }
    }
}

static void generateChariotMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
        std::pair<int,int> directions[4] = {
        {-1, 0}, // up
        {1,0}, //down
        {0,-1}, //left
        {0, 1} //right
    };

    for (int i = 0; i < 4; i++) {
        int steps = 1; 
        bool blocked = false;
        while (!blocked) {
            int newRow = row + (directions[i].first * steps);
            int newCol = col + (directions[i].second * steps);
            if (isOnBoard(newRow, newCol)) {
                Piece occPiece = position.board[newRow][newCol];
                if (occPiece == EMPTY) {
                    moves.push_back({row, col, newRow, newCol});
                    steps++;
                } else if (belongsToSide(occPiece, position.sideToMove)) {
                    blocked = true;
                } else {
                    moves.push_back({row, col, newRow, newCol});
                    blocked = true;
                }
            } else {
                blocked = true;
            }
        }
    }
}

static void generateCannonMoves(const Position& position, int row, int col, std::vector<Move>& moves) {
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
                        moves.push_back({row, col, newRow, newCol});
                        steps++;
                    } else {
                        foundScreen = true;
                        steps++;
                    }
                } else {
                    if (occPiece == EMPTY) {
                        steps++;
                    } else {
                        if (belongsToSide(occPiece, position.sideToMove)) {
                            blocked = true;
                        } else {
                            moves.push_back({row, col, newRow, newCol});
                            blocked = true;
                        }
                    }
                }
            } else {
                blocked = true;
            }
        }
    }
}

std::vector<Move> generateMoves(const Position& position) {
    vector<Move> newMoves;
    
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            Piece piece = position.board[row][col];
            if (belongsToSide(piece, position.sideToMove)) {
                switch (piece) {
                    case RED_GENERAL:
                    case BLACK_GENERAL:
                    generateGeneralMoves(position, row, col, newMoves);
                    break;
                    case RED_ADVISOR:
                    case BLACK_ADVISOR:
                    generateAdvisorMoves(position, row, col, newMoves);
                    break;
                    case RED_SOLDIER:
                    case BLACK_SOLDIER:
                    generateSoldierMoves(position, row, col, newMoves);
                    break;
                    case RED_ELEPHANT:
                    case BLACK_ELEPHANT:
                    generateElephantMoves(position, row, col, newMoves);
                    break;
                    case RED_HORSE:
                    case BLACK_HORSE:
                    generateHorseMoves(position, row, col, newMoves);
                    break;
                    case RED_CHARIOT:
                    case BLACK_CHARIOT:
                    generateChariotMoves(position, row, col, newMoves);
                    break;
                    case RED_CANNON:
                    case BLACK_CANNON:
                    generateCannonMoves(position, row, col, newMoves);
                    break;
                    default: break;
                }
            } 
        }
    }
    return newMoves;
}