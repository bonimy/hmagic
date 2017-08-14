cmake_minimum_required(VERSION 2.8)

project(HMagic)

add_executable(HMagic WIN32
               z3ed.c
               resource.h
               z3ed.rc
               icon1.ico)
               
# Needed for MIDI functionality.
target_link_libraries(HMagic Winmm)

# For Windows common controls.
target_link_libraries(HMagic ComCtl32)