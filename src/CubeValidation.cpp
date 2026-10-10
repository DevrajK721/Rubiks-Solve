#include "rubiks/CubeValidation.hpp"

namespace rubiks {
namespace {
    enum FacePosition { Up, Down, Left, Right, Front, Back };

    struct Sticker {
        int face;
        int row;
        int col;
    };

    // The two stickers belonging to each edge, using the README's face net.
    const Sticker edges[12][2] = {
        {{Up, 1, 2}, {Right, 0, 1}}, // UR
        {{Up, 2, 1}, {Front, 0, 1}}, // UF
        {{Up, 1, 0}, {Left, 0, 1}},  // UL
        {{Up, 0, 1}, {Back, 0, 1}},  // UB
        {{Down, 1, 2}, {Right, 2, 1}}, // DR
        {{Down, 0, 1}, {Front, 2, 1}}, // DF
        {{Down, 1, 0}, {Left, 2, 1}},  // DL
        {{Down, 2, 1}, {Back, 2, 1}},  // DB
        {{Front, 1, 2}, {Right, 1, 0}}, // FR
        {{Front, 1, 0}, {Left, 1, 2}},  // FL
        {{Back, 1, 2}, {Left, 1, 0}},   // BL
        {{Back, 1, 0}, {Right, 1, 2}}   // BR
    };

    // The three stickers belonging to each corner, in a consistent circular
    // order. Turning a corner cycles this order; it cannot mirror the order.
    const Sticker corners[8][3] = {
        {{Up, 2, 2}, {Right, 0, 0}, {Front, 0, 2}}, // URF
        {{Up, 2, 0}, {Front, 0, 0}, {Left, 0, 2}},  // UFL
        {{Up, 0, 0}, {Left, 0, 0}, {Back, 0, 2}},   // ULB
        {{Up, 0, 2}, {Back, 0, 0}, {Right, 0, 2}},  // UBR
        {{Down, 0, 2}, {Front, 2, 2}, {Right, 2, 0}}, // DFR
        {{Down, 0, 0}, {Left, 2, 2}, {Front, 2, 0}},  // DLF
        {{Down, 2, 0}, {Back, 2, 2}, {Left, 2, 0}},   // DBL
        {{Down, 2, 2}, {Right, 2, 2}, {Back, 2, 0}}   // DRB
    };

    // Counting out-of-order pairs tells us whether a rearrangement needs an
    // odd or even number of swaps. Legal turns give edges and corners the
    // same result, once colours are interpreted relative to the centres.
    int permutationParity(const int order[], int size) {
        int pairs = 0;
        for (int i = 0; i < size; ++i) {
            for (int j = i + 1; j < size; ++j) {
                if (order[i] > order[j]) {
                    ++pairs;
                }
            }
        }
        return pairs % 2;
    }
}

std::string validateCube(const Cube& cube) {
    const std::array<FaceColorArray, 6> faces = {{
        cube.getUpFace().getColors(), cube.getDownFace().getColors(),
        cube.getLeftFace().getColors(), cube.getRightFace().getColors(),
        cube.getFrontFace().getColors(), cube.getBackFace().getColors()
    }};

    // 1. Check colour values before using them as array indices.
    // Color is declared in the order R, O, Y, G, B, W (values 0 through 5).
    int colourCounts[6] = {};
    for (const auto& face : faces) {
        for (const auto& row : face) {
            for (Color colour : row) {
                int value = static_cast<int>(colour);
                if (value < 0 || value >= 6) {
                    return "A sticker has an unknown colour.";
                }
                ++colourCounts[value];
            }
        }
    }
    for (int count : colourCounts) {
        if (count != 9) {
            return "Each colour must appear exactly nine times.";
        }
    }

    // 2. Each centre must be different. Their arrangement defines the solved
    // target, so no particular colour has to be on top. This also handles M.
    Color centres[6];
    bool centreSeen[6] = {};
    for (int face = 0; face < 6; ++face) {
        centres[face] = faces[face][1][1];
        int colour = static_cast<int>(centres[face]);
        if (centreSeen[colour]) {
            return "The six centre colours must all be different.";
        }
        centreSeen[colour] = true;
    }

    // 3. Identify each edge by its two colours, checking both orientations.
    int edgeOrder[12] = {};
    bool edgeSeen[12] = {};
    int edgeFlips = 0;
    for (int position = 0; position < 12; ++position) {
        const Sticker& first = edges[position][0];
        const Sticker& second = edges[position][1];
        Color a = faces[first.face][first.row][first.col];
        Color b = faces[second.face][second.row][second.col];
        int match = -1;
        int flip = 0;

        for (int piece = 0; piece < 12; ++piece) {
            Color expectedA = centres[edges[piece][0].face];
            Color expectedB = centres[edges[piece][1].face];
            if (a == expectedA && b == expectedB) {
                match = piece;
                break;
            }
            if (a == expectedB && b == expectedA) {
                match = piece;
                flip = 1;
                break;
            }
        }
        if (match == -1) {
            return "An edge has an impossible colour pair for these centres.";
        }
        if (edgeSeen[match]) {
            return "An edge piece appears more than once; another is missing.";
        }
        edgeSeen[match] = true;
        edgeOrder[position] = match;
        edgeFlips += flip;
    }

    // 4. Identify each corner. Only its three cyclic orientations are allowed;
    // sorting the colours here would incorrectly accept mirrored corners.
    int cornerOrder[8] = {};
    bool cornerSeen[8] = {};
    int cornerTwists = 0;
    for (int position = 0; position < 8; ++position) {
        Color actual[3];
        for (int sticker = 0; sticker < 3; ++sticker) {
            const Sticker& location = corners[position][sticker];
            actual[sticker] = faces[location.face][location.row][location.col];
        }
        int match = -1;
        int twist = 0;
        for (int piece = 0; piece < 8 && match == -1; ++piece) {
            for (int rotation = 0; rotation < 3; ++rotation) {
                bool matches = true;
                for (int sticker = 0; sticker < 3; ++sticker) {
                    Color expected = centres[corners[piece][sticker].face];
                    if (actual[(sticker + rotation) % 3] != expected) {
                        matches = false;
                    }
                }
                if (matches) {
                    match = piece;
                    twist = rotation;
                    break;
                }
            }
        }
        if (match == -1) {
            return "A corner has impossible colours or a mirrored sticker order.";
        }
        if (cornerSeen[match]) {
            return "A corner piece appears more than once; another is missing.";
        }
        cornerSeen[match] = true;
        cornerOrder[position] = match;
        cornerTwists += twist;
    }

    // 5. These three rules distinguish reachable states from arrangements
    // that would require taking the cube apart or moving individual stickers.
    if (edgeFlips % 2 != 0) {
        return "An odd number of edges are flipped.";
    }
    if (cornerTwists % 3 != 0) {
        return "The corner twists do not balance (their total must be divisible by three).";
    }
    if (permutationParity(edgeOrder, 12) != permutationParity(cornerOrder, 8)) {
        return "The edge and corner swaps do not match (permutation parity).";
    }

    return "";
}
}
