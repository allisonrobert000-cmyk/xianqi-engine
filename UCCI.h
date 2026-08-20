#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "position.h"
#include "move.h"
#include "search.h"

Move parseUcciMove(const std::string& moveStr);

std::string moveToUcci(Move move);

static Piece fenCharToPiece(char c);

static std::vector<std::string> splitOnSpaces(const std::string& line);

void runUcciLoop();
