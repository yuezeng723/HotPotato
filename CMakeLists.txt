cmake_minimum_required(VERSION 3.20)
project(potato C)

set(CMAKE_C_STANDARD 99)

add_executable(ringmaster ringmaster.c potato.h host.c host.h)

add_executable(player player.c potato.h host.c host.h)
