#include "rubiks/Face.hpp"

namespace rubiks {
    Face::Face(FaceColorArray colors) : colors(colors){}

    Color Face::getColor(int row, int col) const { // const does not allow function to change the face
        return colors[row][col];
    }

    FaceColorArray Face::getColors() const {
        return colors;
    }

    void Face::setColors(const FaceColorArray& newColors) {
        colors = newColors;
    }
}
