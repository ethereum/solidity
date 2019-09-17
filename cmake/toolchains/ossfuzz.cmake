# Inherit default options
include("${CMAKE_CURRENT_LIST_DIR}/default.cmake")
# Disable CVC4.
set(USE_CVC4 OFF CACHE BOOL "Disable CVC4" FORCE)
# Enable fuzzers
set(OSSFUZZ ON CACHE BOOL "Enable fuzzer build" FORCE)
set(LIB_FUZZING_ENGINE $ENV{LIB_FUZZING_ENGINE} CACHE STRING "Use fuzzer back-end defined by environment variable" FORCE)
# Link statically against boost libraries
set(BOOST_FOUND ON CACHE BOOL "" FORCE)
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link against static Boost libraries" FORCE)
set(Boost_USE_STATIC_RUNTIME ON CACHE BOOL "Link against static Boost runtime library" FORCE)
