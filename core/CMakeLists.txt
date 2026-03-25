add_library(WaveLogic STATIC 
    src/WaveLogic.cpp 
    include/WaveLogic.h
)

# This allows the renderer to simply #include <WaveLogic.h>
target_include_directories(WaveLogic PUBLIC include)