#include "rubiks/Cube.hpp"
#include <array>
#include <cstdlib>
#include <iostream>

using rubiks::Color;
using rubiks::Cube;
using rubiks::Face;
using rubiks::FaceColorArray;

using CubeColors = std::array<FaceColorArray, 6>;
// A reference to a Cube move, so the same checks can run for every move.
using MoveFunction = void (Cube::*)();

CubeColors readCube(const Cube& cube) {
    return {{cube.getUpFace().getColors(), cube.getDownFace().getColors(),
             cube.getLeftFace().getColors(), cube.getRightFace().getColors(),
             cube.getFrontFace().getColors(), cube.getBackFace().getColors()}};
}

Cube makeCube(const CubeColors& colors) {
    return Cube{Face(colors[0]), Face(colors[1]), Face(colors[2]),
                Face(colors[3]), Face(colors[4]), Face(colors[5])};
}

// Put one red sticker among white stickers to track its destination precisely.
// This is a storage test fixture, not a physically solvable cube.
CubeColors markerAt(int position) {
    CubeColors colors;
    for (auto& face : colors) {
        for (auto& row : face) {
            row.fill(Color::W);
        }
    }
    colors[position / 9][(position % 9) / 3][position % 3] = Color::R;
    return colors;
}

void expectCube(const Cube& cube, const CubeColors& expected,
                const char* move, const char* check, int source) {
    if (readCube(cube) != expected) {
        std::cerr << move << ": " << check
                  << " failed for starting sticker " << source << '\n';
        std::exit(1);
    }
}

struct MoveCase {
    const char* name;
    MoveFunction quarter;
    MoveFunction prime;
    MoveFunction half;
    std::array<int, 54> destination;
};

// Number stickers by face, then row, then column:
// U: 0..8, D: 9..17, L: 18..26, R: 27..35, F: 36..44, B: 45..53.
// Each number below is where that source sticker must go after one turn.
// These expected permutations were derived independently using 3D rotations
// of sticker positions and normals, not the strip assignments in Moves.cpp.
const std::array<MoveCase, 7> moves = {{
    {"L", &Cube::L, &Cube::Lprime, &Cube::L2, {{
        36,  1,  2, 39,  4,  5, 42,  7,  8, // U
        53, 10, 11, 50, 13, 14, 47, 16, 17, // D
        20, 23, 26, 19, 22, 25, 18, 21, 24, // L
        27, 28, 29, 30, 31, 32, 33, 34, 35, // R
         9, 37, 38, 12, 40, 41, 15, 43, 44, // F
        45, 46,  6, 48, 49,  3, 51, 52,  0  // B
    }}},
    {"R", &Cube::R, &Cube::Rprime, &Cube::R2, {{
         0,  1, 51,  3,  4, 48,  6,  7, 45, // U
         9, 10, 38, 12, 13, 41, 15, 16, 44, // D
        18, 19, 20, 21, 22, 23, 24, 25, 26, // L
        29, 32, 35, 28, 31, 34, 27, 30, 33, // R
        36, 37,  2, 39, 40,  5, 42, 43,  8, // F
        17, 46, 47, 14, 49, 50, 11, 52, 53  // B
    }}},
    {"U", &Cube::U, &Cube::Uprime, &Cube::U2, {{
         2,  5,  8,  1,  4,  7,  0,  3,  6, // U
         9, 10, 11, 12, 13, 14, 15, 16, 17, // D
        45, 46, 47, 21, 22, 23, 24, 25, 26, // L
        36, 37, 38, 30, 31, 32, 33, 34, 35, // R
        18, 19, 20, 39, 40, 41, 42, 43, 44, // F
        27, 28, 29, 48, 49, 50, 51, 52, 53  // B
    }}},
    {"D", &Cube::D, &Cube::Dprime, &Cube::D2, {{
         0,  1,  2,  3,  4,  5,  6,  7,  8, // U
        11, 14, 17, 10, 13, 16,  9, 12, 15, // D
        18, 19, 20, 21, 22, 23, 42, 43, 44, // L
        27, 28, 29, 30, 31, 32, 51, 52, 53, // R
        36, 37, 38, 39, 40, 41, 33, 34, 35, // F
        45, 46, 47, 48, 49, 50, 24, 25, 26  // B
    }}},
    {"F", &Cube::F, &Cube::Fprime, &Cube::F2, {{
         0,  1,  2,  3,  4,  5, 27, 30, 33, // U
        20, 23, 26, 12, 13, 14, 15, 16, 17, // D
        18, 19,  8, 21, 22,  7, 24, 25,  6, // L
        11, 28, 29, 10, 31, 32,  9, 34, 35, // R
        38, 41, 44, 37, 40, 43, 36, 39, 42, // F
        45, 46, 47, 48, 49, 50, 51, 52, 53  // B
    }}},
    {"B", &Cube::B, &Cube::Bprime, &Cube::B2, {{
        24, 21, 18,  3,  4,  5,  6,  7,  8, // U
         9, 10, 11, 12, 13, 14, 35, 32, 29, // D
        15, 19, 20, 16, 22, 23, 17, 25, 26, // L
        27, 28,  0, 30, 31,  1, 33, 34,  2, // R
        36, 37, 38, 39, 40, 41, 42, 43, 44, // F
        47, 50, 53, 46, 49, 52, 45, 48, 51  // B
    }}},
    {"M", &Cube::M, &Cube::Mprime, &Cube::M2, {{
         0, 37,  2,  3, 40,  5,  6, 43,  8, // U
         9, 52, 11, 12, 49, 14, 15, 46, 17, // D
        18, 19, 20, 21, 22, 23, 24, 25, 26, // L
        27, 28, 29, 30, 31, 32, 33, 34, 35, // R
        36, 10, 38, 39, 13, 41, 42, 16, 44, // F
        45,  7, 47, 48,  4, 50, 51,  1, 53  // B
    }}},
}};

