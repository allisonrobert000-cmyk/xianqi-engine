#include "search.h"
#include "MoveGenerator.h"

std::pair<Move,int> negamax(const Position& position, int depth, int alpha, int beta) {
    GameStatus status = isGameOver(position);

    if (status == CHECKMATE) {
        return { Move{}, -100000 - depth};
    }

    if (status == STALEMATE) {
        return { Move{}, 0};
    }

    if (depth == 0) {
        return { Move{}, evaluateBoard(position) * (position.sideToMove == RED ? 1 : -1)};
    }

    std::vector<Move> legalMoves = validateMoves(position, generateMoves(position));

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

    return { bestMove, bestScore};

}

Move findBestMove(const Position& position, int depth) {
    return negamax(position, depth, -10000000, 10000000).first;
}

