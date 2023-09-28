# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
project(MightyL)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# compiler must be 17
set(CMAKE_CXX_STANDARD 17)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# Specify the version of the antlr4 library needed for this project.
# By default the latest version of antlr4 will be used.  You can specify a
# specific, stable version by setting a repository tag value or a link
# to a zip file containing the libary source.
# set(ANTLR4_TAG 4.13.1)
# set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/4.13.1.zip)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE /home/hsimho/antlr-4.13.1-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(MitlGrammar Mitl.g4 VISITOR
             PACKAGE antlrcpptest)

# include generated files in project environment
include_directories(${ANTLR_MitlGrammar_OUTPUT_DIR})

set(EXTERNAL_INSTALL_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external")

add_subdirectory(external/buddy buddy)

ExternalProject_add(monitaal
    GIT_REPOSITORY git@github.com:DEIS-Tools/MoniTAal.git
    GIT_TAG main
    CMAKE_ARGS (
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}/monitaal -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/monitaal/include/monitaal)
link_directories(${EXTERNAL_INSTALL_LOCATION}/monitaal/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/monitaal-prefix/src/monitaal-build/external/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/monitaal-prefix/src/monitaal-build/external/lib)


# add generated grammar to binary target
add_executable(mightyl main.cpp
               MitlCheckNNFVisitor.cpp
               MitlToNNFVisitor.cpp
               MitlAtomNumberingVisitor.cpp
               MitlCollectTemporalVisitor.cpp
               MitlGetBDDVisitor.cpp
               ${ANTLR_MitlGrammar_CXX_OUTPUTS})


target_link_libraries(mightyl antlr4_static buddy MoniTAal pugixml pardibaal)


if (CMAKE_HOST_UNIX)
  # Make sure to link against threads (pthreads) library in order to be able to
  # make use of std::call_once in the code without producing runtime errors
  # (see also https://github.com/antlr/antlr4/issues/3708 and/or https://stackoverflow.com/q/51584960).
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  if (TARGET mightyl)
    target_link_libraries(mightyl Threads::Threads)
  endif()

endif()

