"""Runnable Python API examples and small helpers for experiments.

From the project root:
    PYTHONPATH=build .venv/bin/python -m research.experiment_basics
"""

from rubiks import Color, Cube, Face, validate_cube


FACE_NAMES = ("up", "down", "left", "right", "front", "back")
MOVE_TOKENS = {
    face + suffix for face in "LRUDFBMES" for suffix in ("", "'", "2")
}


def solved_cube():
    """Construct a solved example; the C++ Cube constructor still takes six faces."""
    def solid(color):
        return Face([[color] * 3 for _ in range(3)])

    return Cube(
        up=solid(Color.W), down=solid(Color.Y), left=solid(Color.O),
        right=solid(Color.R), front=solid(Color.G), back=solid(Color.B),
    )


def checked_tokens(sequence):
    tokens = sequence.split()
    for token in tokens:
        if token not in MOVE_TOKENS:
            raise ValueError(f"Unknown move: {token}")
    return tokens


def apply_sequence(cube, sequence):
    """Apply space-separated notation, checking every token before any move."""
    for token in checked_tokens(sequence):
        getattr(cube, token.replace("'", "prime"))()


def undo_sequence(cube, sequence):
    """Undo the moves in reverse order. Keep the same coordinate orientation."""
    for token in reversed(checked_tokens(sequence)):
        if token.endswith("'"):
            inverse = token[:-1]
        elif token.endswith("2"):
            inverse = token
        else:
            inverse = token + "prime"
        getattr(cube, inverse)()


def cube_from_key(key):
    """Reconstruct and validate a 54-letter U/D/L/R/F/B state snapshot."""
    if not isinstance(key, str) or len(key) != 54 or any(c not in "ROYGBW" for c in key):
        raise ValueError("Expected 54 uppercase colour letters: R, O, Y, G, B, W.")

    faces = []
    for start in range(0, 54, 9):
        grid = [
            [getattr(Color, key[start + row * 3 + col]) for col in range(3)]
            for row in range(3)
        ]
        faces.append(Face(grid))

    cube = Cube(*faces)
    error = validate_cube(cube)
    if error:
        raise ValueError(error)
    return cube


def main():
    start = solved_cube()
    assert start.is_solved()
    initial_key = start.state_key()

    scrambled = start.copy()
    sequence = scrambled.scramble(25, seed=12345)
    assert start.state_key() == initial_key
    assert validate_cube(scrambled) == ""
    print("Scramble:", sequence)
    print("State:", scrambled.state_key())

    replayed = start.copy()
    apply_sequence(replayed, sequence)
    assert replayed.state_key() == scrambled.state_key()

    restored = cube_from_key(scrambled.state_key())
    undo_sequence(restored, sequence)
    assert restored.is_solved()
    assert restored.state_key() == initial_key

    # Explore one move from a position that is one move away from solved.
    position = solved_cube()
    position.R()
    visited = {position.state_key()}
    solution = None
    for move in (face + suffix for face in "LRUDFB" for suffix in ("", "prime", "2")):
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
    assert not position.is_solved()
    print("One-move solution:", solution)
    print("Experiment examples passed.")


if __name__ == "__main__":
    main()
