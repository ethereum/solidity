# Install internal libraries.
#
# Installation will install the internal libraries, headers AND dependencies to the configured locations
# After installation, the libraries can be linked against using `find_package(Solidity)`.
# Note that this feature is expected to work, but unsupported.

# Installing internal libraries
install(TARGETS evmasm DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS langutil DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS smtutil DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS libsolc DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS solidity DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS solutil DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS yul DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# Install internal headers
foreach(ETH_COMPONENT IN LISTS ETH_COMPONENTS)
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/${ETH_COMPONENT}
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          PATTERN "*.cpp" EXCLUDE
          PATTERN "CMakeLists.txt" EXCLUDE
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
endforeach()

# install deps headers
foreach(DEP_DIR concepts json meta range std)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/deps/include/${DEP_DIR}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "*.cpp" EXCLUDE
    PATTERN "*.txt" EXCLUDE
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
endforeach()

install(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/fmtlib-src/include/fmt
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PATTERN "*.cpp" EXCLUDE
  PATTERN "*.txt" EXCLUDE
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)