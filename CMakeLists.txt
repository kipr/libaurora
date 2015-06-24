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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

################################################################################
#   Resolve external library directories                                       #
################################################################################
if(UNIX)
  find_package(PNG REQUIRED)
elseif(WIN32)
  find_file(PNG_DLL libpng16.dll PATHS "${PROJECT_SOURCE_DIR}/../libpng/build/install/bin" NO_DEFAULT_PATH)
  find_path(PNG_INCLUDE_DIR png.h PATHS "${PROJECT_SOURCE_DIR}/../libpng/build/install/include" NO_DEFAULT_PATH)
  find_library(PNG_LIBRARY libpng16.lib PATHS "${PROJECT_SOURCE_DIR}/../libpng/build/install/lib" NO_DEFAULT_PATH)
endif()

if(WIN32)
  find_file(DAYLITE_DLL daylite.dll PATHS "${PROJECT_SOURCE_DIR}/../daylite/build/install/bin" NO_DEFAULT_PATH)
  find_path(DAYLITE_INCLUDE_DIR daylite PATHS "${PROJECT_SOURCE_DIR}/../daylite/build/install/include" NO_DEFAULT_PATH)
  find_library(DAYLITE_LIBRARY daylite.lib PATHS "${PROJECT_SOURCE_DIR}/../daylite/build/install/lib" NO_DEFAULT_PATH)
endif()

if(WIN32)
  find_file(LIBBSON_DLL libbson-1.0.dll PATHS "${PROJECT_SOURCE_DIR}/../libbson/build/install/bin" NO_DEFAULT_PATH)
  find_path(LIBBSON_INCLUDE_DIR bson.h PATHS "${PROJECT_SOURCE_DIR}/../libbson/build/install/include/libbson-1.0" NO_DEFAULT_PATH)
  find_library(LIBBSON_LIBRARY bson-1.0.lib PATHS "${PROJECT_SOURCE_DIR}/../libbson/build/install/lib" NO_DEFAULT_PATH)
endif()

################################################################################
#   Include/link directories                                                   #
################################################################################

include_directories(${INCLUDE} ${PNG_INCLUDE_DIR} ${DAYLITE_INCLUDE_DIR} ${LIBBSON_INCLUDE_DIR})

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -g -std=c++11 -stdlib=libc++ -v")
elseif(WIN32)
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

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
  install(TARGETS daylite LIBRARY DESTINATION lib)
endif()
