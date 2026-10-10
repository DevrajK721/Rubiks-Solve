#include "rubiks/Cube.hpp"

namespace rubiks {
    Cube::Cube(Face up, Face down, Face left, Face right, Face front, Face back) 
        : up(up), down(down), left(left), right(right), front(front), back(back) {}
    
    Face Cube::getUpFace() const {return up;} 
    Face Cube::getDownFace() const {return down;}
    Face Cube::getLeftFace() const {return left;}
    Face Cube::getRightFace() const {return right;}
    Face Cube::getFrontFace() const {return front;}
    Face Cube::getBackFace() const {return back;}

}
