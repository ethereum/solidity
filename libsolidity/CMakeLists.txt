set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(analysis SRC_LIST)
aux_source_directory(ast SRC_LIST)
aux_source_directory(codegen SRC_LIST)
aux_source_directory(formal SRC_LIST)
aux_source_directory(interface SRC_LIST)
aux_source_directory(parsing SRC_LIST)

set(EXECUTABLE solidity)

file(GLOB HEADERS "*/*.h") 

include_directories(BEFORE ..)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

eth_use(${EXECUTABLE} REQUIRED Dev::devcore Eth::evmcore Eth::evmasm)

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )

