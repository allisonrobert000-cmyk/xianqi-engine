#include <iostream>
#include <vector>
#include "pieces.h"
#include "position.h"
#include "MoveGenerator.h"
#include "MoveValidator.h"
#include "Evaluator.h"
#include "search.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Wipes the board so a test can place only the pieces it cares about.
static void clearBoard(Position& position) {
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            position.board[row][col] = EMPTY;
        }
    }
}

static int passCount = 0;
static int failCount = 0;

static void check(const std::string& testName, bool condition) {
    if (condition) {
        std::cout << "  PASS: " << testName << std::endl;
        passCount++;
    } else {
        std::cout << "  FAIL: " << testName << std::endl;
        failCount++;
    }
}

static void checkEquals(const std::string& testName, long long actual, long long expected) {
    if (actual == expected) {
        std::cout << "  PASS: " << testName << " (" << actual << ")" << std::endl;
        passCount++;
    } else {
        std::cout << "  FAIL: " << testName
                  << " -- expected " << expected
                  << ", got " << actual << std::endl;
        failCount++;
    }
}

// ---------------------------------------------------------------------------
// Perft: counts all legal move sequences to a given depth.
// This is THE standard way to validate a move generator. If your numbers
// match the published ones, your move generation + legality filtering is
// almost certainly correct. If they don't, the depth at which they first
// diverge tells you a lot about where the bug is.
// ---------------------------------------------------------------------------

static long long perft(const Position& position, int depth) {
    std::vector<Move> legalMoves = validateMoves(position, generateMoves(position));

    if (depth == 1) {
        return (long long)legalMoves.size();
    }

    long long nodes = 0;
    for (int i = 0; i < legalMoves.size(); i++) {
        Position next = makeMove(position, legalMoves[i]);
        nodes += perft(next, depth - 1);
    }
    return nodes;
}

// ---------------------------------------------------------------------------
// Test 1: Perft against known Xiangqi values
// Published values for the standard starting position:
//   depth 1 -> 44
//   depth 2 -> 1920
//   depth 3 -> 79666
// (depth 3 may take a few seconds with a copy-based makeMove; that's expected)
// ---------------------------------------------------------------------------

static void testPerft() {
    std::cout << "\n=== Test 1: Perft from starting position ===" << std::endl;

    Position startPosition;  // constructor sets up the standard opening board

    checkEquals("perft(1)", perft(startPosition, 1), 44);
    checkEquals("perft(2)", perft(startPosition, 2), 1920);

    std::cout << "  (running perft(3), this may take a few seconds...)" << std::endl;
    checkEquals("perft(3)", perft(startPosition, 3), 79666);
}

// ---------------------------------------------------------------------------
// Test 2: Check detection, one threat type at a time.
// Each position is minimal and legal: it is the checked side's turn to move,
// which is the only way a real game can present a check.
// ---------------------------------------------------------------------------

static void testCheckDetection() {
    std::cout << "\n=== Test 2: Check detection ===" << std::endl;

    // --- Chariot gives check along a column ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[1][4] = RED_CHARIOT;   // directly below, nothing between
        p.board[9][3] = RED_GENERAL;   // off the shared file, so flying general is not involved
        p.sideToMove = BLACK;
        check("Chariot check detected", isInCheck(p, BLACK));
    }

    // --- Cannon gives check with exactly one screen ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[2][4] = BLACK_SOLDIER; // the screen (either side's piece works)
        p.board[5][4] = RED_CANNON;    // jumps the screen to reach the general
        p.board[9][3] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Cannon check detected", isInCheck(p, BLACK));
    }

    // --- Cannon with NO screen should NOT be check ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[5][4] = RED_CANNON;    // clear path, so no screen, so no capture possible
        p.board[9][3] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Cannon with no screen is NOT check", !isInCheck(p, BLACK));
    }

    // --- Cannon with TWO screens should NOT be check ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[2][4] = BLACK_SOLDIER;
        p.board[3][4] = RED_SOLDIER;   // second screen blocks the capture
        p.board[5][4] = RED_CANNON;
        p.board[9][3] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Cannon with two screens is NOT check", !isInCheck(p, BLACK));
    }

    // --- Horse gives check ---
    // Horse at (2,5) reaches (0,4) via the (-2,-1) L-shape; its leg is (1,5), which is empty.
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[2][5] = RED_HORSE;
        p.board[9][3] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Horse check detected", isInCheck(p, BLACK));
    }

    // --- Same horse, but its leg is blocked: should NOT be check ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[2][5] = RED_HORSE;
        p.board[1][5] = BLACK_SOLDIER; // blocks the horse's leg
        p.board[9][3] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Horse with blocked leg is NOT check", !isInCheck(p, BLACK));
    }

    // --- Soldier gives check ---
    // A red soldier advances toward row 0, so one at (1,4) attacks (0,4).
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[1][4] = RED_SOLDIER;
        p.board[9][3] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Soldier check detected", isInCheck(p, BLACK));
    }

    // --- Flying general: two generals facing on an open file ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[9][4] = RED_GENERAL;   // same column, nothing between
        p.sideToMove = BLACK;
        check("Flying general detected", isInCheck(p, BLACK));
    }

    // --- Flying general blocked by a piece: NOT check ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[5][4] = RED_SOLDIER;   // blocks the file
        p.board[9][4] = RED_GENERAL;
        p.sideToMove = BLACK;
        check("Blocked flying general is NOT check", !isInCheck(p, BLACK));
    }

    // --- Quiet position: nothing threatens the general ---
    {
        Position p;
        clearBoard(p);
        p.board[0][4] = BLACK_GENERAL;
        p.board[9][3] = RED_GENERAL;
        p.board[5][0] = RED_CHARIOT;   // far away, wrong row and column
        p.sideToMove = BLACK;
        check("Quiet position is NOT check", !isInCheck(p, BLACK));
    }
}

