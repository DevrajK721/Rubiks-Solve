cmake_minimum_required(VERSION 3.18)
project(RubiksSolve LANGUAGES CXX)

option(RUBIKS_BUILD_PYTHON "Build the Python extension" ON)
option(RUBIKS_BUILD_VIEWER "Build the raylib viewer" ON)
include(CTest)

# Shared cube mechanics, with no graphics or Python dependency.
add_library(rubiks_core STATIC
    src/Face.cpp
    src/Cube.cpp
    src/Moves.cpp
    src/Scramble.cpp
    src/CubeValidation.cpp
)
target_include_directories(rubiks_core PUBLIC include)
target_compile_features(rubiks_core PUBLIC cxx_std_17)
set_target_properties(rubiks_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(BUILD_TESTING)
    function(add_cube_test name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE rubiks_core)
        add_test(NAME ${name} COMMAND ${name})
    endfunction()

    add_cube_test(face_tests test/FaceTests.cpp)
    add_cube_test(cube_tests test/CubeTests.cpp)
    add_cube_test(move_tests test/MoveTests.cpp)
    add_cube_test(validation_tests test/ValidationTests.cpp)
    add_cube_test(scramble_tests test/ScrambleTests.cpp)
    add_cube_test(cube_utilities_tests test/CubeUtilitiesTests.cpp)
endif()

if(RUBIKS_BUILD_PYTHON)
    find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(rubiks bindings/python_bindings.cpp)
    target_link_libraries(rubiks PRIVATE rubiks_core)

    if(BUILD_TESTING)
        # CTest finds the extension without requiring a shell PYTHONPATH export.
        function(add_python_test name script)
            add_test(NAME ${name}
                COMMAND "${CMAKE_COMMAND}" -E env
                    "PYTHONPATH=$<TARGET_FILE_DIR:rubiks>"
                    "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/${script}"
            )
        endfunction()

        add_python_test(python_api_tests test/PythonTests.py)
        add_python_test(python_face_example research/test_init.py)
        add_python_test(python_cube_example research/test_cube.py)
        add_python_test(python_experiment_example research/experiment_basics.py)
    endif()
endif()

if(RUBIKS_BUILD_VIEWER)
    find_package(raylib REQUIRED)

    if(APPLE)
        # Homebrew's raylib can embed an OpenGL path from a different macOS SDK.
        # Resolve OpenGL in the SDK selected for this build instead.
        get_target_property(raylib_links raylib INTERFACE_LINK_LIBRARIES)
        if(raylib_links)
            list(TRANSFORM raylib_links REPLACE "^.*/OpenGL\\.framework$" "-framework OpenGL")
            set_property(TARGET raylib PROPERTY INTERFACE_LINK_LIBRARIES "${raylib_links}")
        endif()
    endif()

    add_executable(cube_viewer src/main.cpp src/CubeView.cpp)
    target_link_libraries(cube_viewer PRIVATE rubiks_core raylib)

    if(BUILD_TESTING)
        # These checks use raylib maths but do not open a window.
        add_executable(view_tests src/CubeView.cpp test/ViewTests.cpp)
        target_link_libraries(view_tests PRIVATE rubiks_core raylib)
        add_test(NAME view_tests COMMAND view_tests)
    endif()
endif()
