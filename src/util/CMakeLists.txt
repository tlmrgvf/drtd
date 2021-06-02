set(SOURCES
    Buffer.hpp
    Config.cpp
    Config.hpp
    Limiter.hpp
    Logger.cpp
    Logger.hpp
    Marker.hpp
    Point.hpp
    Resampler.cpp 
    Resampler.hpp
    RingBuffer.hpp
    Size.hpp
    Util.cpp 
    Util.hpp
    Singleton.hpp
    FFT.cpp
    FFT.hpp
    Cmplx.hpp
    BitBuffer.hpp
    Types.hpp
    CallbackManager.hpp
    CallbackManager.cpp
    SNRCalculator.hpp
    SNRCalculator.cpp)
add_library(util ${SOURCES})
add_subdirectory(bch)
target_link_libraries(util dsp bch)
