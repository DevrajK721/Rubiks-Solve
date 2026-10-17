cmake_minimum_required(VERSION 3.16)
project(RubiksSolve LANGUAGES CXX)

add_library(rubiks_core STATIC
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    src/Scramble.cpp
    src/CubeValidation.cpp
)

target_include_directories(rubiks_core PUBLIC include)
target_compile_features(rubiks_core PUBLIC cxx_std_17)

set_target_properties(rubiks_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

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

add_executable(validation_tests
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    src/CubeValidation.cpp
    test/ValidationTests.cpp
)

add_executable(scramble_tests
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    src/Scramble.cpp
    src/CubeValidation.cpp
    test/ScrambleTests.cpp
)
target_include_directories(scramble_tests PRIVATE include)
target_compile_features(scramble_tests PRIVATE cxx_std_17)

# -- Main Usage --
find_package(raylib REQUIRED)

if(APPLE)
    # Homebrew's raylib can embed an OpenGL path from a different macOS SDK.
    # Let the linker find OpenGL in the SDK selected for this build instead.
    get_target_property(raylib_links raylib INTERFACE_LINK_LIBRARIES)
    if(raylib_links)
        list(TRANSFORM raylib_links REPLACE "^.*/OpenGL\\.framework$" "-framework OpenGL")
        set_property(TARGET raylib PROPERTY INTERFACE_LINK_LIBRARIES "${raylib_links}")
    endif()
endif()

add_executable(cube_viewer
    src/main.cpp
    src/CubeView.cpp
)

target_link_libraries(cube_viewer PRIVATE rubiks_core raylib)
target_compile_features(cube_viewer PRIVATE cxx_std_17)
target_include_directories(cube_viewer PRIVATE include)

# These checks use raylib's maths but do not open a window.
add_executable(view_tests
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    src/CubeValidation.cpp
    src/CubeView.cpp
    test/ViewTests.cpp
)
target_link_libraries(view_tests PRIVATE raylib)
target_compile_features(view_tests PRIVATE cxx_std_17)
target_include_directories(view_tests PRIVATE include)

# -- Compiler Setup --
target_include_directories(face_tests PRIVATE include)
target_compile_features(face_tests PRIVATE cxx_std_17)
target_include_directories(cube_tests PRIVATE include)
target_compile_features(cube_tests PRIVATE cxx_std_17)
target_include_directories(move_tests PRIVATE include)
target_compile_features(move_tests PRIVATE cxx_std_17)
target_include_directories(validation_tests PRIVATE include)
target_compile_features(validation_tests PRIVATE cxx_std_17)

enable_testing()
add_test(NAME face_tests COMMAND face_tests)
add_test(NAME cube_tests COMMAND cube_tests)
add_test(NAME move_tests COMMAND move_tests)
add_test(NAME validation_tests COMMAND validation_tests)
add_test(NAME scramble_tests COMMAND scramble_tests)
add_test(NAME view_tests COMMAND view_tests)
