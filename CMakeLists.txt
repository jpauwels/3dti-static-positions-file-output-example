cmake_minimum_required(VERSION 3.18.2)
project(3dti LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


include_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_Toolkit")
include_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager")
include_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/boost_circular_buffer")
include_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/cereal/include")
include_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/sofacoustics/libsofa/src")
include_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/sofacoustics/libsofa/dependencies/include")

link_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/sofacoustics/libsofa/lib")
if(WIN32)
    set(PLATFORM "win")
elseif(APPLE)
    set(PLATFORM "macos")
else()
    set(PLATFORM "linux")
endif()
link_directories("${PROJECT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/sofacoustics/libsofa/dependencies/lib/${PLATFORM}")

link_libraries(sofa curl hdf5 hdf5_hl netcdf netcdf_c++4 z)

file(GLOB_RECURSE SOURCES
3dti_AudioToolkit/3dti_Toolkit/BinauralSpatializer/*.cpp
3dti_AudioToolkit/3dti_Toolkit/Common/*.cpp
3dti_AudioToolkit/3dti_ResourceManager/HRTF/*.cpp
3dti_AudioToolkit/3dti_ResourceManager/ILD/*.cpp
3dti_AudioToolkit/3dti_ResourceManager/BRIR/*.cpp
)

file(GLOB_RECURSE HEADERS
3dti_AudioToolkit/3dti_Toolkit/BinauralSpatializer/*.h
3dti_AudioToolkit/3dti_Toolkit/Common/*.h
3dti_AudioToolkit/3dti_ResourceManager/HRTF/*.h
3dti_AudioToolkit/3dti_ResourceManager/ILD/*.h
3dti_AudioToolkit/3dti_ResourceManager/BRIR/*.h
)

add_executable(3dti_example main.cpp ${SOURCES} ${HEADERS})
target_include_directories(3dti_example PRIVATE "${PROJECT_SOURCE_DIR}/AudioFile")
