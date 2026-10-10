#pragma once

#include "rubiks/Color.hpp"
#include <array>

namespace rubiks {
    using FaceColorArray = std::array<std::array<Color, 3>, 3>;

    class Face {
        public:
            Face(FaceColorArray colors); // Position {1, 1} will dictate the face color.
            Color getColor(int row, int col) const; // Returns the color of the square at the specified row and column.
            FaceColorArray getColors() const; // Returns the colors of the face.
            void setColors(const FaceColorArray& newColors); // Sets the colors of the face.
        private:
            FaceColorArray colors;
    };
}
