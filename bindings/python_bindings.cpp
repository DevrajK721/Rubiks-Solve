#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "rubiks/Color.hpp"
#include "rubiks/Face.hpp"
#include "rubiks/Cube.hpp"
#include "rubiks/CubeValidation.hpp"

namespace py = pybind11;

PYBIND11_MODULE(rubiks, module) {
    module.doc() = "Python interface to the Rubik's cube simulator.";

    py::enum_<rubiks::Color>(module, "Color") 
        .value("R", rubiks::Color::R)
        .value("O", rubiks::Color::O)
        .value("Y", rubiks::Color::Y)
        .value("G", rubiks::Color::G)
        .value("B", rubiks::Color::B)
        .value("W", rubiks::Color::W);

    py::class_<rubiks::Face>(module, "Face")
        .def(py::init<rubiks::FaceColorArray>(),
            py::arg("colors"))
        .def("get_color", &rubiks::Face::getColor,
            py::arg("row"), py::arg("col"))
        .def("get_colors", &rubiks::Face::getColors)
        .def("set_colors", &rubiks::Face::setColors,
            py::arg("colors"));

    using rubiks::Cube;
    using rubiks::Face;

    py::class_<Cube>(module, "Cube")
        .def(py::init<Face, Face, Face, Face, Face, Face>(),
            py::arg("up"),
            py::arg("down"),
            py::arg("left"),
            py::arg("right"),
            py::arg("front"),
            py::arg("back"))
        .def("get_up_face", &Cube::getUpFace)
        .def("get_down_face", &Cube::getDownFace)
        .def("get_left_face", &Cube::getLeftFace)
        .def("get_right_face", &Cube::getRightFace)
        .def("get_front_face", &Cube::getFrontFace)
        .def("get_back_face", &Cube::getBackFace)
        .def("copy", &Cube::copy,
            "Return an independent copy of this cube.")
        .def("is_solved", &Cube::isSolved,
            "Return whether the cube is valid and each face has one colour.")
        .def("state_key", &Cube::stateKey,
            "Return 54 colour letters in U/D/L/R/F/B row-major order.")
        .def("L", &Cube::L)
        .def("Lprime", &Cube::Lprime)
        .def("L2", &Cube::L2)
        .def("R", &Cube::R)
        .def("Rprime", &Cube::Rprime)
        .def("R2", &Cube::R2)
        .def("U", &Cube::U)
        .def("Uprime", &Cube::Uprime)
        .def("U2", &Cube::U2)
        .def("D", &Cube::D)
        .def("Dprime", &Cube::Dprime)
        .def("D2", &Cube::D2)
        .def("F", &Cube::F)
        .def("Fprime", &Cube::Fprime)
        .def("F2", &Cube::F2)
        .def("B", &Cube::B)
        .def("Bprime", &Cube::Bprime)
        .def("B2", &Cube::B2)
        .def("M", &Cube::M)
        .def("Mprime", &Cube::Mprime)
        .def("M2", &Cube::M2)
        .def("E", &Cube::E)
        .def("Eprime", &Cube::Eprime)
        .def("E2", &Cube::E2)
        .def("S", &Cube::S)
        .def("Sprime", &Cube::Sprime)
        .def("S2", &Cube::S2)
        .def("scramble",
            py::overload_cast<int>(&Cube::scramble),
            py::arg("move_count") = 25)
        .def("scramble",
            py::overload_cast<int, unsigned int>(&Cube::scramble),
            py::arg("move_count"),
            py::arg("seed"));

    module.def("validate_cube",
               &rubiks::validateCube,
               py::arg("cube"),
               "Return an empty string for a possible cube, otherwise the first problem found.");
}
