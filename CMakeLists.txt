################################################################################
#   libaurora CMake Build File                                                 #
################################################################################

project(libaurora)

cmake_minimum_required(VERSION 2.8.0)

set(INCLUDE ${libaurora_SOURCE_DIR}/include)
set(SRC ${libaurora_SOURCE_DIR}/src)

file(GLOB INCLUDES ${INCLUDE}/kipr/*.h ${INCLUDE}/kipr/*.hpp)
file(GLOB SOURCES ${SRC}/*)

if(NOT WIN32)
  set(CXX_FLAGS "-std=c++1y")
endif()

set(CMAKE_C_FLAGS ${C_FLAGS})
set(CMAKE_CXX_FLAGS ${CXX_FLAGS})

################################################################################
#   Resolve external library directories                                       #
################################################################################
if(APPLE)
  find_path(PNG_INCLUDE_DIR png.h)
  find_library(PNG_LIBRARY png)
elseif(WIN32)
  find_file(PNG_DLL libpng16.dll PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/bin")
  find_path(PNG_INCLUDE_DIR png.h PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/include")
  find_library(PNG_LIBRARY libpng16.lib PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/lib")
else()
  find_package(PNG REQUIRED)
endif()

if(WIN32)
  find_file(DAYLITE_DLL daylite.dll PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/bin")
  find_path(DAYLITE_INCLUDE_DIR daylite PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/include")
  find_library(DAYLITE_LIBRARY daylite.lib PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/lib")
elseif(APPLE)
  find_path(DAYLITE_INCLUDE_DIR daylite)
  find_library(DAYLITE_LIBRARY daylite)
endif()

if(WIN32)
  find_file(LIBBSON_DLL libbson-1.0.dll PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/bin")
  find_path(LIBBSON_INCLUDE_DIR bson.h PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/include/libbson-1.0")
  find_library(LIBBSON_LIBRARY bson-1.0.lib PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/lib")
else()
  find_path(LIBBSON_INCLUDE_DIR bson.h)
  find_library(LIBBSON_LIBRARY bson-1.0)
endif()

################################################################################
#   Include/link directories                                                   #
################################################################################

include_directories(${INCLUDE} ${PNG_INCLUDE_DIR} ${DAYLITE_INCLUDE_DIR} ${LIBBSON_INCLUDE_DIR})

if(WIN32)
  set(LIBRARIES ws2_32.lib Mswsock.lib AdvApi32.lib)
endif()

################################################################################
#   Targets                                                                    #
################################################################################
add_library(aurora SHARED ${SOURCES} ${INCLUDES})
target_link_libraries(aurora ${PNG_LIBRARY} ${DAYLITE_LIBRARY} ${LIBRARIES} ${LIBBSON_LIBRARY})

# for the dllimport/dllexport switch
set_target_properties(aurora PROPERTIES COMPILE_FLAGS -DBUILD_AURORA_DLL)

################################################################################
#   Install                                                                    #
################################################################################

install(FILES ${INCLUDES} DESTINATION include/kipr)

if(WIN32)
  install(TARGETS aurora
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin)
else()
  install(TARGETS aurora LIBRARY DESTINATION lib)
endif()
