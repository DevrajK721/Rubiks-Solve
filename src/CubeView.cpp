#include "rubiks/CubeView.hpp"
#include "raymath.h"
#include <cmath>

namespace rubiks {
namespace {
    Quaternion arrowRotation(char arrow) {
        if (arrow == '^') {
            return QuaternionFromAxisAngle({1, 0, 0}, -90.0f * DEG2RAD);
        }
        float angle = (arrow == '>') ? 90.0f : -90.0f;
        return QuaternionFromAxisAngle({0, 1, 0}, angle * DEG2RAD);
    }

    // Compare orientations. A larger score means a closer match.
    // q and -q describe the same rotation, hence the absolute value.
    float similarity(Quaternion a, Quaternion b) {
        return std::fabs(a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w);
    }

    void applyPath(Cube& cube, const std::string& path, bool undo) {
        int count = static_cast<int>(path.size());
        for (int i = 0; i < count; ++i) {
            // To undo a sequence, reverse both the order and each direction.
            char arrow = path[undo ? count - 1 - i : i];
            if (arrow == '>') {
                if (undo) cube.rotateWholeLeft();
                else      cube.rotateWholeRight();
            } else if (arrow == '<') {
                if (undo) cube.rotateWholeRight();
                else      cube.rotateWholeLeft();
            } else if (arrow == '^') {
                if (undo) cube.rotateWholeDown();
                else      cube.rotateWholeUp();
            }
        }
    }
}

CubeView::CubeView() {
    // Six choices of top face, with four spins for each = 24 orientations.
    // These strings describe WHOLE-CUBE turns, not face moves like U or R.
    const char* topPaths[] = {"", "^", "^^", "^^^", ">^", "<^"};
    int index = 0;

    for (const char* topPath : topPaths) {
        for (int spin = 0; spin < 4; ++spin) {
            Orientation orientation;
            orientation.path = std::string(topPath) + std::string(spin, '>');
            orientation.rotation = QuaternionIdentity();

            for (char arrow : orientation.path) {
                orientation.rotation = QuaternionMultiply(
                    arrowRotation(arrow), orientation.rotation);
            }
            orientation.rotation = QuaternionNormalize(orientation.rotation);
            orientations[index++] = orientation;
        }
    }
}

CubeView::CubeView(const Camera3D& camera) : CubeView() {
    // Use the camera's screen-right and screen-up directions, rather than
    // world X/Y. With an angled camera, world axes appear tilted on screen.
    Vector3 towardCamera = Vector3Normalize(
        Vector3Subtract(camera.position, camera.target));
    screenRight = Vector3Normalize(Vector3CrossProduct(camera.up, towardCamera));
    screenUp = Vector3Normalize(Vector3CrossProduct(towardCamera, screenRight));
}

void CubeView::rotate(Cube& cube, float rightDegrees, float upDegrees) {
    if (rightDegrees == 0.0f && upDegrees == 0.0f) return;

    // Apply small visual rotations. Raymath takes radians, not degrees.
    Quaternion horizontal = QuaternionFromAxisAngle(
        screenUp, rightDegrees * DEG2RAD);
    Quaternion vertical = QuaternionFromAxisAngle(
        screenRight, -upDegrees * DEG2RAD);

    visualRotation = QuaternionMultiply(horizontal, visualRotation);
    visualRotation = QuaternionMultiply(vertical, visualRotation);
    visualRotation = QuaternionNormalize(visualRotation);

    // Keep the current interpretation at an exact tie. In a straight-on view,
    // a single-axis turn changes interpretation just beyond 45 degrees.
    int nearest = activeOrientation;
    float best = similarity(visualRotation, orientations[nearest].rotation);
    for (int i = 0; i < 24; ++i) {
        float score = similarity(visualRotation, orientations[i].rotation);
        if (score > best + 0.00001f) {
            best = score;
            nearest = i;
        }
    }

    if (nearest != activeOrientation) {
        // Only undo the old coordinate orientation, NOT the user's face moves.
        applyPath(cube, orientations[activeOrientation].path, true);
        applyPath(cube, orientations[nearest].path, false);
        activeOrientation = nearest;
    }
}

Matrix CubeView::getDrawRotation() const {
    // The stored cube has already been reoriented. Cancel that part visually
    // so the remaining drawing rotation preserves the smooth overall angle.
    Quaternion remaining = QuaternionMultiply(
        visualRotation,
        QuaternionInvert(orientations[activeOrientation].rotation));
    return QuaternionToMatrix(remaining);
}
}
