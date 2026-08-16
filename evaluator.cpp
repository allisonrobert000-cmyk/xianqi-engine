#include "Evaluator.h"

int materialScore(const Position& position) {
    int score = 0;

    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            Piece piece = position.board[row][col];
            switch (piece) {
                case RED_CHARIOT: score += 18;
                break;
                case BLACK_CHARIOT: score -= 18;
                break;
                case RED_CANNON: score += 9;
                break;
                case BLACK_CANNON: score -= 9;
                break;
                case RED_HORSE: score += 8;
                break;
                case BLACK_HORSE: score -= 8;
                break;
                case RED_ELEPHANT: score += 4;
                break;
                case BLACK_ELEPHANT: score -= 4;
                break;
                case RED_ADVISOR: score += 4;
                break;
                case BLACK_ADVISOR: score -= 4;
                break;
                case RED_SOLDIER: 
                if (row >= 0 && row < 5) {
                    score += 4;
                } else {
                    score += 2;
                }
                break;
                case BLACK_SOLDIER: 
                if (row > 4 && row < 10) {
                    score -= 4;
                } else {
                    score -= 2;
                }
                break;
                default: break;
            }
        }
    }
    return score;
}

int evaluateBoard(const Position& position) {
    int totalScore = 0;
    totalScore += materialScore(position);

    return totalScore;
}