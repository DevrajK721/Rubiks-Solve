#include "raylib.h"
#include "rlgl.h"
#include "rubiks/Cube.hpp"
#include "rubiks/CubeView.hpp"
#include "raymath.h"
#include <string>

::Color toDrawingColor(rubiks::Color color) {
    switch (color) {
        case rubiks::Color::R: return RED;
        case rubiks::Color::O: return ORANGE;
        case rubiks::Color::Y: return YELLOW;
        case rubiks::Color::G: return GREEN;
        case rubiks::Color::B: return BLUE;
        case rubiks::Color::W: return RAYWHITE;
    }

    return MAGENTA; // Makes an unexpected colour value easy to spot.
}
int main() {
    InitWindow(900, 600, "Rubik's Cube");
    SetTargetFPS(100);

    Camera3D camera = {};
    camera.position = {6.0f, 6.0f, 6.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE; 

    using C = rubiks::Color; 
    using rubiks::Face; 
    using rubiks::FaceColorArray; 

    // Each entry is an individual sticker that you can change.
    FaceColorArray up    = {{{C::W, C::W, C::W}, {C::W, C::W, C::W}, {C::W, C::W, C::W}}};
    FaceColorArray down  = {{{C::Y, C::Y, C::Y}, {C::Y, C::Y, C::Y}, {C::Y, C::Y, C::Y}}};
    FaceColorArray left  = {{{C::O, C::O, C::O}, {C::O, C::O, C::O}, {C::O, C::O, C::O}}};
    FaceColorArray right = {{{C::R, C::R, C::R}, {C::R, C::R, C::R}, {C::R, C::R, C::R}}};
    FaceColorArray front = {{{C::G, C::G, C::G}, {C::G, C::G, C::G}, {C::G, C::G, C::G}}};
    FaceColorArray back  = {{{C::B, C::B, C::B}, {C::B, C::B, C::B}, {C::B, C::B, C::B}}};

    rubiks::Cube cube{
        Face(up), Face(down), Face(left),
        Face(right), Face(front), Face(back)
    }; 

    rubiks::CubeView view(camera);
    std::string scrambleMoves;

    while (!WindowShouldClose()) {
        // Hold arrows to rotate smoothly; release to keep the partial angle.
        float frameTime = GetFrameTime();
        if (frameTime > 0.05f) frameTime = 0.05f; // Avoid a leap after a pause.
        const float degreesPerSecond = 90.0f;
        float step = degreesPerSecond * frameTime;
        float horizontal = 0.0f;
        float vertical = 0.0f;

        if (IsKeyDown(KEY_RIGHT)) horizontal += step;
        if (IsKeyDown(KEY_LEFT))  horizontal -= step;
        if (IsKeyDown(KEY_UP))    vertical += step;
        if (IsKeyDown(KEY_DOWN))  vertical -= step;

        // Update the control orientation before handling this frame's moves.
        view.rotate(cube, horizontal, vertical);

        if (IsKeyPressed(KEY_SPACE)) {
            scrambleMoves = cube.scramble();
        }

        // Either Shift key can select an inverse move.
        bool shift = IsKeyDown(KEY_LEFT_SHIFT)
                || IsKeyDown(KEY_RIGHT_SHIFT);

        // Move commands 
        if (IsKeyPressed(KEY_L)) {
            if (shift) cube.Lprime();
            else       cube.L();
        }
        if (IsKeyPressed(KEY_R)) {
            if (shift) cube.Rprime();
            else       cube.R();
        }
        if (IsKeyPressed(KEY_U)) {
            if (shift) cube.Uprime();
            else       cube.U();
        }
        if (IsKeyPressed(KEY_D)) {
            if (shift) cube.Dprime();
            else       cube.D();
        }
        if (IsKeyPressed(KEY_F)) {
            if (shift) cube.Fprime();
            else       cube.F();
        }
        if (IsKeyPressed(KEY_B)) {
            if (shift) cube.Bprime();
            else       cube.B();
        }
        if (IsKeyPressed(KEY_M)) {
            if (shift) cube.Mprime();
            else       cube.M();
        }
        if (IsKeyPressed(KEY_E)) {
            if (shift) cube.Eprime();
            else       cube.E();
        } 
        if (IsKeyPressed(KEY_S)) {
            if (shift) cube.Sprime();
            else       cube.S();
        }

        BeginDrawing(); 

        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(view.getDrawRotation()));

        DrawCube({0.0f, 0.0f, 0.0f}, 3.0f, 3.0f, 3.0f, DARKGRAY);
        DrawCubeWires({0.0f, 0.0f, 0.0f}, 3.0f, 3.0f, 3.0f, BLACK);

        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                float x = col - 1.0f; 
                float y = 1.0f - row;
                float z = 1.51f;
            
                // Front
                DrawCube({x, y, z}, 0.9f, 0.9f, 0.02f,
                    toDrawingColor(cube.getFrontFace().getColor(row, col)));

                // Back
                DrawCube({-x, y, -z}, 0.9f, 0.9f, 0.02f,
                    toDrawingColor(cube.getBackFace().getColor(row, col)));

                // Right
                DrawCube({z, y, -x}, 0.02f, 0.9f, 0.9f,
                    toDrawingColor(cube.getRightFace().getColor(row, col)));

                // Left
                DrawCube({-z, y, x}, 0.02f, 0.9f, 0.9f,
                    toDrawingColor(cube.getLeftFace().getColor(row, col)));

                // Up
                DrawCube({x, z, -y}, 0.9f, 0.02f, 0.9f,
                    toDrawingColor(cube.getUpFace().getColor(row, col)));

                // Down
                DrawCube({x, -z, y}, 0.9f, 0.02f, 0.9f,
                    toDrawingColor(cube.getDownFace().getColor(row, col)));
            }
        }

        rlPopMatrix();
        EndMode3D();
        DrawText("Cube viewer", 20, 20, 24, DARKGRAY);
        DrawText("Arrows: view | L R U D F B M E S: turn | Shift: inverse | Space: scramble",
                 20, 55, 16, DARKGRAY);
        if (!scrambleMoves.empty()) {
            std::string label = "Scramble: " + scrambleMoves;
            DrawText(label.c_str(), 20, GetScreenHeight() - 30, 16, DARKGRAY);
        }

        EndDrawing(); 
    }

    CloseWindow();
    return 0;
}
