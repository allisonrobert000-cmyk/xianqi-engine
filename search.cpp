#include "search.h"
#include "MoveGenerator.h"

std::pair<Move,int> negamax(const Position& position, int depth, int alpha, int beta) {
    std::vector<Move> legalMoves = validateMoves(position, generateMoves(position));

    // No legal moves: checkmate if in check, stalemate otherwise.
    if (legalMoves.empty()) {
        if (isInCheck(position, position.sideToMove)) {
            return { Move{}, -100000 - depth };
        }
        return { Move{}, 0 };
    }

    if (depth == 0) {
        return { Move{}, evaluateBoard(position) * (position.sideToMove == RED ? 1 : -1) };
    }

    int bestScore = -1000000;
    Move bestMove = legalMoves[0];

    for (int i = 0; i < legalMoves.size(); i++) {
        Position resultingPosition = makeMove(position, legalMoves[i]);
        std::pair<Move, int> childResult = negamax(resultingPosition, depth - 1, -beta, -alpha);
        int childScore = -childResult.second;

        if (childScore > bestScore) {
            bestScore = childScore;
            bestMove = legalMoves[i];
        }

        alpha = std::max(alpha, bestScore);
        if (alpha >= beta) {
            break;
        }
    }

    return { bestMove, bestScore };
}

Move findBestMove(const Position& position, int depth) {
    return negamax(position, depth, -10000000, 10000000).first;
}

