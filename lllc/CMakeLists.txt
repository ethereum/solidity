aux_source_directory(. SRC_LIST)

set(EXECUTABLE lllc)

file(GLOB HEADERS "*.h")
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

eth_use(${EXECUTABLE} REQUIRED Eth::lll Dev::buildinfo)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

