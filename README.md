# Rubiks-Solve
A Rubik's cube solver implemented in C++.

## Face orientation

The constructor takes faces in this order: **up, down, left, right, front, back**.
Each face stores the colours supplied by the caller, with rows and columns
numbered from 0 to 2. Position `[0][0]` is the top-left sticker when looking
directly at that face from outside the cube.

Use this unfolded net to orient the grids:

```text
          U
      L   F   R   B
          D
```

- The top edges of F, L, R, and B meet U.
- The top edge of U meets B; its bottom edge meets F.
- The top edge of D meets F; its bottom edge meets B.
- The right edge of B meets L. This is why some back-face strips must be
  reversed when moved to another face.

Face names describe fixed positions, independently of their centre colours.

## Moves

`L()`, `R()`, `U()`, `D()`, `F()`, and `B()` turn that face clockwise when
viewed directly from outside. Each move also transfers the adjacent strips.
`M()` turns the middle slice between L and R in the same direction as L:
U's middle column moves to F, then D, then B, then U. It also moves the
centres on those four faces, while leaving the L and R faces unchanged.
This follows [standard slice notation](https://www.jperm.net/3x3/moves).

The current function names use `prime` for an inverse and `2` for a half turn:

```cpp
cube.L();
cube.Uprime();
cube.R2();
```

The implementations are in `src/Moves.cpp`. A quarter turn prepares copies
of the four adjoining grids, changes only the affected strips, and writes
them back after all original colours have been read. `2 - i` reverses the
order of a three-sticker strip. Prime moves repeat a quarter turn three
times, and half turns repeat it twice.

## Validating a cube

Include `rubiks/CubeValidation.hpp` and call:

```cpp
std::string error = rubiks::validateCube(cube);
bool possible = error.empty();
```

An empty string means the state is possible. Otherwise, the string explains
the first problem found, for example `An odd number of edges are flipped.`
The function does not change the cube. Validation is an explicit check;
constructing a `Cube` does not automatically validate it.

The implementation in `src/CubeValidation.cpp` checks:

1. Every sticker has a known colour, with exactly nine of each colour.
2. The six centres have different colours.
3. All twelve edges and eight corners occur exactly once, with possible
   colour combinations. A mirrored corner is rejected too.
4. Edge flips balance: their total is even.
5. Corner twists balance: their total is divisible by three.
6. Edge and corner permutations have the same parity. For example, swapping
   just two edges is impossible, but swapping two edges and two corners is
   possible.

The centre colours define the solved target. No fixed colour scheme is
imposed: white does not have to be on top, and the validator does not enforce
a manufacturer's opposite-colour pairs. A consistent alternative scheme is
accepted. Interpreting pieces relative to the current centres also lets
validation work after `M` moves and whole-cube rotations.

The piece-orientation and parity rules are described in Herbert Kociemba's
[cubie explanation](https://www.kociemba.org/math/cubielevel.htm) and
[coordinate explanation](https://www.kociemba.org/math/coordlevel.htm).

## Building and testing

From the project directory, configure once with GNU GCC:

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_BUILD_TYPE=Debug
```

Use the name of your installed GNU compiler if it differs. To build and run:

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```

The move tests track each of the 54 sticker positions through all 21 moves
and compare against independently derived expected destinations. They also
check inverse moves, repeated turns, and undoing a mixed sequence. The marker
fixtures intentionally isolate individual stickers; they do not represent
physically solvable cubes.

The validation tests cover incorrect counts and centres, missing/repeated or
mirrored pieces, single edge flips, corner twists, mismatched permutations,
and valid combinations of flips, twists, and swaps. They also check every
supported move and 4,000 legal scramble prefixes, and verify that validation
leaves its input unchanged.
