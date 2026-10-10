#include "rubiks/Cube.hpp"
#include <cassert>
#include <iostream>

int main() {
    using rubiks::Color;
    using rubiks::Face;
    using rubiks::FaceColorArray;

    // -- Test 1: Check up face color is correctly accessed -- 
    FaceColorArray upFace = {{
        {Color::R, Color::G, Color::B},
        {Color::Y, Color::W, Color::O},
        {Color::G, Color::R, Color::Y}
    }};

    FaceColorArray downFace = {{
        {Color::Y, Color::B, Color::W},
        {Color::G, Color::R, Color::O},
        {Color::O, Color::B, Color::W}
    }};

    FaceColorArray leftFace = {{
        {Color::W, Color::B, Color::R},
        {Color::B, Color::Y, Color::G},
        {Color::R, Color::G, Color::O}
    }};

    FaceColorArray rightFace = {{
        {Color::O, Color::W, Color::O},
        {Color::B, Color::Y, Color::G},
        {Color::R, Color::G, Color::R}
    }};

    FaceColorArray frontFace = {{
        {Color::B, Color::W, Color::W},
        {Color::Y, Color::B, Color::B},
        {Color::Y, Color::B, Color::G}
    }};

    FaceColorArray backFace = {{
        {Color::W, Color::W, Color::Y},
        {Color::O, Color::G, Color::B},
        {Color::R, Color::B, Color::O}
    }};

    rubiks::Cube cube{
        Face(upFace),
        Face(downFace),
        Face(leftFace),
        Face(rightFace),
        Face(frontFace),
        Face(backFace)
    };

    assert(cube.getUpFace().getColor(0, 1) == upFace[0][1]);
    assert(cube.getDownFace().getColor(1, 2) == downFace[1][2]);
    assert(cube.getLeftFace().getColor(2, 0) == leftFace[2][0]);
    assert(cube.getRightFace().getColor(1, 1) == rightFace[1][1]);
    assert(cube.getFrontFace().getColor(0, 2) == frontFace[0][2]);
    assert(cube.getBackFace().getColor(2, 1) == backFace[2][1]);

    std::cout << "Cube tests passed!\n";
}
