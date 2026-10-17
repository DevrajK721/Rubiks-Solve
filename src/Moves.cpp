#include "rubiks/Cube.hpp"

namespace rubiks {
    // See README.md for the orientation of each face's rows and columns.
    void Cube::rotateFaceClockwise(Face& face) {
        FaceColorArray newColors;
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                newColors[col][2 - row] = face.getColor(row, col);
            }
        }
        face.setColors(newColors);
    }

    void Cube::L() {
        rotateFaceClockwise(left);

        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newUp = up.getColors();
        FaceColorArray newFront = front.getColors();
        FaceColorArray newDown = down.getColors();
        FaceColorArray newBack = back.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newUp[i][0] = back.getColor(2 - i, 2);
            newFront[i][0] = up.getColor(i, 0);
            newDown[i][0] = front.getColor(i, 0);
            newBack[i][2] = down.getColor(2 - i, 0);
        }

        up.setColors(newUp);
        front.setColors(newFront);
        down.setColors(newDown);
        back.setColors(newBack);
    }

    void Cube::Lprime() {
        L();
        L();
        L();
    }

    void Cube::L2() {
        L();
        L();
    }

    void Cube::R() {
        rotateFaceClockwise(right);

        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newUp = up.getColors();
        FaceColorArray newFront = front.getColors();
        FaceColorArray newDown = down.getColors();
        FaceColorArray newBack = back.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newUp[i][2] = front.getColor(i, 2);
            newFront[i][2] = down.getColor(i, 2);
            newDown[i][2] = back.getColor(2 - i, 0);
            newBack[i][0] = up.getColor(2 - i, 2);
        }

        up.setColors(newUp);
        front.setColors(newFront);
        down.setColors(newDown);
        back.setColors(newBack);
    }

    void Cube::Rprime() {
        R();
        R();
        R();
    }

    void Cube::R2() {
        R();
        R();
    }

    void Cube::U() {
        rotateFaceClockwise(up);

        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newFront = front.getColors();
        FaceColorArray newRight = right.getColors();
        FaceColorArray newBack = back.getColors();
        FaceColorArray newLeft = left.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newFront[0][i] = right.getColor(0, i);
            newRight[0][i] = back.getColor(0, i);
            newBack[0][i] = left.getColor(0, i);
            newLeft[0][i] = front.getColor(0, i);
        }

        front.setColors(newFront);
        right.setColors(newRight);
        back.setColors(newBack);
        left.setColors(newLeft);
    }

    void Cube::Uprime() {
        U();
        U();
        U();
    }

    void Cube::U2() {
        U();
        U();
    }

    void Cube::D() {
        rotateFaceClockwise(down);

        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newFront = front.getColors();
        FaceColorArray newRight = right.getColors();
        FaceColorArray newBack = back.getColors();
        FaceColorArray newLeft = left.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newFront[2][i] = left.getColor(2, i);
            newRight[2][i] = front.getColor(2, i);
            newBack[2][i] = right.getColor(2, i);
            newLeft[2][i] = back.getColor(2, i);
        }

        front.setColors(newFront);
        right.setColors(newRight);
        back.setColors(newBack);
        left.setColors(newLeft);
    }

    void Cube::Dprime() {
        D();
        D();
        D();
    }

    void Cube::D2() {
        D();
        D();
    }

    void Cube::F() {
        rotateFaceClockwise(front);

        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newUp = up.getColors();
        FaceColorArray newRight = right.getColors();
        FaceColorArray newDown = down.getColors();
        FaceColorArray newLeft = left.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newUp[2][i] = left.getColor(2 - i, 2);
            newRight[i][0] = up.getColor(2, i);
            newDown[0][i] = right.getColor(2 - i, 0);
            newLeft[i][2] = down.getColor(0, i);
        }

        up.setColors(newUp);
        right.setColors(newRight);
        down.setColors(newDown);
        left.setColors(newLeft);
    }

    void Cube::Fprime() {
        F();
        F();
        F();
    }

    void Cube::F2() {
        F();
        F();
    }

    void Cube::B() {
        rotateFaceClockwise(back);

        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newUp = up.getColors();
        FaceColorArray newRight = right.getColors();
        FaceColorArray newDown = down.getColors();
        FaceColorArray newLeft = left.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newUp[0][i] = right.getColor(i, 2);
            newRight[i][2] = down.getColor(2, 2 - i);
            newDown[2][i] = left.getColor(i, 0);
            newLeft[i][0] = up.getColor(0, 2 - i);
        }

        up.setColors(newUp);
        right.setColors(newRight);
        down.setColors(newDown);
        left.setColors(newLeft);
    }

    void Cube::Bprime() {
        B();
        B();
        B();
    }

    void Cube::B2() {
        B();
        B();
    }

    void Cube::M() {
        // M turns the middle slice in the same direction as L.
        // This includes the centres of up, front, down, and back.
        // Copy the grids so the other stickers keep their original colours.
        FaceColorArray newUp = up.getColors();
        FaceColorArray newFront = front.getColors();
        FaceColorArray newDown = down.getColors();
        FaceColorArray newBack = back.getColors();

        // Read from the original faces until every new strip is ready.
        for (int i = 0; i < 3; ++i) {
            newUp[i][1] = back.getColor(2 - i, 1);
            newFront[i][1] = up.getColor(i, 1);
            newDown[i][1] = front.getColor(i, 1);
            newBack[i][1] = down.getColor(2 - i, 1);
        }

        up.setColors(newUp);
        front.setColors(newFront);
        down.setColors(newDown);
        back.setColors(newBack);
    }

    void Cube::Mprime() {
        M();
        M();
        M();
    }

    void Cube::M2() {
        M();
        M();
    }

    void Cube::E() {
        FaceColorArray newFront = front.getColors();
        FaceColorArray newRight = right.getColors();
        FaceColorArray newBack = back.getColors();
        FaceColorArray newLeft = left.getColors();

        // Transfer the middle rows in the same direction as D.
        for (int col = 0; col < 3; ++col) {
            newFront[1][col] = left.getColor(1, col);
            newRight[1][col] = front.getColor(1, col);
            newBack[1][col] = right.getColor(1, col);
            newLeft[1][col] = back.getColor(1, col);
        }

        front.setColors(newFront);
        right.setColors(newRight);
        back.setColors(newBack);
        left.setColors(newLeft);
    }

    void Cube::Eprime() {
        E();
        E();
        E();
    }

    void Cube::E2() {
        E();
        E();
    }

    void Cube::S() {
        FaceColorArray newUp = up.getColors();
        FaceColorArray newRight = right.getColors();
        FaceColorArray newDown = down.getColors();
        FaceColorArray newLeft = left.getColors();

        for (int i = 0; i < 3; ++i) {
            newUp[1][i] = left.getColor(2 - i, 1);
            newRight[i][1] = up.getColor(1, i);
            newDown[1][i] = right.getColor(2 - i, 1);
            newLeft[i][1] = down.getColor(1, i);
        }

        up.setColors(newUp);
        right.setColors(newRight);
        down.setColors(newDown);
        left.setColors(newLeft);
    }

    void Cube::Sprime() {
        S();
        S();
        S();
    }

    void Cube::S2() {
        S();
        S();
    }

    void Cube::rotateWholeRight() {
        // the old front becomes the new right 
        Face oldFront = front; 

        front = left; 
        left = back; 
        back = right; 
        right = oldFront; 

        // the top and bottom stay in place, but their grids rotate 
        // Top: anticlockwise 
        rotateFaceClockwise(up);
        rotateFaceClockwise(up);
        rotateFaceClockwise(up);

        // Bottom: clockwise 
        rotateFaceClockwise(down);
    }

    void Cube::rotateWholeLeft() {
        rotateWholeRight();
        rotateWholeRight();
        rotateWholeRight();
    }

    void Cube::rotateWholeUp() {
        R();
        Mprime();
        Lprime();
    }

    void Cube::rotateWholeDown() {
        Rprime();
        M();
        L();
    }
}