int main() {
    // Checking all 54 possible marker positions verifies every moved AND
    // unchanged sticker, including centres and reversed back-face strips.
    for (const auto& move : moves) {
        for (int source = 0; source < 54; ++source) {
            CubeColors original = markerAt(source);
            int once = move.destination[source];
            int twice = move.destination[once];
            int thrice = move.destination[twice];

            Cube cube = makeCube(original);
            (cube.*move.quarter)();
            expectCube(cube, markerAt(once), move.name, "quarter turn", source);
            (cube.*move.prime)();
            expectCube(cube, original, move.name, "turn then inverse", source);

            cube = makeCube(original);
            (cube.*move.prime)();
            expectCube(cube, markerAt(thrice), move.name, "prime turn", source);
            (cube.*move.quarter)();
            expectCube(cube, original, move.name, "inverse then turn", source);

            cube = makeCube(original);
            (cube.*move.half)();
            expectCube(cube, markerAt(twice), move.name, "half turn", source);
            (cube.*move.half)();
            expectCube(cube, original, move.name, "two half turns", source);

            cube = makeCube(original);
            for (int count = 0; count < 4; ++count) {
                (cube.*move.quarter)();
            }
            expectCube(cube, original, move.name, "four quarter turns", source);
        }
    }

    // A mixed sequence must be undone by inverse moves in reverse order.
    for (int source = 0; source < 54; ++source) {
        CubeColors original = markerAt(source);
        Cube cube = makeCube(original);
        cube.L(); cube.Rprime(); cube.U2(); cube.D();
        cube.Fprime(); cube.B2(); cube.M();
        cube.Mprime(); cube.B2(); cube.F(); cube.Dprime();
        cube.U2(); cube.R(); cube.Lprime();
        expectCube(cube, original, "sequence", "reverse sequence", source);
    }
    std::cout << "Move tests passed: all 21 moves, every sticker position, "
                 "inverses, and a mixed sequence.\n";
}
