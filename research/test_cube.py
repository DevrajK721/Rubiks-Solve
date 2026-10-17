from rubiks import Color, Face, Cube, validate_cube

def solid_face(color):
    return Face([[color] * 3 for _ in range(3)])

cube = Cube(
    up=solid_face(Color.W),
    down=solid_face(Color.Y),
    left=solid_face(Color.O),
    right=solid_face(Color.R),
    front=solid_face(Color.G),
    back=solid_face(Color.B),
)

original_front = cube.get_front_face().get_colors()

cube.L()

assert cube.get_front_face().get_color(0, 0) == Color.W
assert cube.get_front_face().get_color(0, 1) == Color.G

cube.Lprime()

assert cube.get_front_face().get_colors() == original_front

cube.R()
cube.Rprime()

moves = cube.scramble(25, seed=12345)
print("Scramble:", moves)

error = validate_cube(cube)
assert error == "", error

print("Cube binding test passed!")