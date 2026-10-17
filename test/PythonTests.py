"""Python API contract checks. Run through CTest or with PYTHONPATH=build."""

import unittest

from rubiks import Color, Cube, Face, validate_cube


FACE_NAMES = ("up", "down", "left", "right", "front", "back")
SOLVED_KEY = "W" * 9 + "Y" * 9 + "O" * 9 + "R" * 9 + "G" * 9 + "B" * 9
MOVE_NAMES = tuple(
    face + suffix for face in "LRUDFBMES" for suffix in ("", "prime", "2")
)


def solid_face(color):
    return Face([[color] * 3 for _ in range(3)])


def solved_cube():
    return Cube(*(solid_face(color) for color in (
        Color.W, Color.Y, Color.O, Color.R, Color.G, Color.B
    )))


class PythonAPITests(unittest.TestCase):
    def test_constructor_and_face_order(self):
        cube = solved_cube()
        self.assertEqual(cube.state_key(), SOLVED_KEY)
        for name, color in zip(FACE_NAMES, (
            Color.W, Color.Y, Color.O, Color.R, Color.G, Color.B
        )):
            self.assertEqual(getattr(cube, f"get_{name}_face")().get_color(1, 1), color)

    def test_face_grid_roundtrip_and_replacement(self):
        grid = [
            [Color.R, Color.G, Color.B],
            [Color.Y, Color.W, Color.O],
            [Color.G, Color.R, Color.Y],
        ]
        face = Face(colors=grid)
        self.assertEqual(face.get_colors(), grid)
        self.assertEqual(face.get_color(row=0, col=1), Color.G)
        replacement = [[Color.B] * 3 for _ in range(3)]
        self.assertIsNone(face.set_colors(replacement))
        self.assertEqual(face.get_colors(), replacement)

    def test_python_lists_are_copies(self):
        grid = [[Color.W] * 3 for _ in range(3)]
        face = Face(grid)
        grid[0][0] = Color.R
        self.assertEqual(face.get_color(0, 0), Color.W)
        snapshot = face.get_colors()
        snapshot[1][1] = Color.G
        self.assertEqual(face.get_color(1, 1), Color.W)

    def test_constructor_and_face_getters_copy(self):
        faces = [solid_face(color) for color in (
            Color.W, Color.Y, Color.O, Color.R, Color.G, Color.B
        )]
        cube = Cube(*faces)
        faces[0].set_colors([[Color.R] * 3 for _ in range(3)])
        self.assertEqual(cube.state_key(), SOLVED_KEY)
        detached = cube.get_front_face()
        detached.set_colors([[Color.R] * 3 for _ in range(3)])
        self.assertEqual(cube.state_key(), SOLVED_KEY)

    def test_face_bounds(self):
        face = solid_face(Color.W)
        for row, col in ((-1, 0), (0, -1), (3, 0), (0, 3)):
            with self.subTest(row=row, col=col):
                with self.assertRaises(IndexError):
                    face.get_color(row, col)

    def test_face_shape_and_types(self):
        for grid in ([], [[Color.W] * 3] * 2, [[Color.W] * 2] * 3,
                     [[Color.W] * 4] * 3, [["W"] * 3] * 3):
            with self.subTest(grid=grid):
                with self.assertRaises(TypeError):
                    Face(grid)
        with self.assertRaises(TypeError):
            Cube()

    def test_copy_is_independent(self):
        original = solved_cube()
        branch = original.copy()
        self.assertIsNot(branch, original)
        self.assertEqual(branch.state_key(), original.state_key())
        branch.L()
        branch_key = branch.state_key()
        self.assertEqual(original.state_key(), SOLVED_KEY)
        original.R()
        self.assertEqual(branch.state_key(), branch_key)

    def test_state_key_is_an_immutable_snapshot(self):
        cube = solved_cube()
        key = cube.state_key()
        self.assertIsInstance(key, str)
        self.assertEqual(len(key), 54)
        seen = {key}
        depths = {key: 0}
        cube.L()
        self.assertEqual(key, SOLVED_KEY)
        self.assertNotIn(cube.state_key(), seen)
        self.assertEqual(depths[key], 0)
        cube.Lprime()
        self.assertIn(cube.state_key(), seen)

    def test_all_move_bindings(self):
        for name in MOVE_NAMES:
            with self.subTest(move=name):
                cube = solved_cube()
                self.assertIsNone(getattr(cube, name)())
                self.assertFalse(cube.is_solved())
                self.assertNotEqual(cube.state_key(), SOLVED_KEY)
                self.assertEqual(validate_cube(cube), "")
                for _ in range(3):
                    getattr(cube, name)()
                self.assertTrue(cube.is_solved())
                self.assertEqual(cube.state_key(), SOLVED_KEY)

    def test_solvedness_and_validation_are_distinct_from_construction(self):
        cube = Cube(*(solid_face(Color.R) for _ in range(6)))
        self.assertFalse(cube.is_solved())
        self.assertNotEqual(validate_cube(cube), "")
        # A key is a snapshot, not a proof that the state is reachable.
        self.assertEqual(cube.state_key(), "R" * 54)

    def test_any_distinct_solved_centre_scheme(self):
        cube = Cube(*(solid_face(color) for color in (
            Color.G, Color.B, Color.Y, Color.W, Color.R, Color.O
        )))
        self.assertTrue(cube.is_solved())
        self.assertEqual(validate_cube(cube), "")

    def test_seeded_scramble_and_inverse(self):
        first = solved_cube()
        second = first.copy()
        sequence = first.scramble(25, seed=12345)
        self.assertEqual(sequence, second.scramble(move_count=25, seed=12345))
        self.assertEqual(first.state_key(), second.state_key())
        self.assertEqual(validate_cube(first), "")
        self.assertEqual(len(sequence.split()), 25)
        previous_face = None
        for token in sequence.split():
            self.assertIn(token.replace("'", "prime"), MOVE_NAMES)
            self.assertNotEqual(token[0], previous_face)
            previous_face = token[0]

        for token in reversed(sequence.split()):
            inverse = token[:-1] if token.endswith("'") else (
                token if token.endswith("2") else token + "prime"
            )
            getattr(first, inverse)()
        self.assertTrue(first.is_solved())
        self.assertEqual(first.state_key(), SOLVED_KEY)

    def test_default_scramble_and_no_reset(self):
        cube = solved_cube()
        self.assertEqual(len(cube.scramble().split()), 25)
        cube.M()
        original = cube.copy()
        sequence = cube.scramble(10, seed=99)
        for token in sequence.split():
            getattr(original, token.replace("'", "prime"))()
        self.assertEqual(cube.state_key(), original.state_key())

    def test_zero_and_invalid_scrambles(self):
        cube = solved_cube()
        self.assertEqual(cube.scramble(0), "")
        self.assertEqual(cube.state_key(), SOLVED_KEY)
        with self.assertRaises(ValueError):
            cube.scramble(-1)
        with self.assertRaises(TypeError):
            cube.scramble(25, seed=-1)
        self.assertEqual(cube.state_key(), SOLVED_KEY)


if __name__ == "__main__":
    unittest.main(verbosity=2)
