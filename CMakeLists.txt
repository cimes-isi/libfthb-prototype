cmake_minimum_required(VERSION 2.8.5)

project(fthb)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")

include_directories(${PROJECT_SOURCE_DIR}/inc)

if(CMAKE_HOST_UNIX)
  include(GNUInstallDirs)
else()
  set(CMAKE_INSTALL_LIBDIR lib)
  set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

find_library(LIBRT rt)
find_package(Threads REQUIRED)

enable_testing()


# Libraries

macro(LIB_CONFIGURE name)
  # Link against librt if needed
  if(LIBRT)
    target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT} ${LIBRT})
  endif()
  # Set shared object version properties
  if(BUILD_SHARED_LIBS)
    set_target_properties(${name} PROPERTIES VERSION ${PROJECT_VERSION}
                                             SOVERSION ${VERSION_MAJOR})
  endif()
  # Add tests
  add_executable(${name}-test test/fthb-test.c)
  target_link_libraries(${name}-test ${name})
  add_test(${name}-test ${EXECUTABLE_OUTPUT_PATH}/${name}-test)
  # Other executable
  add_executable(${name}-producer-consumer src/fthb-producer-consumer.c)
  target_link_libraries(${name}-producer-consumer ${name})
  # Install lib
  install(TARGETS ${name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endmacro(LIB_CONFIGURE)

add_library(fthb_shmem_posix src/fthb_shmem_posix.c)
LIB_CONFIGURE(fthb_shmem_posix)

add_library(fthb_shmem_sysv src/fthb_shmem_sysv.c)
LIB_CONFIGURE(fthb_shmem_sysv)


# Other Install/Uninstall

install(FILES inc/fthb.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  @ONLY
)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