// ---------------------------------------------------------------------------
// Test 3: findGeneral
// ---------------------------------------------------------------------------

static void testFindGeneral() {
    std::cout << "\n=== Test 3: findGeneral ===" << std::endl;

    Position p;
    clearBoard(p);
    p.board[0][4] = BLACK_GENERAL;
    p.board[9][3] = RED_GENERAL;
    p.sideToMove = RED;

    std::pair<int,int> blackPos = findGeneral(p, BLACK);
    std::pair<int,int> redPos = findGeneral(p, RED);

    check("finds black general", blackPos.first == 0 && blackPos.second == 4);
    check("finds red general", redPos.first == 9 && redPos.second == 3);

    // And on the untouched starting board
    Position startPosition;
    std::pair<int,int> startBlack = findGeneral(startPosition, BLACK);
    std::pair<int,int> startRed = findGeneral(startPosition, RED);
    check("start position black general at (0,4)", startBlack.first == 0 && startBlack.second == 4);
    check("start position red general at (9,4)", startRed.first == 9 && startRed.second == 4);
}

// ---------------------------------------------------------------------------
// Test 4: Evaluator
// ---------------------------------------------------------------------------

static void testEvaluator() {
    std::cout << "\n=== Test 4: Evaluator ===" << std::endl;

    // Starting position is perfectly symmetric, so material should be dead even.
    Position startPosition;
    checkEquals("starting position evaluates to 0", evaluateBoard(startPosition), 0);

    // Red up a chariot should be strongly positive (Red-positive convention).
    Position p;
    clearBoard(p);
    p.board[0][4] = BLACK_GENERAL;
    p.board[9][3] = RED_GENERAL;
    p.board[5][0] = RED_CHARIOT;
    p.sideToMove = RED;
    check("red up a chariot scores positive", evaluateBoard(p) > 0);

    // Mirror it: black up a chariot should be negative.
    Position q;
    clearBoard(q);
    q.board[0][4] = BLACK_GENERAL;
    q.board[9][3] = RED_GENERAL;
    q.board[5][0] = BLACK_CHARIOT;
    q.sideToMove = RED;
    check("black up a chariot scores negative", evaluateBoard(q) < 0);
}

// ---------------------------------------------------------------------------
// Test 5: Search picks up a free capture
// Red chariot at (5,4) can take an undefended black chariot at (5,6).
// Nothing defends it and nothing punishes the capture, so any correct search
// should play it.
// ---------------------------------------------------------------------------

static void testSearchFindsFreeCapture() {
    std::cout << "\n=== Test 5: Search finds a free capture ===" << std::endl;

    Position p;
    clearBoard(p);
    p.board[0][3] = BLACK_GENERAL;
    p.board[9][4] = RED_GENERAL;
    p.board[5][4] = RED_CHARIOT;
    p.board[5][6] = BLACK_CHARIOT;  // undefended, two squares away on an open row
    p.sideToMove = RED;

    Move best = findBestMove(p, 2);

    std::cout << "  engine played: (" << best.fromRow << "," << best.fromCol
              << ") -> (" << best.toRow << "," << best.toCol << ")" << std::endl;

    check("captures the hanging chariot",
          best.fromRow == 5 && best.fromCol == 4 &&
          best.toRow == 5 && best.toCol == 6);
}

// ---------------------------------------------------------------------------
// Test 6: isGameOver
// ---------------------------------------------------------------------------

static void testGameOver() {
    std::cout << "\n=== Test 6: isGameOver ===" << std::endl;

    // Starting position: game obviously continues.
    Position startPosition;
    check("start position continues", isGameOver(startPosition) == CONTINUE);
}

// ---------------------------------------------------------------------------

static void testPieceSymmetry() {
    std::cout << "\n=== Piece value symmetry ===" << std::endl;
    Piece redPieces[6]  = {RED_CHARIOT, RED_CANNON, RED_HORSE, RED_ELEPHANT, RED_ADVISOR, RED_SOLDIER};
    Piece blackPieces[6] = {BLACK_CHARIOT, BLACK_CANNON, BLACK_HORSE, BLACK_ELEPHANT, BLACK_ADVISOR, BLACK_SOLDIER};
    const char* names[6] = {"chariot", "cannon", "horse", "elephant", "advisor", "soldier"};

    for (int i = 0; i < 6; i++) {
        Position p;
        clearBoard(p);
        p.board[0][3] = BLACK_GENERAL;
        p.board[9][4] = RED_GENERAL;
        p.board[6][0] = redPieces[i];    // red side of river
        p.board[3][0] = blackPieces[i];  // mirrored, black side of river
        p.sideToMove = RED;
        checkEquals(names[i], evaluateBoard(p), 0);
    }
}

int main() {
    std::cout << "Running Xiangqi engine tests..." << std::endl;

    testFindGeneral();
    testCheckDetection();
    testEvaluator();
    testPieceSymmetry();
    testGameOver();
    testSearchFindsFreeCapture();
    testPerft();   // slowest, so run it last

    std::cout << "\n===================================" << std::endl;
    std::cout << "Passed: " << passCount << "   Failed: " << failCount << std::endl;
    std::cout << "===================================" << std::endl;

    return failCount == 0 ? 0 : 1;
}