#include "rubiks/CubeValidation.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>

using rubiks::Color;
using rubiks::Cube;
using rubiks::Face;
using rubiks::FaceColorArray;
using rubiks::validateCube;

using CubeColors = std::array<FaceColorArray, 6>;
enum FacePosition { U, D, L, R, F, B };

CubeColors solvedColors() {
    const Color centres[6] = {Color::W, Color::Y, Color::O,
                              Color::R, Color::G, Color::B};
    CubeColors colors;
    for (int face = 0; face < 6; ++face) {
        for (auto& row : colors[face]) {
            row.fill(centres[face]);
        }
    }
    return colors;
}

Cube makeCube(const CubeColors& colors) {
    return Cube{Face(colors[U]), Face(colors[D]), Face(colors[L]),
                Face(colors[R]), Face(colors[F]), Face(colors[B])};
}

CubeColors readCube(const Cube& cube) {
    return {{cube.getUpFace().getColors(), cube.getDownFace().getColors(),
             cube.getLeftFace().getColors(), cube.getRightFace().getColors(),
             cube.getFrontFace().getColors(), cube.getBackFace().getColors()}};
}

void check(const Cube& cube, const char* name, const std::string& expectedError = "") {
    CubeColors before = readCube(cube);
    std::string error = validateCube(cube);
    bool correct = expectedError.empty() ? error.empty()
                                        : error.find(expectedError) != std::string::npos;
    if (!correct || readCube(cube) != before) {
        std::cerr << name << " failed. Expected: " << expectedError
                  << "; received: " << error << '\n';
        std::exit(1);
    }
}

void cycleColors(Color& a, Color& b, Color& c) {
    Color oldA = a;
    a = b;
    b = c;
    c = oldA;
}

void swapTwoEdges(CubeColors& colors) {
    // Exchange the complete UR and UF edges without flipping them.
    std::swap(colors[U][1][2], colors[U][2][1]);
    std::swap(colors[R][0][1], colors[F][0][1]);
}

void swapTwoCorners(CubeColors& colors) {
    // Exchange the complete URF and UFL corners without twisting them.
    std::swap(colors[U][2][2], colors[U][2][0]);
    std::swap(colors[R][0][0], colors[F][0][0]);
    std::swap(colors[F][0][2], colors[L][0][2]);
}

