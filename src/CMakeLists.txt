set(SOURCES
    Drtd.cpp
    Drtd.hpp)
add_executable(drtd ${SOURCES})

add_subdirectory(decoder)
add_subdirectory(dsp)
add_subdirectory(pipe)
add_subdirectory(thread)
add_subdirectory(ui)
add_subdirectory(util)

find_library(LIB_ASND asound)
find_library(LIB_FFTW3 fftw3)
find_library(LIB_PTHREAD pthread)
set(LIB_STDCPPFS stdc++fs)

set(LIBS
    decoder
    dsp
    pipe
    thread
    ui
    util
    component)

target_link_libraries(drtd 
    ${LIBS}
    "${LIB_ASND}" 
    "${LIB_FFTW3}" 
    "${LIB_PTHREAD}" 
    "${LIB_STDCPPFS}" 
    "${FLTK_LIBRARIES}" 
    "${OPENGL_LIBRARIES}")
