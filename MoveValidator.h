#pragma once
#include "MoveGenerator.h"


enum GameStatus {
    CHECKMATE,
    STALEMATE,
    CONTINUE
};

std::pair<int,int> findGeneral(const Position& position, Side side);

bool isThreatenedByChariot(const Position& position, int row, int col, Side side);

bool isThreatenedByCannon(const Position& position, int row, int col, Side side);

bool isThreatenedBySoldier(const Position& position, int row, int col, Side side);

bool isThreatenedByHorse(const Position& position, int row, int col, Side side);

bool isThreatenedByGeneral(const Position& position, int row, int col, Side side);

bool isInCheck(const Position& position, Side side);

std::vector<Move> validateMoves(const Position& position, const std::vector<Move>& moves);

GameStatus isGameOver(const Position& position);

