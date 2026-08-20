#include "UCCI.h"

Move parseUcciMove(const std::string& moveStr) {
    Move move;
    move.fromCol = moveStr[0] - 'a';
    move.fromRow = moveStr[1] - '0';
    move.toCol = moveStr[2] - 'a';
    move.toRow = moveStr[3] - '0';
    return move;
}

std::string moveToUcci(Move move) {
    std::string result;
    result += ('a' + move.fromCol);
    result += ('0' + move.fromRow);
    result += ('a' + move.toCol);
    result += ('0' + move.toRow);
    return result;
}

static Piece fenCharToPiece(char c) {
    switch(c) {
        case 'R': return RED_CHARIOT;
        case 'N': return RED_HORSE;
        case 'B': return RED_ELEPHANT;
        case 'A': return RED_ADVISOR;
        case 'K': return RED_GENERAL;
        case 'C': return RED_CANNON;
        case 'P': return RED_SOLDIER;
        case 'r': return BLACK_CHARIOT;
        case 'n': return BLACK_HORSE;
        case 'b': return BLACK_ELEPHANT;
        case 'a': return BLACK_ADVISOR;
        case 'k': return BLACK_GENERAL;
        case 'c': return BLACK_CANNON;
        case 'p': return BLACK_SOLDIER;
        default: return EMPTY;
    }
}

Position parseFen(const std::string& fen) {
    Position position;
 
    // Start empty -- the FEN board section will fill in every occupied square.
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 9; col++) {
            position.board[row][col] = EMPTY;
        }
    }
 
    int row = 0;
    int col = 0;
    size_t i = 0;  // current index into the fen string
 
    // --- Parse the board section, up to the first space ---
    while (i < fen.size() && fen[i] != ' ') {
        char c = fen[i];
 
        if (c == '/') {
            // End of this row: move to the next row down, reset column.
            row++;
            col = 0;
        } else if (c >= '1' && c <= '9') {
            // A digit means "this many empty squares in a row."
            // Same char-to-int trick as the move parser: c - '0'.
            int emptyCount = c - '0';
            col += emptyCount;
        } else {
            // Anything else is a piece letter.
            position.board[row][col] = fenCharToPiece(c);
            col++;
        }
        i++;
    }
 
    // --- Skip the space, then read the side-to-move field ('w' or 'b') ---
    i++;  // skip the space itself
    if (i < fen.size()) {
        char sideChar = fen[i];
        position.sideToMove = (sideChar == 'w') ? RED : BLACK;
    }
 
    // The remaining FEN fields (castling/en-passant placeholders, halfmove
    // clock, fullmove number) aren't used by this engine, so they're
    // deliberately not parsed here.
 
    return position;
}

// Splits a string on spaces into a vector of tokens.
// Used to break a command line like "position fen ... moves h2e2 h7e7"
// into individual words that can be processed one at a time.
static std::vector<std::string> splitOnSpaces(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;

    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ' ') {
            if (!current.empty()) {
                tokens.push_back(current);
                current = "";
            }
        } else {
            current += line[i];
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

void runUcciLoop() {
    Position currentPosition;  // persists across loop iterations
    std::string line;

    while (std::getline(std::cin, line)) {

        if (line == "ucci") {
            std::cout << "id name MyXiangqiEngine" << std::endl;
            std::cout << "id author YourName" << std::endl;
            std::cout << "ucciok" << std::endl;
        }

        else if (line == "uci") {
            std::cout << "id name MyXiangqiEngine" << std::endl;
            std::cout << "id author YourName" << std::endl;
            std::cout << "option name UCI_Variant type combo default xiangqi var xiangqi" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        else if (line == "isready") {
            std::cout << "readyok" << std::endl;
        }

        else if (line.substr(0, 8) == "position") {
            std::vector<std::string> tokens = splitOnSpaces(line);
            // tokens[0] = "position"
            // tokens[1] = either "startpos" or "fen"

            size_t tokenIndex = 1;

            if (tokens[tokenIndex] == "startpos") {
                currentPosition = Position();  // your constructor's default opening layout
                tokenIndex++;
            } else if (tokens[tokenIndex] == "fen") {
                tokenIndex++;
                // The FEN board string is one token; the side-to-move letter
                // ('w' or 'b') is the token right after it. Reassemble both
                // with a space, since parseFen expects "<board> <side>".
                std::string fenString = tokens[tokenIndex] + " " + tokens[tokenIndex + 1];
                currentPosition = parseFen(fenString);
                tokenIndex += 2;
                // NOTE: a real FEN has more trailing fields (castling
                // placeholder, halfmove clock, fullmove number) which are
                // skipped here since parseFen doesn't use them. If those
                // extra fields appear before "moves", this simple version
                // won't account for them -- see the note below the function.
            }

            // If the line continues with "moves <move1> <move2> ...",
            // apply each one in sequence to currentPosition.
            if (tokenIndex < tokens.size() && tokens[tokenIndex] == "moves") {
                tokenIndex++;
                for (size_t i = tokenIndex; i < tokens.size(); i++) {
                    Move m = parseUcciMove(tokens[i]);
                    currentPosition = makeMove(currentPosition, m);
                }
            }
        }

        else if (line.substr(0, 2) == "go") {
            Move best = findBestMove(currentPosition, 4);  // depth 4, adjust as needed
            std::cout << "bestmove " << moveToUcci(best) << std::endl;
        }

                else if (line == "legalmoves") {
            std::vector<Move> legal = validateMoves(currentPosition, generateMoves(currentPosition));
            std::cout << "legalmoves";
            for (size_t i = 0; i < legal.size(); i++) {
                std::cout << " " << moveToUcci(legal[i]);
            }
            std::cout << std::endl;
        }
 
        else if (line == "status") {
            GameStatus status = isGameOver(currentPosition);
            if (status == CHECKMATE) {
                std::cout << "status checkmate" << std::endl;
            } else if (status == STALEMATE) {
                std::cout << "status stalemate" << std::endl;
            } else {
                std::cout << "status continue" << std::endl;
            }
        }

        else if (line == "incheck") {
            if (isInCheck(currentPosition, currentPosition.sideToMove)) {
                std::cout << "incheck yes" << std::endl;
            } else {
                std::cout << "incheck no" << std::endl;
            }
        }

        else if (line == "quit") {
            break;
        }
    }
}