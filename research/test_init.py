from rubiks import Color, Face 

colors = [
    [Color.R, Color.G, Color.B],
    [Color.Y, Color.W, Color.O],
    [Color.G, Color.R, Color.Y]
]

face = Face(colors) 

assert face.get_color(0, 0) == Color.R
assert face.get_color(1, 1) == Color.W
assert face.get_colors() == colors 
