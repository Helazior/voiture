cmake_minimum_required(VERSION 3.21)
project(voiture C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(voiture
        include/background.h
        include/collision_car.h
        include/create_map.h
        include/ia.h
        include/jeu.h
        src/background.c
        src/collision_car.c
        src/create_map.c
        src/ia.c
        src/jeu.c
        src/main.c)
