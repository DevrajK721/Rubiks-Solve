#include "rubiks/CubeView.hpp"
#include "rubiks/CubeValidation.hpp"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>

using rubiks::Cube;
using rubiks::CubeView;
using rubiks::Face;
using rubiks::FaceColorArray;
using StickerColor = rubiks::Color;
using CubeColors = std::array<FaceColorArray, 6>;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "View test failed: " << description << '\n';
        std::exit(1);
    }
}

Cube makeCube(const CubeColors& colors) {
    return Cube{Face(colors[0]), Face(colors[1]), Face(colors[2]),
                Face(colors[3]), Face(colors[4]), Face(colors[5])};
}

CubeColors readCube(const Cube& cube) {
    return {{cube.getUpFace().getColors(), cube.getDownFace().getColors(),
             cube.getLeftFace().getColors(), cube.getRightFace().getColors(),
             cube.getFrontFace().getColors(), cube.getBackFace().getColors()}};
}

CubeColors solvedColors() {
    const StickerColor centres[] = {
        StickerColor::W, StickerColor::Y, StickerColor::O,
        StickerColor::R, StickerColor::G, StickerColor::B
    };
    CubeColors colors;
    for (int face = 0; face < 6; ++face) {
        for (auto& row : colors[face]) row.fill(centres[face]);
    }
    return colors;
}

// Isolate one sticker so we can check its position AND facing direction.
// Numbering: U 0..8, D 9..17, L 18..26, R 27..35, F 36..44, B 45..53.
CubeColors markerAt(int index) {
    CubeColors colors;
    for (auto& face : colors) {
        for (auto& row : face) row.fill(StickerColor::W);
    }
    colors[index / 9][index % 9 / 3][index % 3] = StickerColor::R;
    return colors;
}

Vector3 stickerPosition(int index) {
    float row = index % 9 / 3;
    float col = index % 3;
    const Vector3 positions[] = {
        {col - 1, 1, row - 1},  {col - 1, -1, 1 - row},
        {-1, 1 - row, col - 1}, {1, 1 - row, 1 - col},
        {col - 1, 1 - row, 1},  {1 - col, 1 - row, -1}
    };
    return positions[index / 9];
}

Vector3 stickerNormal(int index) {
    const Vector3 normals[] = {
        {0, 1, 0}, {0, -1, 0}, {-1, 0, 0},
        {1, 0, 0}, {0, 0, 1}, {0, 0, -1}
    };
    return normals[index / 9];
}

void checkNear(Vector3 actual, Vector3 expected) {
    check(Vector3Distance(actual, expected) < 0.0005f,
          "displayed sticker position or direction changed unexpectedly");
}

void checkThresholds() {
    CubeColors original = solvedColors();
    Cube cube = makeCube(original);
    CubeView view;

    view.rotate(cube, 44, 0);
    check(readCube(cube) == original, "no reorientation before halfway");
    view.rotate(cube, 1, 0);
    check(readCube(cube) == original, "keep the existing orientation at a tie");
    view.rotate(cube, 1, 0);
    Cube expected = makeCube(original);
    expected.rotateWholeRight();
    check(readCube(cube) == readCube(expected), "right reorientation past halfway");
    checkNear(Vector3Transform({0, 0, 1}, view.getDrawRotation()),
              Vector3RotateByAxisAngle({0, 0, 1}, {0, 1, 0}, -44 * DEG2RAD));

    view.rotate(cube, -2, 0);
    check(readCube(cube) == original, "reverse threshold crossing");
    view.rotate(cube, -44, 46);
    expected = makeCube(original);
    expected.rotateWholeUp();
    check(readCube(cube) == readCube(expected), "vertical threshold crossing");

    CubeColors paused = readCube(cube);
    Matrix pausedRotation = view.getDrawRotation();
    view.rotate(cube, 0, 0);
    check(readCube(cube) == paused, "no input must preserve the cube");
    checkNear(Vector3Transform({0, 0, 1}, view.getDrawRotation()),
              Vector3Transform({0, 0, 1}, pausedRotation));
}

