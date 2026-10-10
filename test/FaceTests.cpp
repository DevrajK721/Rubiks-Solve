#include "rubiks/Face.hpp"
#include <cassert>
#include <iostream>

int main() {
    using rubiks::Color;
    using rubiks::Face;
    using rubiks::FaceColorArray;

    // -- Test 1: Create a Face and check colors -- 
    FaceColorArray input = {{
        {Color::R, Color::G, Color::B},
        {Color::Y, Color::W, Color::O},
        {Color::G, Color::R, Color::Y}
    }};

    Face face(input);

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            assert(face.getColor(row, col) == input[row][col]);
        }
    }

    std::cout << "Face tests passed!\n";
}
