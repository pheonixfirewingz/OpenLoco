set(public_files
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OpenLoco/Audio/AudioChannel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OpenLoco/Audio/AudioEffect.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OpenLoco/Audio/AudioEngine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OpenLoco/Audio/AudioFormat.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OpenLoco/Audio/AudioHandle.h"
)

set(private_files
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioEngine.cpp"
)

set(public_link_libraries
    Core
    Diagnostics
)

# For Emscripten, create a stub library that doesn't require OpenAL
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # Create a stub Audio library for web builds (audio not supported)
    set(emscripten_files
        "${CMAKE_CURRENT_SOURCE_DIR}/src/AudioEngine.Emscripten.cpp"
    )
    
    add_library(Audio STATIC
        ${public_files}
        ${emscripten_files}
    )
    add_library(OpenLoco::Audio ALIAS Audio)
    
    target_include_directories(Audio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(Audio PUBLIC ${public_link_libraries})
    
    # Add stub compile definition
    target_compile_definitions(Audio PUBLIC OPENLOCO_AUDIO_STUB=1)
else()
    set(private_link_libraries
        ${OPENAL_LIBRARIES}
    )
    
    loco_add_library(Audio STATIC
        PUBLIC_FILES
            ${public_files}
        PRIVATE_FILES
            ${private_files}
        PUBLIC_LINK_LIBRARIES
            ${public_link_libraries}
        PRIVATE_LINK_LIBRARIES
            ${private_link_libraries}
    )
endif()
