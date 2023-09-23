cmake_minimum_required(VERSION 3.26)
project(LLP_Lab1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

# Add the src directory and all its subdirectories to the list of source directories
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp" "src/*.cxx")
# Create the executable from the sources
add_executable(${PROJECT_NAME} ${SOURCES})


