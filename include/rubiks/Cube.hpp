#pragma once 

#include "rubiks/Face.hpp"

namespace rubiks {
    class Cube {
        public:
            Cube(Face up, Face down, Face left, Face right, Face front, Face back); // Constructor to initialize the cube's 6 faces. 
            
            Face getUpFace() const; // Returns the up face of the cube.
            Face getDownFace() const; // Returns the down face of the cube.
            Face getLeftFace() const; // Returns the left face of the cube.
            Face getRightFace() const; // Returns the right face of the cube.
            Face getFrontFace() const; // Returns the front face of the cube.
            Face getBackFace() const; // Returns the back face of the cube.
            
            void L(); // Rotate left face clockwise.
            void Lprime(); // Rotate left face counter-clockwise.
            void L2(); // Rotate left face 180 degrees.
            void R(); // Rotate right face clockwise.
            void Rprime(); // Rotate right face counter-clockwise.
            void R2(); // Rotate right face 180 degrees.
            void U(); // Rotate up face clockwise.
            void Uprime(); // Rotate up face counter-clockwise.
            void U2(); // Rotate up face 180 degrees.
            void D(); // Rotate down face clockwise.
            void Dprime(); // Rotate down face counter-clockwise.
            void D2(); // Rotate down face 180 degrees.
            void F(); // Rotate front face clockwise.
            void Fprime(); // Rotate front face counter-clockwise.
            void F2(); // Rotate front face 180 degrees.
            void B(); // Rotate back face clockwise.
            void Bprime(); // Rotate back face counter-clockwise.
            void B2(); // Rotate back face 180 degrees.
            void M(); // Turn the middle slice in the same direction as L.
            void Mprime(); // Turn the middle slice in the opposite direction to L.
            void M2(); // Rotate middle layer 180 degrees.

        private:
            Face up; 
            Face down; 
            Face left; 
            Face right; 
            Face front; 
            Face back; 

            void rotateFaceClockwise(Face& face); 
    };
}
