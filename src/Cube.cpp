#include "rubiks/Cube.hpp"
#include "rubiks/CubeValidation.hpp"
#include <stdexcept>

namespace rubiks {
    Cube::Cube(Face up, Face down, Face left, Face right, Face front, Face back) 
        : up(up), down(down), left(left), right(right), front(front), back(back) {}
    
    Face Cube::getUpFace() const {return up;} 
    Face Cube::getDownFace() const {return down;}
    Face Cube::getLeftFace() const {return left;}
    Face Cube::getRightFace() const {return right;}
    Face Cube::getFrontFace() const {return front;}
    Face Cube::getBackFace() const {return back;}

    Cube Cube::copy() const {
        // Faces contain arrays by value, so the normal C++ copy is independent.
        return *this;
    }

    bool Cube::isSolved() const {
        const FaceColorArray faces[] = {
            up.getColors(), down.getColors(), left.getColors(),
            right.getColors(), front.getColors(), back.getColors()
        };

        for (const auto& face : faces) {
            Color centre = face[1][1];
            for (const auto& row : face) {
                for (Color colour : row) {
                    if (colour != centre) return false;
                }
            }
        }

        // Uniform faces alone would incorrectly call an all-red cube solved.
        return validateCube(*this).empty();
    }

    std::string Cube::stateKey() const {
        const FaceColorArray faces[] = {
            up.getColors(), down.getColors(), left.getColors(),
            right.getColors(), front.getColors(), back.getColors()
        };
        std::string key;
        key.reserve(54);

        for (const auto& face : faces) {
            for (const auto& row : face) {
                for (Color colour : row) {
                    switch (colour) {
                        case Color::R: key += 'R'; break;
                        case Color::O: key += 'O'; break;
                        case Color::Y: key += 'Y'; break;
                        case Color::G: key += 'G'; break;
                        case Color::B: key += 'B'; break;
                        case Color::W: key += 'W'; break;
                        default:
                            throw std::invalid_argument("Cannot encode an unknown sticker colour.");
                    }
                }
            }
        }

        return key;
    }
}
