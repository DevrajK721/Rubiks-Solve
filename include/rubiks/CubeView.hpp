#pragma once

#include "raylib.h"
#include "rubiks/Cube.hpp"
#include <array>
#include <string>

namespace rubiks {
class CubeView {
public:
    // The default axes suit a camera looking straight along Z.
    CubeView();
    explicit CubeView(const Camera3D& camera);

    // Positive angles turn right/up relative to the camera, in degrees.
    void rotate(Cube& cube, float rightDegrees, float upDegrees);
    Matrix getDrawRotation() const;

private:
    struct Orientation {
        Quaternion rotation;
        std::string path; // Whole-cube arrow turns: >, <, ^.
    };

    std::array<Orientation, 24> orientations;
    int activeOrientation = 0;
    Quaternion visualRotation = {0.0f, 0.0f, 0.0f, 1.0f};
    Vector3 screenUp = {0.0f, 1.0f, 0.0f};
    Vector3 screenRight = {1.0f, 0.0f, 0.0f};
};
}
