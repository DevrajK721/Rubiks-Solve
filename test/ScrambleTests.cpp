#include "rubiks/Cube.hpp"
#include "rubiks/CubeValidation.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

using rubiks::Color;
using rubiks::Cube;
using rubiks::Face;
using rubiks::FaceColorArray;
using CubeColors = std::array<FaceColorArray, 6>;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "Scramble test failed: " << description << '\n';
        std::exit(1);
    }
}

Cube makeSolvedCube() {
    const Color centres[] = {Color::W, Color::Y, Color::O,
                             Color::R, Color::G, Color::B};
    CubeColors colors;
    for (int face = 0; face < 6; ++face) {
        for (auto& row : colors[face]) row.fill(centres[face]);
    }
    return Cube{Face(colors[0]), Face(colors[1]), Face(colors[2]),
                Face(colors[3]), Face(colors[4]), Face(colors[5])};
}

CubeColors readCube(const Cube& cube) {
    return {{cube.getUpFace().getColors(), cube.getDownFace().getColors(),
             cube.getLeftFace().getColors(), cube.getRightFace().getColors(),
             cube.getFrontFace().getColors(), cube.getBackFace().getColors()}};
}

struct Move {
    const char* notation;
    void (Cube::*apply)();
    void (Cube::*undo)();
};

// Replay the returned text through the named move functions, independently
// of the scrambler's numeric choices and repeated-quarter-turn implementation.
const Move moves[] = {
    {"L", &Cube::L, &Cube::Lprime}, {"L'", &Cube::Lprime, &Cube::L}, {"L2", &Cube::L2, &Cube::L2},
    {"R", &Cube::R, &Cube::Rprime}, {"R'", &Cube::Rprime, &Cube::R}, {"R2", &Cube::R2, &Cube::R2},
    {"U", &Cube::U, &Cube::Uprime}, {"U'", &Cube::Uprime, &Cube::U}, {"U2", &Cube::U2, &Cube::U2},
    {"D", &Cube::D, &Cube::Dprime}, {"D'", &Cube::Dprime, &Cube::D}, {"D2", &Cube::D2, &Cube::D2},
    {"F", &Cube::F, &Cube::Fprime}, {"F'", &Cube::Fprime, &Cube::F}, {"F2", &Cube::F2, &Cube::F2},
    {"B", &Cube::B, &Cube::Bprime}, {"B'", &Cube::Bprime, &Cube::B}, {"B2", &Cube::B2, &Cube::B2}
};

std::vector<int> parse(const std::string& sequence) {
    std::istringstream input(sequence);
    std::string token;
    std::vector<int> result;
    char previousFace = '\0';
    while (input >> token) {
        int match = -1;
        for (int i = 0; i < 18; ++i) {
            if (token == moves[i].notation) match = i;
        }
        check(match != -1, "only the 18 outer-face moves may appear");
        check(token[0] != previousFace, "consecutive faces must differ");
        previousFace = token[0];
        result.push_back(match);
    }
    return result;
}

void checkReplay(const Cube& original, const Cube& scrambled,
                 const std::string& sequence, int count) {
    std::vector<int> indices = parse(sequence);
    check(static_cast<int>(indices.size()) == count, "requested move count");

    Cube replayed = original;
    for (int index : indices) {
        (replayed.*moves[index].apply)();
    }
    check(readCube(replayed) == readCube(scrambled), "returned notation reproduces the state");

    Cube restored = scrambled;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        (restored.*moves[*it].undo)();
    }
    check(readCube(restored) == readCube(original), "inverse sequence restores the starting state");
    check(rubiks::validateCube(scrambled).empty(), "scrambled cube remains physically possible");

    CubeColors before = readCube(original);
    CubeColors after = readCube(scrambled);
    for (int face = 0; face < 6; ++face) {
        check(before[face][1][1] == after[face][1][1], "outer-face scramble preserves centres");
    }
}

int main() {
    for (int count : {0, 1, 25, 1000}) {
        Cube original = makeSolvedCube();
        Cube scrambled = original;
        std::string sequence = scrambled.scramble(count, 12345);
        checkReplay(original, scrambled, sequence, count);

        Cube repeated = original;
        check(repeated.scramble(count, 12345) == sequence, "same seed gives the same sequence");
        check(readCube(repeated) == readCube(scrambled), "same seed gives the same state");
    }

    Cube original = makeSolvedCube();
    Cube defaultScramble = original;
    std::string sequence = defaultScramble.scramble();
    checkReplay(original, defaultScramble, sequence, 25);

    // Scramble the current state rather than silently replacing it with solved.
    original.L(); original.M(); original.rotateWholeRight();
    Cube scrambled = original;
    sequence = scrambled.scramble(40, 9876);
    checkReplay(original, scrambled, sequence, 40);
    Cube nextStart = scrambled;
    sequence = scrambled.scramble(15, 42);
    checkReplay(nextStart, scrambled, sequence, 15);

    Cube unchanged = original;
    check(unchanged.scramble(0, 42).empty(), "zero moves returns an empty sequence");
    check(readCube(unchanged) == readCube(original), "zero moves preserves state");
    bool rejected = false;
    try {
        unchanged.scramble(-1, 42);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, "negative move count is rejected");
    check(readCube(unchanged) == readCube(original), "invalid count preserves state");

    Cube shortCube = makeSolvedCube();
    Cube longCube = makeSolvedCube();
    std::string shortSequence = shortCube.scramble(10, 12345);
    std::string longSequence = longCube.scramble(25, 12345);
    check(longSequence.substr(0, shortSequence.size()) == shortSequence,
          "seeded sequences share a reproducible prefix");

    std::cout << "Scramble tests passed: counts, notation, replay, inverses, seeds, and validity.\n";
}
