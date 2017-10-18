# Until we have a clear separation, libjulia has to be included here
file(GLOB_RECURSE sources "*.cpp" "../libjulia/*.cpp")
file(GLOB_RECURSE headers "*.h" "../libjulia/*.h")

find_package(Z3 QUIET)
if (${Z3_FOUND})
  include_directories(${Z3_INCLUDE_DIR})
  add_definitions(-DHAVE_Z3)
  message("Z3 SMT solver found. This enables optional SMT checking.")
else()
  message("Z3 SMT solver NOT found. Optional SMT checking will not be available. Please install Z3 if it is desired.")
  list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/formal/Z3Interface.cpp")
endif()

add_library(solidity ${sources} ${headers})
target_link_libraries(solidity PUBLIC evmasm devcore)

if (${Z3_FOUND})
  target_link_libraries(solidity PUBLIC ${Z3_LIBRARY})
endif()