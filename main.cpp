#include "pieces.h"
#include "position.h"
#include "UCCI.h"
#include <iostream>
using namespace std;

string pieceToString(Piece piece) {
    switch (piece)
    {
        case EMPTY: return " . ";
        case BLACK_ADVISOR: return "B_AD";
        case BLACK_CANNON: return "B_CA";
        case BLACK_CHARIOT: return "B_CH";
        case BLACK_ELEPHANT: return "B_EL";
        case BLACK_GENERAL: return "B_GE";
        case BLACK_HORSE: return "B_HO";
        case BLACK_SOLDIER: return "B_SO";
        case RED_ADVISOR: return "R_AD";
        case RED_CANNON: return "R_CA";
        case RED_CHARIOT: return "R_CH";
        case RED_ELEPHANT: return "R_EL";
        case RED_GENERAL: return "R_GE";
        case RED_HORSE: return "R_HO";
        case RED_SOLDIER: return "R_SO";
        default: return "????";
        
    }
}


int main() {
    runUcciLoop();
    return 0;
}