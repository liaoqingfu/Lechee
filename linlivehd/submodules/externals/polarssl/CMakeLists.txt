cmake_minimum_required(VERSION 2.6)
project(POLARSSL C)

enable_testing()

string(REGEX MATCH "clang" CMAKE_COMPILER_IS_CLANG "${CMAKE_C_COMPILER}")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra -W -Wdeclaration-after-statement")
  set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
  set(CMAKE_C_FLAGS_COVERAGE "-g3 -O0 -fprofile-arcs -ftest-coverage -lgcov")
  set(CMAKE_C_FLAGS_CHECK "${CMAKE_C_FLAGS} -Werror -Wlogical-op -Wwrite-strings")
  set(CMAKE_C_FLAGS_CHECKFULL "${CMAKE_C_FLAGS_CHECK} -Wcast-qual")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CMAKE_COMPILER_IS_CLANG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra -W -Wdeclaration-after-statement")
  set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
  set(CMAKE_C_FLAGS_CHECK "${CMAKE_C_FLAGS} -Werror -Wpointer-arith -Wwrite-strings -Wdocumentation")
endif(CMAKE_COMPILER_IS_CLANG)

set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
    CACHE STRING "Choose the type of build: None Debug Release Coverage Check CheckFull"
    FORCE)
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_SHARED_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(CMAKE_BUILD_TYPE STREQUAL "Coverage")

option(USE_PKCS11_HELPER_LIBRARY "Build PolarSSL with the pkcs11-helper library." OFF)

option(ENABLE_ZLIB_SUPPORT "Build PolarSSL with zlib library." OFF)

option(COMPILE_PROGRAMS "Build PolarSSL programs." OFF)

option(COMPILE_TESTS "Build PolarSSL tests." OFF)

option(GENERATE_APIDOC "Generate API documentation of PolarSSL." OFF)

if(LIB_INSTALL_DIR)
else()
set(LIB_INSTALL_DIR lib)
endif()

include_directories(include/)

if(ENABLE_ZLIB_SUPPORT)
  find_package(ZLIB)

  if(ZLIB_FOUND)
    include_directories(ZLIB_INCLUDE_DIR)
  endif(ZLIB_FOUND)
endif(ENABLE_ZLIB_SUPPORT)

add_subdirectory(library)
add_subdirectory(include)

if(CMAKE_COMPILER_IS_GNUCC)
if(COMPILE_TESTS)
  add_subdirectory(tests)
endif(COMPILE_TESTS)
endif(CMAKE_COMPILER_IS_GNUCC)
if(CMAKE_COMPILER_IS_CLANG)
  add_subdirectory(tests)
endif(CMAKE_COMPILER_IS_CLANG)

if(COMPILE_PROGRAMS)
add_subdirectory(programs)
endif()

if(GENERATE_APIDOC)
	ADD_CUSTOM_TARGET(apidoc
		COMMAND doxygen doxygen/polarssl.doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif(GENERATE_APIDOC)

ADD_CUSTOM_TARGET(memcheck
    COMMAND ctest -O memcheck.log -D ExperimentalMemCheck
    COMMAND tail -n1 memcheck.log | grep 'Memory checking results:' > /dev/null
    COMMAND rm -f memcheck.log
    )

if(INSTALL_COMPONENT_IN_POSTBUILD)
	add_install_target(INSTALL_libpolarssl COMP_libpolarssl libpolarssl)
endif()
