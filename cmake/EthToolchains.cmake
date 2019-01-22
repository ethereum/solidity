if(NOT CMAKE_TOOLCHAIN_FILE)
  # Use default toolchain file if none is provided.
  set(
      CMAKE_TOOLCHAIN_FILE
      "${CMAKE_CURRENT_LIST_DIR}/toolchains/default.cmake"
      CACHE FILEPATH "The CMake toolchain file"
  )
endif()
