# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
project(MightyPPL)

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
set(ANTLR_EXECUTABLE /home/user/antlr-4.13.1-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(MitlGrammar Mitl.g4 VISITOR
            PACKAGE mightypplcpp)
         #antlr_target(SampleGrammarParser TParser.g4 PARSER
         #             PACKAGE antlrcpptest
         #             DEPENDS_ANTLR SampleGrammarLexer
         #             COMPILE_FLAGS -lib ${ANTLR_SampleGrammarLexer_OUTPUT_DIR})

# include generated files in project environment
include_directories(${ANTLR_MitlGrammar_OUTPUT_DIR})
#include_directories(${ANTLR_SampleGrammarParser_OUTPUT_DIR})

set(EXTERNAL_INSTALL_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external")

add_subdirectory (${EXTERNAL_INSTALL_LOCATION}/buddy buddy)


# add generated grammar to demo binary target
add_executable(demo main.cpp
               MitlCheckNNFVisitor.cpp
               MitlToNNFVisitor.cpp
               MitlAtomNumberingVisitor.cpp
               MitlCollectTemporalVisitor.cpp
               MitlGetBDDVisitor.cpp
               MightyPPL.cpp
               ${ANTLR_MitlGrammar_CXX_OUTPUTS})


target_link_libraries(demo antlr4_static buddy)


if (CMAKE_HOST_UNIX)
  # Make sure to link against threads (pthreads) library in order to be able to
  # make use of std::call_once in the code without producing runtime errors
  # (see also https://github.com/antlr/antlr4/issues/3708 and/or https://stackoverflow.com/q/51584960).
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  if (TARGET demo)
    target_link_libraries(demo Threads::Threads)
    message(STATUS "demo Linked with Threads!")
  endif()

endif()

