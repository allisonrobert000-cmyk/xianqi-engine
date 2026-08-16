#pragma once
#include "MoveValidator.h"
#include "Evaluator.h"

std::pair<Move,int> negamax(const Position& position, int depth, int alpha, int beta);

Move findBestMove(const Position& position, int depth);