void checkVisualContinuity(const Camera3D& camera,
                           Vector3 horizontalAxis, Vector3 verticalAxis) {
    // Reference motion is calculated with vector rotations, independently of
    // the helper's quaternions and its choice of control orientation.
    for (int source = 0; source < 54; ++source) {
        Cube cube = makeCube(markerAt(source));
        CubeView view(camera);
        Vector3 expectedPosition = stickerPosition(source);
        Vector3 expectedNormal = stickerNormal(source);

        for (int step = 0; step < 240; ++step) {
            float right = (step % 5 - 2) * 7.0f;
            float up = (step % 7 - 3) * 9.0f;
            view.rotate(cube, right, up);

            expectedPosition = Vector3RotateByAxisAngle(
                expectedPosition, horizontalAxis, right * DEG2RAD);
            expectedPosition = Vector3RotateByAxisAngle(
                expectedPosition, verticalAxis, -up * DEG2RAD);
            expectedNormal = Vector3RotateByAxisAngle(
                expectedNormal, horizontalAxis, right * DEG2RAD);
            expectedNormal = Vector3RotateByAxisAngle(
                expectedNormal, verticalAxis, -up * DEG2RAD);

            CubeColors colors = readCube(cube);
            int destination = -1;
            for (int i = 0; i < 54; ++i) {
                if (colors[i / 9][i % 9 / 3][i % 3] == StickerColor::R) {
                    destination = i;
                }
            }
            check(destination >= 0, "marker sticker must not disappear");
            checkNear(Vector3Transform(stickerPosition(destination), view.getDrawRotation()),
                      expectedPosition);
            checkNear(Vector3Transform(stickerNormal(destination), view.getDrawRotation()),
                      expectedNormal);
        }
    }
}

void checkMovesWhileRotated() {
    Cube cube = makeCube(solvedColors());
    CubeView view;
    view.rotate(cube, 46, 0);
    cube.L();
    view.rotate(cube, -46, 0);

    Cube expected = makeCube(solvedColors());
    expected.rotateWholeRight();
    expected.L();
    expected.rotateWholeLeft();
    check(readCube(cube) == readCube(expected),
          "L must use the new orientation and survive rotating back");

    std::mt19937 random(7);
    for (int i = 0; i < 1000; ++i) {
        float right = static_cast<int>(random() % 41) - 20;
        float up = static_cast<int>(random() % 41) - 20;
        view.rotate(cube, right, up);
        cube.L(); cube.M(); cube.Eprime(); cube.S();
        check(rubiks::validateCube(cube).empty(), "moves and rotations must remain legal");
    }
}

void checkFullRevolution() {
    for (int source = 0; source < 54; ++source) {
        Cube cube = makeCube(markerAt(source));
        CubeView view;
        for (int step = 0; step < 360; ++step) view.rotate(cube, 1, 0);
        check(readCube(cube) == markerAt(source), "a full revolution restores the cube");
        checkNear(Vector3Transform(stickerPosition(source), view.getDrawRotation()),
                  stickerPosition(source));
    }
}

int main() {
    checkThresholds();

    Camera3D frontCamera = {};
    frontCamera.position = {0, 0, 10};
    frontCamera.up = {0, 1, 0};
    checkVisualContinuity(frontCamera, {0, 1, 0}, {1, 0, 0});

    // The viewer's diagonal camera has these screen axes analytically.
    // Test against them rather than repeating the constructor's cross products.
    Camera3D angledCamera = frontCamera;
    angledCamera.position = {6, 6, 6};
    float inverseRoot2 = 1.0f / std::sqrt(2.0f);
    float inverseRoot6 = 1.0f / std::sqrt(6.0f);
    checkVisualContinuity(angledCamera,
        {-inverseRoot6, 2 * inverseRoot6, -inverseRoot6},
        {inverseRoot2, 0, -inverseRoot2});

    checkMovesWhileRotated();
    checkFullRevolution();
    std::cout << "View tests passed: thresholds, visual continuity, mixed moves, and revolutions.\n";
}
