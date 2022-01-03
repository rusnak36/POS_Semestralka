cmake_minimum_required(VERSION 3.13)
project(POS_Semestralka C)

set(CMAKE_C_STANDARD 11)

add_executable(Main Main.c)
add_executable(Server server.c)
add_executable(Client client.c)