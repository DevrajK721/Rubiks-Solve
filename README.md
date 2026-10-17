# Rubiks-Solve

A C++ Rubik's cube simulator for building solving and search experiments, with
an importable Python API and an optional interactive 3D viewer.

The same C++ implementation powers the viewer and Python. It supports custom
states, all outer-face and middle-slice moves, validation, reproducible
scrambling, independent copies, solved-state checks, and immutable state keys.

## Contents

- [Setup](#setup)
- [Functionality](#functionality)
- [API reference](#api-reference)
- [API usage in depth](#api-usage-in-depth)
- [Running tests](#running-tests)
- [Troubleshooting](#troubleshooting)
- [Project layout](#project-layout)

## Setup

Run the commands below from the repository root unless another directory is
specified. The examples use a POSIX shell, as on macOS or Linux.

### Requirements

| Component | Purpose |
| --- | --- |
| C++17 compiler | Compiles the cube mechanics and bindings |
| CMake 3.18 or newer | Configures the build and tests |
| Python with development headers | Builds and runs the Python extension |
| pybind11 | Exposes the C++ types to Python; pinned in `requirements.txt` |
| raylib | Required only when building the 3D viewer and its tests |

The full setup has been verified on Apple Silicon with GNU GCC 15.2.0
(`g++-15`), CMake 4.0.3, Python 3.14.2, pybind11 3.1.0, and the macOS 26.5 SDK.
Using a different Python version requires rebuilding the extension for that
interpreter.

### Install dependencies on macOS

With [Homebrew](https://brew.sh/) installed:

```sh
brew install cmake gcc python raylib
```

Check your compiler name with `g++-15 --version`; Homebrew may install a
newer GCC version, in which case substitute that compiler's executable below.
On macOS, plain `g++` may invoke Apple Clang rather than GNU GCC.

Create a local Python environment:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

The environment and generated files are ignored by Git. Activate the environment
again in new terminals, or use `.venv/bin/python` explicitly as these examples do.

### Configure and build on the tested Mac setup

The following command explicitly selects the working compiler, SDK, and Python
environment. This avoids CMake reverting to an incompatible default linker/SDK
combination on this machine:

```sh
cmake -S . -B build \
    -DCMAKE_CXX_COMPILER=g++-15 \
    -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk \
    -DCMAKE_BUILD_TYPE=Debug \
    -DPython_EXECUTABLE="$PWD/.venv/bin/python" \
    -Dpybind11_DIR="$(.venv/bin/python -m pybind11 --cmakedir)"

cmake --build build
ctest --test-dir build --output-on-failure
```

The SDK path above is specific to the tested installation. To see the SDKs
installed on another Mac:

```sh
ls /Library/Developer/CommandLineTools/SDKs
```

Select a compatible installed SDK and compiler rather than copying an unavailable
path. The CMake file also works around raylib packages that embed an OpenGL
framework path from a different SDK.

Use `Debug` while developing and running tests: some existing C++ tests use
`assert`, which is normally disabled in `Release` builds.

### Configure on other systems

With a working default compiler and Python development environment, the portable
configuration is:

```sh
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DRUBIKS_BUILD_VIEWER=OFF \
    -DPython_EXECUTABLE="$PWD/.venv/bin/python" \
    -Dpybind11_DIR="$(.venv/bin/python -m pybind11 --cmakedir)"
cmake --build build
ctest --test-dir build --output-on-failure
```

This builds the cube and Python API without raylib. To include graphics, install
raylib for your platform and set `RUBIKS_BUILD_VIEWER=ON`. On Windows, adapt the
virtual-environment executable and shell syntax, and use `--config Debug` for
multi-configuration generators.

### Build options

| CMake option | Default | Effect |
| --- | --- | --- |
| `RUBIKS_BUILD_PYTHON` | `ON` | Build the `rubiks` Python extension and Python tests |
| `RUBIKS_BUILD_VIEWER` | `ON` | Find raylib and build the viewer and view tests |
| `BUILD_TESTING` | `ON` | Build and register tests with CTest |

For Python-only experiments on the tested Mac, use a separate build directory:

```sh
cmake -S . -B build-python \
    -DCMAKE_CXX_COMPILER=g++-15 \
    -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk \
    -DCMAKE_BUILD_TYPE=Debug \
    -DRUBIKS_BUILD_VIEWER=OFF \
    -DPython_EXECUTABLE="$PWD/.venv/bin/python" \
    -Dpybind11_DIR="$(.venv/bin/python -m pybind11 --cmakedir)"
cmake --build build-python
ctest --test-dir build-python --output-on-failure
```

For a C++-only build, Python, pybind11, and raylib are unnecessary:

```sh
cmake -S . -B build-core \
    -DCMAKE_CXX_COMPILER=g++-15 \
    -DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk \
    -DCMAKE_BUILD_TYPE=Debug \
    -DRUBIKS_BUILD_PYTHON=OFF \
    -DRUBIKS_BUILD_VIEWER=OFF
cmake --build build-core
ctest --test-dir build-core --output-on-failure
```

Omit the macOS-specific compiler/SDK options when using another platform's
working toolchain. For performance measurements, create a separate `Release`
build with `BUILD_TESTING=OFF` after checking correctness in `Debug`.

### Import the Python module

The extension is built into the selected build directory; it is not installed
into site-packages by the current workflow. From the repository root:

```sh
PYTHONPATH=build .venv/bin/python -c "import rubiks; print(rubiks.Color.R)"
```

Expected output:

```text
Color.R
```

Start an interactive session the same way:

```sh
PYTHONPATH=build .venv/bin/python
```

`PYTHONPATH` tells Python where to find the extension. Use `build-python` instead
of `build` if that is where you compiled it. From inside `build/`, the equivalent
command is `PYTHONPATH=. ../.venv/bin/python`.

After editing C++ code or bindings, rebuild before starting a new Python process:

```sh
cmake --build build --target rubiks
```

Restart notebook kernels or interactive sessions after rebuilding so they load
the new extension. For general binding details, see the official
[pybind11 build guide](https://pybind11.readthedocs.io/en/stable/compiling.html).

## Functionality

### Cube representation and face orientation

A `Face` stores a 3x3 grid. A `Cube` owns six independent faces, always in this
constructor and serialization order:

**up, down, left, right, front, back** (`U D L R F B`).

Rows and columns are zero-based. `[0][0]` is the top-left sticker when looking
directly at a face from outside the cube. `[1][1]` is its centre. Orient the grids
using this unfolded net:

```text
          U
      L   F   R   B
          D
```

- The top edges of F, L, R, and B meet U.
- The top edge of U meets B; its bottom edge meets F.
- The top edge of D meets F; its bottom edge meets B.
- The right edge of B meets L.

Face names describe positions, independently of their centre colours. A complete
face turn rotates its own grid and transfers the adjoining strips. A slice move
also transfers the affected centre stickers.

### Moves

Clockwise means looking directly at the turned face from outside the cube.
`prime` is the inverse, and `2` is a half turn. A half turn counts as one move in
the returned scramble sequence, despite being implemented with two quarter turns.

| Layer | Quarter turn | Inverse | Half turn |
| --- | --- | --- | --- |
| Left | `L()` | `Lprime()` | `L2()` |
| Right | `R()` | `Rprime()` | `R2()` |
| Up | `U()` | `Uprime()` | `U2()` |
| Down | `D()` | `Dprime()` | `D2()` |
| Front | `F()` | `Fprime()` | `F2()` |
| Back | `B()` | `Bprime()` | `B2()` |
| Middle between L/R, direction of L | `M()` | `Mprime()` | `M2()` |
| Middle between U/D, direction of D | `E()` | `Eprime()` | `E2()` |
| Middle between F/B, direction of F | `S()` | `Sprime()` | `S2()` |

These 27 methods are available in both C++ and Python and modify the cube in
place. Move functions return `None` in Python. Standard written notation uses
apostrophes (`L'`), while method names use `prime` (`cube.Lprime()`). The example
sequence helpers below translate between the two.

### Validation

Validation checks all of the following:

1. Known sticker colours, with nine stickers of each colour.
2. Six different centre colours.
3. Exactly one of each of the twelve edges and eight corners, with possible
   colour combinations and corner sticker order.
4. An even total number of edge flips.
5. A total corner twist divisible by three.
6. Matching edge and corner permutation parity.

The centre arrangement defines the solved colour scheme. The validator accepts
consistent alternative schemes; it does not enforce a manufacturer's opposite
colour pairs. It also works after legal slice moves and whole-cube rotations.
Construction is separate from validation, so explicitly validate external states.

See Herbert Kociemba's [cubie explanation](https://www.kociemba.org/math/cubielevel.htm)
and [coordinate explanation](https://www.kociemba.org/math/coordlevel.htm) for the
piece-orientation and parity rules.

### Scrambling

Scrambling applies random moves to the **current** state. It uses the six outer
faces and all three turn amounts, avoiding consecutive moves on the same face.
It returns the applied notation, such as `L U' R2`.

The default is 25 moves. A fixed seed repeats the sequence on the same build;
matching final states also require matching starting states and orientations.
The generator uses random move sequences, not a uniform sample of all cube
states. Save the initial state and returned sequence for reproducible experiments;
seed-to-sequence identity across different standard-library implementations is
not guaranteed.

### Interactive 3D viewer

With the viewer enabled:

```sh
./build/cube_viewer
```

| Input | Action |
| --- | --- |
| Hold arrow keys | Smooth whole-cube viewing rotation, aligned with the screen |
| Release arrows | Keep the current partial viewing angle |
| `L R U D F B M E S` | Apply the corresponding move once per press |
| Either Shift + a move key | Apply the inverse move |
| Press a move key twice | Apply its half turn |
| Space | Apply 25 random moves to the current state and display that sequence |
| Escape / close window | Exit |

Move keys follow the nearest of 24 quarter-turn orientations. When the chosen
orientation changes, the stored faces are reoriented and a drawing correction
preserves the continuous image. Whole-cube reorientation preserves solvedness,
but can change an orientation-sensitive state key. Individual layer moves update
immediately; smooth layer-turn animations are not included.

The viewer starts with a solved example configured in `src/main.cpp`. Its last
scramble label records that scramble only, not subsequent manual moves. The
Python API runs independently and does not open or control the viewer window.

## API reference

Import the compiled module with:

```python
from rubiks import Color, Face, Cube, validate_cube
```

### `Color`

| Value | Colour |
| --- | --- |
| `Color.R` | Red |
| `Color.O` | Orange |
| `Color.Y` | Yellow |
| `Color.G` | Green |
| `Color.B` | Blue |
| `Color.W` | White |

Pass these enum values when constructing faces; strings such as `"W"` are not
converted automatically by the `Face` constructor.

### `Face`

| Python API | Return | Meaning |
| --- | --- | --- |
| `Face(colors)` | `Face` | Construct from three rows of three `Color` values |
| `get_color(row, col)` | `Color` | Read one sticker; indices must be 0, 1, or 2 |
| `get_colors()` | Nested list | Copy all nine colours into Python data |
| `set_colors(colors)` | `None` | Replace the complete grid with a 3x3 input |

The constructor and setter accept compatible nested Python sequences, including
lists and tuples. They copy the values. Returned grids are also copies.

### `Cube`

The constructor is:

```text
Cube(up, down, left, right, front, back)
```

All six arguments must be `Face` objects. Positional and keyword arguments work.
There is no zero-argument constructor; `solved_cube()` in the examples is a Python
helper, not a built-in `Cube.solved()` method.

| Python API | Return | Meaning |
| --- | --- | --- |
| `get_up_face()` | `Face` | Copy of the upper face |
| `get_down_face()` | `Face` | Copy of the lower face |
| `get_left_face()` | `Face` | Copy of the left face |
| `get_right_face()` | `Face` | Copy of the right face |
| `get_front_face()` | `Face` | Copy of the front face |
| `get_back_face()` | `Face` | Copy of the back face |
| All 27 [move methods](#moves) | `None` | Modify the cube in place |
| `copy()` | `Cube` | Independent copy of all six faces |
| `is_solved()` | `bool` | True when the state is valid and all faces are uniform |
| `state_key()` | `str` | Immutable 54-letter snapshot, detailed below |
| `scramble(move_count=25)` | `str` | Apply a scramble using a fresh random seed |
| `scramble(move_count, seed)` | `str` | Apply a repeatable scramble |

The seeded overload requires the count as well, for example
`cube.scramble(25, seed=12345)`. `cube.scramble(seed=12345)` is not a supported call.
A count of zero returns `""` without moving anything. A negative count raises
`ValueError`. A seed must fit C++ `unsigned int` (0 through 4294967295 on the tested
platform); negative or oversized seeds are rejected.

`is_solved()` compares each sticker with its own face's centre and confirms the
state is valid. Thus a reoriented solved cube is solved, while an impossible
all-red cube is not. It does not modify the cube.

### `validate_cube(cube)`

Returns an empty string for a possible state, otherwise an explanation of the
first problem found. It leaves the cube unchanged. Typical errors include
incorrect colour counts, a flipped edge, a twisted corner, or unmatched swaps.

### State-key format

`state_key()` returns the full state rather than a hash digest:

| Characters | Face |
| --- | --- |
| `0:9` | Up |
| `9:18` | Down |
| `18:27` | Left |
| `27:36` | Right |
| `36:45` | Front |
| `45:54` | Back |

Each face is stored row by row: top-left through bottom-right. The alphabet is
`R O Y G B W`. For a face index `f`, row `r`, and column `c`, the character offset
is `9*f + 3*r + c`.

The solved example's key is the concatenation of nine Ws, nine Ys, nine Os, nine
Rs, nine Gs, and nine Bs. Keys compare exact sticker positions: rotated copies of
the same physical cube can have different keys. Rotation-equivalence
normalization is not provided.

Keys are snapshots and do not change when the cube later changes. Use them for
state equality, sets, dictionaries, and saved experiment records. The mutable
`Cube` class has no custom sticker-equality or state-hashing operators.
`state_key()` can encode impossible arrangements of known colours; call
`validate_cube()` when reachability matters. Unknown C++ enum values cannot be
encoded and raise `std::invalid_argument` (`ValueError` through Python).

### C++ names

The core uses the same move names as Python. Other names map as follows:

| C++ | Python |
| --- | --- |
| `Face::getColor(row, col)` | `face.get_color(row, col)` |
| `Face::getColors()` | `face.get_colors()` |
| `Face::setColors(colors)` | `face.set_colors(colors)` |
| `Cube::getUpFace()` and the other face getters | `cube.get_up_face()` etc. |
| `Cube::copy()` | `cube.copy()` |
| `Cube::isSolved()` | `cube.is_solved()` |
| `Cube::stateKey()` | `cube.state_key()` |
| `Cube::scramble(...)` | `cube.scramble(...)` |
| `rubiks::validateCube(cube)` | `validate_cube(cube)` |

C++ also provides `rotateWholeRight()`, `rotateWholeLeft()`, `rotateWholeUp()`,
and `rotateWholeDown()` for the viewer. These and `CubeView` are not exposed in
the Python module. `FaceColorArray` is a C++ alias for
`std::array<std::array<Color, 3>, 3>`.

## API usage in depth

These examples can be pasted into a Python session launched from the repository
root with `PYTHONPATH=build .venv/bin/python`. The small reusable helpers live in
`research/experiment_basics.py`; they are ordinary Python functions, separate from
the compiled API.

### Quick start: construct, scramble, branch

```python
from rubiks import Color, Face, Cube, validate_cube


def solid_face(color):
    return Face([[color] * 3 for _ in range(3)])


def solved_cube():
    return Cube(
        up=solid_face(Color.W), down=solid_face(Color.Y),
        left=solid_face(Color.O), right=solid_face(Color.R),
        front=solid_face(Color.G), back=solid_face(Color.B),
    )


cube = solved_cube()
assert cube.is_solved()
assert validate_cube(cube) == ""

start_key = cube.state_key()
trial = cube.copy()
sequence = trial.scramble(25, seed=12345)
assert cube.state_key() == start_key
assert validate_cube(trial) == ""

print("Scramble:", sequence)
print("Trial state:", trial.state_key())
```

A Python assignment such as `trial = cube` refers to the same mutable object.
Use `cube.copy()` for a genuinely independent branch.

### Read and replace face grids

```python
face = Face([
    [Color.R, Color.G, Color.B],
    [Color.Y, Color.W, Color.O],
    [Color.G, Color.R, Color.Y],
])
assert face.get_color(row=1, col=1) == Color.W

colors = face.get_colors()
colors[0][0] = Color.Y
assert face.get_color(0, 0) == Color.R  # Editing the list changes only the copy.
face.set_colors(colors)
assert face.get_color(0, 0) == Color.Y
```

A cube also copies the faces passed into its constructor. Calling
`cube.get_front_face().set_colors(...)` modifies a detached face, not the cube.
To construct a changed configuration, supply six faces to a new `Cube`.

### Reconstruct a cube from a configuration

This example captures an existing valid state's six grids. The grids could also
come from your own input code in the same face orientation and colour format:

```python
face_names = ("up", "down", "left", "right", "front", "back")
source = solved_cube()
source.scramble(10, seed=17)
configuration = {
    name: getattr(source, f"get_{name}_face")().get_colors()
    for name in face_names
}

restored = Cube(**{name: Face(grid) for name, grid in configuration.items()})
error = validate_cube(restored)
if error:
    raise ValueError(error)
assert restored.state_key() == source.state_key()
```

A `Face` constructor checks the shape/types needed for conversion, but only
whole-cube validation checks physical possibility. Counts of nine colours alone
are insufficient.

### Apply and undo moves

```python
cube = solved_cube()
before = cube.state_key()
cube.L()
cube.Uprime()
cube.R2()
assert not cube.is_solved()

# Reverse the order and invert each move; a half turn is its own inverse.
cube.R2()
cube.U()
cube.Lprime()
assert cube.state_key() == before
assert cube.is_solved()
```

The functions mutate the existing object. Keep the object and call `cube.L()`;
assigning `cube = cube.L()` would replace your variable with `None`.

For standard space-separated notation, use the supplied Python helpers:

```python
from research.experiment_basics import apply_sequence, undo_sequence

cube = solved_cube()
before = cube.state_key()
apply_sequence(cube, "L U' R2 M E' S2")
assert validate_cube(cube) == ""
undo_sequence(cube, "L U' R2 M E' S2")
assert cube.state_key() == before
```

They accept the nine layer letters with an optional apostrophe or `2` suffix,
check every token before applying anything, and translate to the C++ move
methods. Undoing a sequence assumes the same coordinate orientation and no
unrecorded intervening moves.

### Reproduce and undo a scramble

```python
first = solved_cube()
second = first.copy()
sequence = first.scramble(move_count=25, seed=12345)
assert second.scramble(25, seed=12345) == sequence
assert first.state_key() == second.state_key()

replayed = solved_cube()
apply_sequence(replayed, sequence)
assert replayed.state_key() == first.state_key()
undo_sequence(replayed, sequence)
assert replayed.is_solved()

record = {
    "seed": 12345,
    "initial_state": solved_cube().state_key(),
    "moves": sequence,
    "final_state": first.state_key(),
}
```

Seeds are convenient for repeated runs on one toolchain. Saving both the starting
state and actual move text gives you a reproducible record even when the random
number distribution implementation changes.

### Compare states and track visited positions

```python
start = solved_cube()
visited = {start.state_key()}
depth_by_state = {start.state_key(): 0}

branch = start.copy()
branch.R()
key = branch.state_key()
if key not in visited:
    visited.add(key)
    depth_by_state[key] = 1

assert start.is_solved()
assert not branch.is_solved()
assert start.state_key() != branch.state_key()
assert depth_by_state[branch.state_key()] == 1

snapshot = branch.state_key()
branch.Rprime()
assert branch.is_solved()
assert snapshot in visited  # Old snapshots stay unchanged after more moves.
```

Store the strings themselves. A visited key represents the state when it was
created, not a live reference to a mutable cube. Equality through keys includes
orientation and all 54 stickers.

### Search one move ahead

This small experiment uses copies, solvedness, and visited-state keys together:

```python
position = solved_cube()
position.R()
visited = {position.state_key()}
solution = None

moves = [face + suffix for face in "LRUDFB" for suffix in ("", "prime", "2")]
for move in moves:
    candidate = position.copy()
    getattr(candidate, move)()
    key = candidate.state_key()
    if key in visited:
        continue
    visited.add(key)
    if candidate.is_solved():
        solution = move
        break

assert solution == "Rprime"
assert not position.is_solved()  # Candidate exploration preserved the input.
print("One-move solution:", solution)
```

For an arbitrary target rather than a solved cube, compare
`candidate.state_key()` with the target's key. This is a one-step demonstration,
not a general solver; deeper search needs an explicit frontier and stopping rule.
For large experiments, keep the move set and starting-state distribution in your
experiment records and measure Python/C++ call overhead before optimizing.

### Save and restore a state snapshot

The supplied helper `cube_from_key()` parses the documented format, constructs
six faces, and validates the result:

```python
from pathlib import Path
from research.experiment_basics import cube_from_key

cube = solved_cube()
cube.scramble(25, seed=82)
Path("cube_state.txt").write_text(cube.state_key() + "\n", encoding="utf-8")

saved_key = Path("cube_state.txt").read_text(encoding="utf-8").strip()
loaded = cube_from_key(saved_key)
assert loaded.state_key() == cube.state_key()
```

This is an example Python loader, not an extra compiled constructor. It accepts
exactly 54 uppercase colour letters, rejects malformed or impossible states with
`ValueError`, and does not normalize orientation. Whitespace is stripped in the
file example before parsing.

### Handle invalid input

```python
face = solid_face(Color.W)
try:
    face.get_color(3, 0)
except IndexError as error:
    print("Bad position:", error)

cube = solved_cube()
try:
    cube.scramble(-1)
except ValueError as error:
    print("Bad scramble count:", error)

impossible = Cube(*(solid_face(Color.R) for _ in range(6)))
error = validate_cube(impossible)
assert error != ""
assert not impossible.is_solved()
```

| Situation | Result |
| --- | --- |
| Face index outside 0..2, including negative indices | `IndexError` |
| Wrong face grid dimensions or element types | `TypeError` |
| Missing constructor faces or unsupported method arguments | `TypeError` |
| Negative scramble count | `ValueError` |
| Malformed key passed to the example loader | `ValueError` |
| Physically impossible state passed to `validate_cube` | Nonempty error string |

Bindings translate the relevant C++ exceptions into Python exceptions; see the
[pybind11 exception reference](https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html).

### Run the complete experiment example

```sh
PYTHONPATH=build .venv/bin/python -m research.experiment_basics
```

It checks seeded scrambling, replay and undo, state restoration, independent
branches, and a one-move search. The example prints its scramble, state, and
solution, then exits successfully when all assertions pass.

## Running tests

### Run the complete suite

Build before running CTest, since CTest runs the existing executables/module:

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```

The default configuration registers these checks:

| CTest name | Coverage |
| --- | --- |
| `face_tests` | Face construction and sticker access |
| `cube_tests` | Construction from six supplied faces and face accessors |
| `move_tests` | Each sticker's destination for the original 21 L/R/U/D/F/B/M moves, inverses, and mixed sequences |
| `validation_tests` | Impossible pieces, flips, twists, parity, and legal scramble prefixes |
| `scramble_tests` | Counts, notation, replay, inverse replay, seeds, and physical validity |
| `cube_utilities_tests` | Independent copies, solvedness across all 27 moves, and exact state-key positions |
| `python_api_tests` | Python API contracts, all 27 moves, copied values, errors, scrambling, and the new helpers |
| `python_face_example` | `research/test_init.py` |
| `python_cube_example` | `research/test_cube.py` |
| `python_experiment_example` | `research/experiment_basics.py` |
| `view_tests` | View/control thresholds, continuous sticker positions, camera axes, and moves while viewing |

Python tests use the standard library's `unittest` module; pytest is not required.
CTest supplies the Python module search path automatically. The view tests use
raylib maths and do not open a window. Disabling the viewer omits `view_tests`;
disabling Python omits the Python checks.

### Run selected tests

```sh
ctest --test-dir build -R python --output-on-failure
ctest --test-dir build -R cube_utilities_tests --output-on-failure
ctest --test-dir build -V -R python_api_tests
```

Run the Python contract tests directly from the repository root:

```sh
PYTHONPATH=build .venv/bin/python test/PythonTests.py
```

The original smaller scripts remain usable:

```sh
PYTHONPATH=build .venv/bin/python research/test_init.py
PYTHONPATH=build .venv/bin/python research/test_cube.py
```

Successful assertion-only scripts can exit without output. The unittest suite
prints its individual test results. If you use another build directory, replace
`build` consistently in the build, CTest, and `PYTHONPATH` commands.

## Troubleshooting

### `ModuleNotFoundError: No module named 'rubiks'`

Build the extension and put its build directory on Python's search path:

```sh
cmake --build build --target rubiks
PYTHONPATH=build .venv/bin/python research/test_cube.py
```

Use the same interpreter that CMake selected. Check it with:

```sh
.venv/bin/python -c "import sys; print(sys.executable)"
```

A module compiled for a different Python version or architecture must be rebuilt.

### Python cannot find a new method

Check its exact Python spelling (`get_color`, `get_front_face`, `Lprime`,
`is_solved`, `state_key`). Rebuild `rubiks` and restart Python. To check which
module was imported:

```sh
PYTHONPATH=build .venv/bin/python -c "import rubiks; print(rubiks.__file__)"
```

Avoid naming your own script `rubiks.py`, which can shadow the extension.

### CMake cannot find pybind11 or selects the wrong Python

Re-run the full configure command with `Python_EXECUTABLE` and `pybind11_DIR`
pointing into the same `.venv`. Check the package location with:

```sh
.venv/bin/python -m pybind11 --cmakedir
```

Install `requirements.txt` into that interpreter's environment if it is missing.

### macOS `tapi error`, `malformed file`, or `unknown architecture`

This can result from a newer SDK being read by an older linker. The tested setup
selects `g++-15` and `MacOSX26.5.sdk` explicitly. CMake must receive the SDK option
at configuration time, before its compiler test. Changing compilers is best done
in a new build directory using the full setup command above.

Check the choices saved by CMake in `build/CMakeCache.txt`:

- `CMAKE_CXX_COMPILER`
- `CMAKE_OSX_SYSROOT`
- `Python_EXECUTABLE`

CMake's cache belongs to its build directory. Recreating that directory means
supplying those configuration settings again.

### Editing a returned face does not update the cube

Face getters return independent copies, and `get_colors()` returns copied Python
lists. Modify the cube with move methods, or reconstruct a new cube from six
updated faces. `cube.copy()` creates an independent cube for experiments.

## Project layout

```text
CMakeLists.txt              Build options, shared library, bindings, tests, viewer
requirements.txt           Python binding dependency
include/rubiks/             Public C++ types and declarations
src/Face.cpp                Face storage and access
src/Cube.cpp                Cube construction, copies, solvedness, state keys
src/Moves.cpp               Face/slice moves and whole-cube reorientation
src/Scramble.cpp            Seeded and unseeded random move sequences
src/CubeValidation.cpp      Physical-state validation
src/CubeView.cpp            Smooth viewing and orientation-aware controls
src/main.cpp                Window, keyboard controls, and 3D drawing
bindings/python_bindings.cpp Python names for the C++ API
research/                   Small Python examples and experiment helpers
test/                       Native and Python correctness checks
build/                      Generated binaries and Python extension (ignored)
```

`rubiks_core` has no dependency on Python or raylib. The `rubiks` extension links
that core; the viewer links the core and raylib. This lets experiments use the
same mechanics as the display while running without a graphics window.
