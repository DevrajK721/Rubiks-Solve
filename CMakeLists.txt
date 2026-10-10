cmake_minimum_required(VERSION 3.16)
project(RubiksSolve LANGUAGES CXX)

# -- Tests --
add_executable(face_tests
    src/Face.cpp
    test/FaceTests.cpp
)

add_executable(cube_tests
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    test/CubeTests.cpp
)

add_executable(move_tests
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    test/MoveTests.cpp
)

# -- Main Usage --

# -- Compiler Setup --
target_include_directories(face_tests PRIVATE include)
target_compile_features(face_tests PRIVATE cxx_std_17)
target_include_directories(cube_tests PRIVATE include)
target_compile_features(cube_tests PRIVATE cxx_std_17)
target_include_directories(move_tests PRIVATE include)
target_compile_features(move_tests PRIVATE cxx_std_17)

enable_testing()
add_test(NAME face_tests COMMAND face_tests)
add_test(NAME cube_tests COMMAND cube_tests)
add_test(NAME move_tests COMMAND move_tests)
