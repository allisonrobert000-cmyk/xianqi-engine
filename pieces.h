//Declares enums for all possible pieces and for side to move
#pragma once

enum Piece { 
    EMPTY, 
    RED_GENERAL, 
    BLACK_GENERAL, 
    RED_ADVISOR, 
    BLACK_ADVISOR, 
    RED_ELEPHANT, 
    BLACK_ELEPHANT, 
    RED_HORSE, 
    BLACK_HORSE, 
    RED_CANNON, 
    BLACK_CANNON, 
    RED_CHARIOT, 
    BLACK_CHARIOT, 
    RED_SOLDIER, 
    BLACK_SOLDIER 
};

enum Side {
    RED,
    BLACK
};