set(SOURCES
    Canvas.cpp
    Canvas.hpp
    Layer.cpp
    Layer.hpp
    Scope.cpp
    Scope.hpp
    Waterfall.cpp
    Waterfall.hpp
    FrequencyPlot.cpp
    FrequencyPlot.hpp
    TextDisplay.cpp
    TextDisplay.hpp
    Indicator.cpp
    Indicator.hpp
    XYScope.hpp
    XYScope.cpp)
add_library(component ${SOURCES})
target_link_libraries(component ui util pipe)
