cmake_minimum_required(VERSION 3.1.0)
project(ipc_handlers VERSION 0.0.1 LANGUAGES C CXX)

option(BUILD_TESTING "Build tests" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


if(BUILD_TESTING)
  enable_testing()
  find_package(GTest REQUIRED)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  #add_compile_options(-Wall -Wextra -Werror=conversion -Werror=unused-but-set-variable -Werror=return-type)
endif()


set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /usr/local)

find_package(timespec_chrono_conversions REQUIRED)

set(SOURCES
  src/shm_handler.cpp
)

add_library(
  ${CMAKE_PROJECT_NAME}
  STATIC
  ${SOURCES}
)
target_compile_features(
  ${CMAKE_PROJECT_NAME}
  PUBLIC cxx_std_23
)
target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PUBLIC
  ${timespec_chrono_conversions_INCLUDE_DIRS}/timespec_chrono_conversions
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:include/>
)
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  rt
)

set(GENERAL_INSTALL_DIR /usr/local)
set(INCLUDE_INSTALL_DIR ${GENERAL_INSTALL_DIR}/include)
set(LIBS_INSTALL_DIR ${GENERAL_INSTALL_DIR}/lib) 

install(
  TARGETS ${CMAKE_PROJECT_NAME}
  ARCHIVE DESTINATION ${LIBS_INSTALL_DIR}/${CMAKE_PROJECT_NAME}
  LIBRARY DESTINATION ${LIBS_INSTALL_DIR}/${CMAKE_PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION ${INCLUDE_INSTALL_DIR}/${CMAKE_PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR}/${CMAKE_PROJECT_NAME})
set(LIB_INSTALL_DIR ${LIBS_INSTALL_DIR}/${CMAKE_PROJECT_NAME})
set(LIBS ${LIB_INSTALL_DIR}/${CMAKE_PROJECT_NAME})

configure_package_config_file(
${CMAKE_PROJECT_NAME}.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION /usr/local/lib/cmake/${CMAKE_PROJECT_NAME}
  PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR LIBS
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  VERSION 1.0.0
  COMPATIBILITY AnyNewerVersion
)

install(
    TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}Targets
)

install(
    EXPORT ${CMAKE_PROJECT_NAME}Targets
    FILE ${CMAKE_PROJECT_NAME}Targets.cmake
    DESTINATION /usr/local/lib/cmake/${CMAKE_PROJECT_NAME}
    
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  DESTINATION /usr/local/lib/cmake/${CMAKE_PROJECT_NAME})

if(BUILD_TESTING)
  
  add_executable(
    shm_handler_producer_test
    test/shm_handler_test/shm_handler_producer.cpp
  )
  target_compile_features(shm_handler_producer_test
  PRIVATE cxx_std_23
  )
  
  target_link_libraries(
    shm_handler_producer_test
    ${CMAKE_PROJECT_NAME}
  )

  add_executable(
    shm_handler_consumer_test
    test/shm_handler_test/shm_handler_consumer.cpp
  )
  target_compile_features(shm_handler_consumer_test
  PRIVATE cxx_std_23
  )
  target_link_libraries(
    shm_handler_consumer_test
    ${CMAKE_PROJECT_NAME}
  )

  install(
    TARGETS shm_handler_producer_test shm_handler_consumer_test
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
  )

endif()