int main() {
    check(makeCube(solvedColors()), "solved cube");

    CubeColors colors = solvedColors();
    colors[U][0][0] = static_cast<Color>(-1);
    check(makeCube(colors), "negative colour value", "unknown colour");
    colors[U][0][0] = static_cast<Color>(6);
    check(makeCube(colors), "out-of-range colour value", "unknown colour");

    colors = solvedColors();
    colors[U][0][0] = Color::R;
    check(makeCube(colors), "wrong colour count", "exactly nine");

    colors = solvedColors();
    // Preserve the counts while giving U the same centre as R.
    std::swap(colors[U][1][1], colors[R][0][0]);
    check(makeCube(colors), "duplicate centres", "centre colours");

    colors = solvedColors();
    std::swap(colors[U][1][1], colors[F][1][1]);
    check(makeCube(colors), "two centres swapped alone", "impossible");

    colors = solvedColors();
    std::swap(colors[F][0][1], colors[U][1][2]);
    check(makeCube(colors), "same colour twice on an edge", "impossible colour pair");

    colors = solvedColors();
    std::swap(colors[F][0][1], colors[D][1][2]);
    check(makeCube(colors), "opposite colours on an edge", "impossible colour pair");

    colors = solvedColors();
    std::swap(colors[R][0][1], colors[F][2][1]);
    check(makeCube(colors), "duplicate edges with correct counts", "edge piece appears");

    colors = solvedColors();
    std::swap(colors[R][0][0], colors[U][2][0]);
    check(makeCube(colors), "invalid corner colours", "corner has impossible");

    colors = solvedColors();
    std::swap(colors[R][0][0], colors[F][0][2]);
    check(makeCube(colors), "mirrored corner", "mirrored sticker order");

    colors = solvedColors();
    // Replace URF with another UFL, and DLF with another DFR.
    // All six colour counts remain nine, but two corners are duplicated.
    colors[R][0][0] = Color::G;
    colors[F][0][2] = Color::O;
    colors[L][2][2] = Color::G;
    colors[F][2][0] = Color::R;
    check(makeCube(colors), "duplicate corners with correct counts", "corner piece appears");

    colors = solvedColors();
    std::swap(colors[U][2][1], colors[F][0][1]);
    check(makeCube(colors), "single flipped edge", "odd number of edges");
    std::swap(colors[U][1][2], colors[R][0][1]);
    check(makeCube(colors), "two flipped edges are possible");

    colors = solvedColors();
    cycleColors(colors[U][2][2], colors[R][0][0], colors[F][0][2]);
    check(makeCube(colors), "single twisted corner", "corner twists do not balance");
    cycleColors(colors[U][2][2], colors[R][0][0], colors[F][0][2]);
    check(makeCube(colors), "single corner twisted the other way", "corner twists do not balance");
    cycleColors(colors[U][2][0], colors[F][0][0], colors[L][0][2]);
    check(makeCube(colors), "opposite corner twists are possible");

    colors = solvedColors();
    swapTwoEdges(colors);
    check(makeCube(colors), "only two edges swapped", "permutation parity");
    swapTwoCorners(colors);
    check(makeCube(colors), "matching edge and corner swaps are possible");

    colors = solvedColors();
    swapTwoCorners(colors);
    check(makeCube(colors), "only two corners swapped", "permutation parity");

    // Colour names do not fix the scheme: consistent global relabelling is OK.
    colors = solvedColors();
    for (auto& face : colors) {
        for (auto& row : face) {
            for (Color& color : row) {
                if (color == Color::W) color = Color::R;
                else if (color == Color::R) color = Color::W;
            }
        }
    }
    check(makeCube(colors), "different centre-defined colour scheme");

    // Every supported move, including centre-moving M, preserves possibility.
    using MoveFunction = void (Cube::*)();
    const MoveFunction moves[] = {
        &Cube::L, &Cube::Lprime, &Cube::L2,
        &Cube::R, &Cube::Rprime, &Cube::R2,
        &Cube::U, &Cube::Uprime, &Cube::U2,
        &Cube::D, &Cube::Dprime, &Cube::D2,
        &Cube::F, &Cube::Fprime, &Cube::F2,
        &Cube::B, &Cube::Bprime, &Cube::B2,
        &Cube::M, &Cube::Mprime, &Cube::M2
    };
    for (MoveFunction move : moves) {
        Cube cube = makeCube(solvedColors());
        (cube.*move)();
        check(cube, "one legal move");
    }

    // Whole-cube x rotation: all three layers turn together.
    Cube rotated = makeCube(solvedColors());
    rotated.R(); rotated.Mprime(); rotated.Lprime();
    check(rotated, "whole-cube rotation");

    std::mt19937 random(12345); // Fixed seed makes failures reproducible.
    for (int scheme = 0; scheme < 2; ++scheme) {
        Cube scrambled = makeCube(scheme == 0 ? solvedColors() : colors);
        for (int step = 0; step < 2000; ++step) {
            MoveFunction move = moves[random() % 21];
            (scrambled.*move)();
            check(scrambled, "legal scramble prefix");
        }
    }

    // An illegal flipped edge remains illegal even after a legal scramble.
    colors = solvedColors();
    std::swap(colors[U][2][1], colors[F][0][1]);
    Cube impossible = makeCube(colors);
    for (int step = 0; step < 100; ++step) {
        MoveFunction move = moves[random() % 21];
        (impossible.*move)();
        check(impossible, "scrambled flipped edge", "odd number of edges");
    }

    std::cout << "Validation tests passed: impossible states, all moves, "
                 "4000 legal scramble prefixes, and unchanged inputs.\n";
}
