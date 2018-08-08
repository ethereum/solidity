# Until we have a clear separation, libjulia has to be included here
file(GLOB_RECURSE sources "*.cpp" "../libjulia/*.cpp")
file(GLOB_RECURSE headers "*.h" "../libjulia/*.h")

find_package(Z3 QUIET)
if (${Z3_FOUND})
  include_directories(${Z3_INCLUDE_DIR})
  add_definitions(-DHAVE_Z3)
  message("Z3 SMT solver found. This enables optional SMT checking with Z3.")
else()
  list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/formal/Z3Interface.cpp")
endif()

find_package(GMP QUIET)
find_package(CVC4 QUIET)
if (${CVC4_FOUND})
  if (${GMP_FOUND})
    include_directories(${CVC4_INCLUDE_DIR})
    add_definitions(-DHAVE_CVC4)
    message("CVC4 SMT solver and GMP found. This enables optional SMT checking with CVC4.")
  else()
    message("CVC4 SMT solver found but its dependency GMP was NOT found. Optional SMT checking with CVC4 will not be available. Please install GMP if it is desired.")
    list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/formal/CVC4Interface.cpp")
  endif()
else()
  list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/formal/CVC4Interface.cpp")
endif()

if (NOT (${Z3_FOUND} OR ${CVC4_FOUND}))
  message("No SMT solver found (or it has been forcefully disabled). Optional SMT checking will not be available.\
  \nPlease install Z3 or CVC4 or remove the option disabling them (USE_Z3, USE_CVC4).")
endif()

add_library(solidity ${sources} ${headers})
target_link_libraries(solidity PUBLIC evmasm devcore ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

if (${Z3_FOUND})
  target_link_libraries(solidity PUBLIC ${Z3_LIBRARY})
endif()

if (${CVC4_FOUND} AND ${GMP_FOUND})
  target_link_libraries(solidity PUBLIC ${CVC4_LIBRARIES})
  target_link_libraries(solidity PUBLIC ${GMP_LIBRARY})
endif()
