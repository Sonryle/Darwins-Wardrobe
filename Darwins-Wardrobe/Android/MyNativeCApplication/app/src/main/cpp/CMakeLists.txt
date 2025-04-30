# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

project("mynativecapplication")

add_library(${CMAKE_PROJECT_NAME} SHARED
        main.cpp)

# Searches for game-activity package
find_package(game-activity REQUIRED CONFIG)

target_link_libraries(${CMAKE_PROJECT_NAME}
        # Include game-activity for native-app-glue
        game-activity::game-activity

        # Include dependencies for communicating with android
        android
        log)