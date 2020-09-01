# Require C++17.
set(CMAKE_CXX_STANDARD 17) # This requires at least CMake 3.8 to accept this C++17 flag.
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_TOOLCHAIN_FILE)
  # Use default toolchain file if none is provided.
  set(
      CMAKE_TOOLCHAIN_FILE
      "${CMAKE_CURRENT_LIST_DIR}/toolchains/default.cmake"
      CACHE FILEPATH "The CMake toolchain file"
  )
endif()
