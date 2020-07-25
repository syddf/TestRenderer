CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(AssetImporter)

FILE (GLOB SOURCE_LIST "./*.cpp" "./*.h")
list (APPEND SOURCE_LIST ${CMAKE_SOURCE_DIR}/Source/Prefix.h)
ADD_EXECUTABLE(importer ${SOURCE_LIST})

include_directories(${CMAKE_SOURCE_DIR}/External/glm)
include_directories(${CMAKE_SOURCE_DIR}/External/Assimp/include)
link_directories(${CMAKE_SOURCE_DIR}/External/Assimp/lib/)
target_link_libraries(importer ${CMAKE_SOURCE_DIR}/External/Assimp/lib/assimp-vc140-mt.lib)

add_custom_command(TARGET importer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/External/Assimp/lib/assimp-vc140-mt.dll"
        $<TARGET_FILE_DIR:importer>)