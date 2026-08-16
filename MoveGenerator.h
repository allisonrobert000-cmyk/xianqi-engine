//declares the move generator function
#pragma once
#include "position.h"
#include "move.h"
#include <vector>

// generateMoves takes a Position as input and outputs an array of all possible Moves 
std::vector<Move> generateMoves(const Position& position);

