CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(PathTracer)

AUX_SOURCE_DIRECTORY(. SRC_LIST)
FILE (GLOB SOURCE_LIST "./Vulkan/*.cpp" "./Vulkan/*.h")
foreach(target ${SOURCE_LIST})
	list(APPEND SRC_LIST ${target})
endforeach()
FILE (GLOB SOURCE_LIST "./GraphicsAPICommon/*.cpp" "./GraphicsAPICommon/*.h")
foreach(target ${SOURCE_LIST})
	list(APPEND SRC_LIST ${target})
endforeach()

list (APPEND SOURCE_LIST ${CMAKE_SOURCE_DIR}/Source/Prefix.h)
ADD_EXECUTABLE(pt ${SRC_LIST})

include_directories(${CMAKE_SOURCE_DIR}/External/glm)
include_directories(${CMAKE_SOURCE_DIR}/External/Glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/External/Vulkan/include)
include_directories(${CMAKE_SOURCE_DIR}/External/stb_image)

source_group("Vulkan/" FILES 
./Vulkan/Device.h 
./Vulkan/Device.cpp 
./Vulkan/VulkanCommon.h
./Vulkan/VulkanCommon.cpp
./Vulkan/Window.h 
./Vulkan/Window.cpp 
./Vulkan/SwapChain.h 
./Vulkan/SwapChain.cpp 
./Vulkan/Image.h
./Vulkan/ImageView.h
./Vulkan/Image.cpp
./Vulkan/ImageView.cpp
./Vulkan/Shader.h
./Vulkan/Shader.cpp
./Vulkan/RenderPass.h
./Vulkan/RenderPass.cpp
./Vulkan/TranslateEngine.h
./Vulkan/TranslateEngine.cpp
./Vulkan/ResourceCreator.h
./Vulkan/ResourceCreator.cpp
./Vulkan/RenderingPipeline.h
./Vulkan/RenderingPipeline.cpp
./Vulkan/CommandBufferPool.h
./Vulkan/CommandBufferPool.cpp
./Vulkan/Buffer.h
./Vulkan/Buffer.cpp
./Vulkan/Material.h
./Vulkan/Material.cpp
./Vulkan/Mesh.h
./Vulkan/Mesh.cpp
)

source_group("GraphicsAPICommon/" FILES 
./GraphicsAPICommon/GraphicsInterface.h
./GraphicsAPICommon/GraphicsStructure.h
)

if (UNIX AND NOT APPLE)
    MESSAGE(STATUS "unix")
elseif (WIN32)
    link_directories(${CMAKE_SOURCE_DIR}/External/Vulkan/lib/win)
    link_directories(${CMAKE_SOURCE_DIR}/External/Glfw/lib/win)
    target_link_libraries(pt ${CMAKE_SOURCE_DIR}/External/Vulkan/lib/win/vulkan-1.lib)
    target_link_libraries(pt ${CMAKE_SOURCE_DIR}/External/Glfw/lib/win/glfw3.lib)
elseif (APPLE)
    link_directories(${CMAKE_SOURCE_DIR}/External/Vulkan/lib/macos)
    link_directories(${CMAKE_SOURCE_DIR}/External/Glfw/lib/macos)
    TARGET_LINK_LIBRARIES(pt ${CMAKE_SOURCE_DIR}/External/Glfw/lib/macos/libvulkan.1.dylib)
    TARGET_LINK_LIBRARIES(pt ${CMAKE_SOURCE_DIR}/External/Glfw/lib/macos/libvulkan.1.1.108.dylib)
    TARGET_LINK_LIBRARIES(pt ${CMAKE_SOURCE_DIR}/External/Glfw/lib/macos/libglfw.3.dylib)
    MESSAGE(STATUS "mac os")
else ()
    MESSAGE(STATUS "other platform")
endif ()
