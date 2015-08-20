cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(. SRC_LIST)

set(EXECUTABLE solidity)

file(GLOB HEADERS "*.h") 

include_directories(BEFORE ..)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})
add_dependencies(${EXECUTABLE} BuildInfo.h)

target_link_libraries(${EXECUTABLE} ${JSONCPP_LIBRARIES})
target_link_libraries(${EXECUTABLE} ${Boost_LIBRARIES})
target_link_libraries(${EXECUTABLE} ${ETH_DEVCORE_LIBRARY})
target_link_libraries(${EXECUTABLE} ${ETH_EVMCORE_LIBRARY})
target_link_libraries(${EXECUTABLE} ${ETH_EVMASM_LIBRARY})
target_link_libraries(${EXECUTABLE} ${ETH_DEVCRYPTO_LIBRARY})

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )

