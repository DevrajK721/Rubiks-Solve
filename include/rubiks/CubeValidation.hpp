#pragma once

#include "rubiks/Cube.hpp"
#include <string>

namespace rubiks {
    // Returns an empty string for a possible cube, or the first problem found.
    // The six centres define the colour scheme. The cube is not changed.
    std::string validateCube(const Cube& cube);
}
