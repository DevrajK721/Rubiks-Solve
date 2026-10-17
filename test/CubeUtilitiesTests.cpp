#include "rubiks/Cube.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

using rubiks::Color;
using rubiks::Cube;
using rubiks::Face;
using rubiks::FaceColorArray;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "Cube utility test failed: " << description << '\n';
        std::exit(1);
    }
}

Face solidFace(Color colour) {
    FaceColorArray colors;
    for (auto& row : colors) row.fill(colour);
    return Face(colors);
}

Cube solvedCube() {
    return Cube{solidFace(Color::W), solidFace(Color::Y), solidFace(Color::O),
                solidFace(Color::R), solidFace(Color::G), solidFace(Color::B)};
}

int main() {
    Cube original = solvedCube();
    const std::string solvedKey = "WWWWWWWWWYYYYYYYYYOOOOOOOOORRRRRRRRRGGGGGGGGGBBBBBBBBB";
    check(original.stateKey() == solvedKey, "key has the documented face order");
    check(original.isSolved(), "solved cube is recognised");

    Cube branch = original.copy();
    branch.L();
    const std::string branchKey = branch.stateKey();
    check(!branch.isSolved(), "single face turn is unsolved");
    check(original.stateKey() == solvedKey, "changing a copy preserves the original");
    original.R();
    check(branch.stateKey() == branchKey, "changing the original preserves its copy");
    branch.Lprime();
    check(branch.isSolved(), "an inverse restores solvedness");
    check(branch.stateKey() == solvedKey, "inverse restores the exact key");

    // Keys preserve physical positions instead of merging rotated states.
    original = solvedCube();
    original.rotateWholeRight();
    check(original.isSolved(), "a reoriented solved cube is still solved");
    check(original.stateKey() != solvedKey, "keys distinguish whole-cube orientations");
    original.rotateWholeLeft();
    check(original.stateKey() == solvedKey, "reverse orientation restores the key");

    using Move = void (Cube::*)();
    const Move moves[] = {
        &Cube::L, &Cube::Lprime, &Cube::L2,
        &Cube::R, &Cube::Rprime, &Cube::R2,
        &Cube::U, &Cube::Uprime, &Cube::U2,
        &Cube::D, &Cube::Dprime, &Cube::D2,
        &Cube::F, &Cube::Fprime, &Cube::F2,
        &Cube::B, &Cube::Bprime, &Cube::B2,
        &Cube::M, &Cube::Mprime, &Cube::M2,
        &Cube::E, &Cube::Eprime, &Cube::E2,
        &Cube::S, &Cube::Sprime, &Cube::S2
    };
    for (Move move : moves) {
        Cube cube = solvedCube();
        (cube.*move)();
        check(!cube.isSolved(), "every supported single move breaks solvedness");
        check(cube.stateKey() != solvedKey, "every supported move changes the key");
        for (int repeat = 0; repeat < 3; ++repeat) (cube.*move)();
        check(cube.isSolved(), "four repeated moves restore solvedness");
        check(cube.stateKey() == solvedKey, "four repeated moves restore all stickers");
    }

    Cube first = solvedCube();
    first.scramble(50, 12345);
    Cube second = first.copy();
    check(first.stateKey() == second.stateKey(), "copy preserves a scrambled state");
    check(first.scramble(25, 99) == second.scramble(25, 99), "copied states accept matching experiments");
    check(first.stateKey() == second.stateKey(), "matching experiments have matching keys");

    // Every position has its own character in the immutable snapshot.
    for (int sticker = 0; sticker < 54; ++sticker) {
        std::array<FaceColorArray, 6> faces;
        for (auto& face : faces) for (auto& row : face) row.fill(Color::W);
        faces[sticker / 9][sticker % 9 / 3][sticker % 3] = Color::R;
        Cube marker{Face(faces[0]), Face(faces[1]), Face(faces[2]),
                    Face(faces[3]), Face(faces[4]), Face(faces[5])};
        std::string expected(54, 'W');
        expected[sticker] = 'R';
        check(marker.stateKey() == expected, "each sticker occupies its specified key position");
    }

    Cube invalid{solidFace(Color::R), solidFace(Color::R), solidFace(Color::R),
                 solidFace(Color::R), solidFace(Color::R), solidFace(Color::R)};
    check(!invalid.isSolved(), "uniform but impossible cube is not solved");
    check(invalid.stateKey() == std::string(54, 'R'), "keys can describe invalid known-colour states");

    Cube unknown{solidFace(static_cast<Color>(99)), solidFace(Color::Y), solidFace(Color::O),
                 solidFace(Color::R), solidFace(Color::G), solidFace(Color::B)};
    check(!unknown.isSolved(), "unknown sticker colours are not solved");
    bool rejected = false;
    try {
        unknown.stateKey();
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, "a key cannot silently encode an unknown colour");

    std::cout << "Cube utility tests passed: independent copies, solvedness, and state keys.\n";
}
