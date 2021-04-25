set(SOURCES
    ProcessingThread.cpp
    ProcessingThread.hpp
    SoundCardThread.cpp
    SoundCardThread.hpp
    StdinThread.cpp
    StdinThread.hpp)
add_library(thread "${SOURCES}")
