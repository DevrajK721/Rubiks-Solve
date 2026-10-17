#include "rubiks/Cube.hpp"
#include <random>
#include <stdexcept>

namespace rubiks {
    std::string Cube::scramble(int moveCount) {
        return scramble(moveCount, std::random_device{}());
    }

    std::string Cube::scramble(int moveCount, unsigned int seed) {
        if (moveCount < 0) {
            throw std::invalid_argument("Scramble move count cannot be negative.");
        }

        std::mt19937 random(seed);
        std::uniform_int_distribution<int> chooseFace(0, 5);
        std::uniform_int_distribution<int> chooseTurns(1, 3);
        const char faceNames[] = {'L', 'R', 'U', 'D', 'F', 'B'};

        std::string sequence;
        int previousFace = -1;

        for (int move = 0; move < moveCount; ++move) {
            // Adjacent turns of the same face would just combine or cancel.
            int face;
            do {
                face = chooseFace(random);
            } while (face == previousFace);

            int turns = chooseTurns(random);

            // Reuse the existing quarter turns. Two turns make a half turn;
            // three make a prime turn, exactly as in the movement functions.
            for (int turn = 0; turn < turns; ++turn) {
                switch (face) {
                    case 0: L(); break;
                    case 1: R(); break;
                    case 2: U(); break;
                    case 3: D(); break;
                    case 4: F(); break;
                    case 5: B(); break;
                }
            }

            // Record what happened so an experiment can be reproduced.
            if (!sequence.empty()) sequence += ' ';
            sequence += faceNames[face];
            if (turns == 2) sequence += '2';
            if (turns == 3) sequence += '\'';

            previousFace = face;
        }

        return sequence;
    }
}
