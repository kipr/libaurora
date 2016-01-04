################################################################################
#   libaurora CMake Build File                                                 #
################################################################################

project(libaurora)

cmake_minimum_required(VERSION 2.8.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") 
find_package(Daylite REQUIRED)
find_package(BsonBind REQUIRED)

set(INCLUDE ${libaurora_SOURCE_DIR}/include)
set(SRC ${libaurora_SOURCE_DIR}/src)

add_bson_binds(BSON_BIND_INCLUDES
               msg/aurora_frame.bsonbind
               msg/aurora_mouse.bsonbind
               msg/aurora_key.bsonbind)

SET(BSON_BIND_INCLUDES_TMP "")
FOREACH(f ${BSON_BIND_INCLUDES})
  LIST(APPEND BSON_BIND_INCLUDES_TMP "${CMAKE_CURRENT_BINARY_DIR}/${f}")
ENDFOREACH(f)
SET(BSON_BIND_INCLUDES ${BSON_BIND_INCLUDES_TMP})

file(GLOB INCLUDES ${INCLUDE}/kipr/*.h ${INCLUDE}/kipr/*.hpp)
file(GLOB SOURCES ${SRC}/*)

if(NOT WIN32)
  add_definitions("-std=c++11")
endif()

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

################################################################################
#   Include/link directories                                                   #
################################################################################

include_directories(${INCLUDE}
                    ${PNG_INCLUDE_DIR}
                    ${DAYLITE_INCLUDE_DIR}
                    ${BSON_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  set(LIBRARIES ws2_32.lib Mswsock.lib AdvApi32.lib)
endif()

################################################################################
#   Targets                                                                    #
################################################################################
add_library(aurora SHARED ${SOURCES} ${INCLUDES} ${BSON_BIND_INCLUDES})
target_link_libraries(aurora ${PNG_LIBRARY} ${DAYLITE_LIBRARY} ${LIBRARIES} ${BSON_LIBRARY})

# for the dllimport/dllexport switch
set_target_properties(aurora PROPERTIES COMPILE_FLAGS -DBUILD_AURORA_DLL)

################################################################################
#   Install                                                                    #
################################################################################

install(FILES ${INCLUDES} DESTINATION include/aurora)
install(FILES ${BSON_BIND_INCLUDES} DESTINATION include/aurora)

if(WIN32)
  install(TARGETS aurora
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin)
else()
  install(TARGETS aurora LIBRARY DESTINATION lib)
endif()
