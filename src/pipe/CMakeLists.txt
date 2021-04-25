set(SOURCES
    GenericComponent.cpp
    GenericComponent.hpp
    Component.hpp
    Line.hpp
    Parallel.hpp
    Interpreter.hpp
    Interpreter.cpp
    ComponentContainer.hpp)
add_library(pipe ${SOURCES})
