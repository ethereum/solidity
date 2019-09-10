#------------------------------------------------------------------------------
# EthCompilerSettings.cmake
#
# CMake file for cpp-ethereum project which specifies our compiler settings
# for each supported platform and build configuration.
#
# The documentation for cpp-ethereum is hosted at http://cpp-ethereum.org
#
# Copyright (c) 2014-2016 cpp-ethereum contributors.
#------------------------------------------------------------------------------

# Clang seeks to be command-line compatible with GCC as much as possible, so
# most of our compiler settings are common between GCC and Clang.
#
# These settings then end up spanning all POSIX platforms (Linux, OS X, BSD, etc)

include(EthCheckCXXCompilerFlag)

eth_add_cxx_compiler_flag_if_supported(-fstack-protector-strong have_stack_protector_strong_support)
if(NOT have_stack_protector_strong_support)
	eth_add_cxx_compiler_flag_if_supported(-fstack-protector)
endif()

eth_add_cxx_compiler_flag_if_supported(-Wimplicit-fallthrough)

if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
	# Enables all the warnings about constructions that some users consider questionable,
	# and that are easy to avoid.  Also enable some extra warning flags that are not
	# enabled by -Wall.   Finally, treat at warnings-as-errors, which forces developers
	# to fix warnings as they arise, so they don't accumulate "to be fixed later".
	add_compile_options(-Wall)
	add_compile_options(-Wextra)
	add_compile_options(-Werror)

	# Configuration-specific compiler settings.
	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g3 -DETH_DEBUG")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3")

	# Additional GCC-specific compiler settings.
	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

		# Check that we've got GCC 4.7 or newer.
		execute_process(
			COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
			message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
		endif ()

	# Additional Clang-specific compiler settings.
	elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
			# Set stack size to 32MB - by default Apple's clang defines a stack size of 8MB.
			# Normally 16MB is enough to run all tests, but it will exceed the stack, if -DSANITIZE=address is used.
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-stack_size -Wl,0x2000000")

			# Boost libraries use visibility=hidden to reduce unnecessary DWARF entries.
			# Unless we match visibility, ld will give a warning message like:
			#   ld: warning: direct access in function 'boost::filesystem... from file ...
			#   means the weak symbol cannot be overridden at runtime. This was likely caused by different translation units being compiled with different visibility settings.
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
		endif()

		# Some Linux-specific Clang settings.  We don't want these for OS X.
		if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

			# TODO - Is this even necessary?  Why?
			# See http://stackoverflow.com/questions/19774778/when-is-it-necessary-to-use-use-the-flag-stdlib-libstdc.
			add_compile_options(-stdlib=libstdc++)

			# Tell Boost that we're using Clang's libc++.   Not sure exactly why we need to do.
			add_definitions(-DBOOST_ASIO_HAS_CLANG_LIBCXX)

			# Use fancy colors in the compiler diagnostics
			add_compile_options(-fcolor-diagnostics)

			# See "How to silence unused command line argument error with clang without disabling it?"
			# When using -Werror with clang, it transforms "warning: argument unused during compilation" messages
			# into errors, which makes sense.
			# http://stackoverflow.com/questions/21617158/how-to-silence-unused-command-line-argument-error-with-clang-without-disabling-i
			add_compile_options(-Qunused-arguments)
		elseif(EMSCRIPTEN)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --memory-init-file 0")
			# Leave only exported symbols as public and aggressively remove others
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections -Wl,--gc-sections -fvisibility=hidden")
			# Optimisation level
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
			# Re-enable exception catching (optimisations above -O1 disable it)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
			# Remove any code related to exit (such as atexit)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
			# Remove any code related to filesystem access
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=1")
			# Remove variables even if it needs to be duplicated (can improve speed at the cost of size)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s AGGRESSIVE_VARIABLE_ELIMINATION=1")
			# Allow memory growth, but disable some optimisations
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
			# Disable eval()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_DYNAMIC_EXECUTION=1")
			# Disable greedy exception catcher
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NODEJS_CATCH_EXIT=0")
			# Abort if linking results in any undefined symbols
			# Note: this is on by default in the CMake Emscripten module which we aren't using
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ERROR_ON_UNDEFINED_SYMBOLS=1")
			# Disallow deprecated emscripten build options.
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s STRICT=1")
			# Export the Emscripten-generated auxiliary methods which are needed by solc-js.
			# Which methods of libsolc itself are exported is specified in libsolc/CMakeLists.txt.
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap','addFunction','removeFunction','Pointer_stringify','lengthBytesUTF8','_malloc','stringToUTF8','setValue']")
			# Do not build as a WebAssembly target - we need an asm.js output.
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=0")
		endif()
	endif()

# The major alternative compiler to GCC/Clang is Microsoft's Visual C++ compiler, only available on Windows.
elseif (DEFINED MSVC)

    add_compile_options(/MP)						# enable parallel compilation
	add_compile_options(/EHsc)						# specify Exception Handling Model in msvc
	add_compile_options(/WX)						# enable warnings-as-errors
	add_compile_options(/wd4068)					# disable unknown pragma warning (4068)
	add_compile_options(/wd4996)					# disable unsafe function warning (4996)
	add_compile_options(/wd4503)					# disable decorated name length exceeded, name was truncated (4503)
	add_compile_options(/wd4267)					# disable conversion from 'size_t' to 'type', possible loss of data (4267)
	add_compile_options(/wd4180)					# disable qualifier applied to function type has no meaning; ignored (4180)
	add_compile_options(/wd4290)					# disable C++ exception specification ignored except to indicate a function is not __declspec(nothrow) (4290)
	add_compile_options(/wd4244)					# disable conversion from 'type1' to 'type2', possible loss of data (4244)
	add_compile_options(/wd4800)					# disable forcing value to bool 'true' or 'false' (performance warning) (4800)
	add_compile_options(-D_WIN32_WINNT=0x0600)		# declare Windows Vista API requirement
	add_compile_options(-DNOMINMAX)					# undefine windows.h MAX && MIN macros cause it cause conflicts with std::min && std::max functions
	add_compile_options(/utf-8)					# enable utf-8 encoding (solves warning 4819)

	# disable empty object file warning
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4221")
	# warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/SAFESEH' specification
	# warning LNK4099: pdb was not found with lib
	# stack size 16MB
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099,4075 /STACK:16777216")

# If you don't have GCC, Clang or VC++ then you are on your own.  Good luck!
else ()
	message(WARNING "Your compiler is not tested, if you run into any issues, we'd welcome any patches.")
endif ()

if (SANITIZE)
	# Perform case-insensitive string compare
	string(TOLOWER "${SANITIZE}" san)
	# -fno-omit-frame-pointer gives more informative stack trace in case of an error
	# -fsanitize-address-use-after-scope throws an error when a variable is used beyond its scope
	if (san STREQUAL "address")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope")
	endif()
endif()

# Code coverage support.
# Copied from Cable:
# https://github.com/ethereum/cable/blob/v0.2.4/CableCompilerSettings.cmake#L118-L132
option(COVERAGE "Build with code coverage support" OFF)
if(COVERAGE)
	# Set the linker flags first, they are required to properly test the compiler flag.
	set(CMAKE_SHARED_LINKER_FLAGS "--coverage ${CMAKE_SHARED_LINKER_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "--coverage ${CMAKE_EXE_LINKER_FLAGS}")

	set(CMAKE_REQUIRED_LIBRARIES "--coverage ${CMAKE_REQUIRED_LIBRARIES}")
	check_cxx_compiler_flag(--coverage have_coverage)
	string(REPLACE "--coverage " "" CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	if(NOT have_coverage)
		message(FATAL_ERROR "Coverage not supported")
	endif()
	add_compile_options(-g --coverage)
endif()

# SMT Solvers integration
option(USE_Z3 "Allow compiling with Z3 SMT solver integration" ON)
option(USE_CVC4 "Allow compiling with CVC4 SMT solver integration" ON)

if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
	option(USE_LD_GOLD "Use GNU gold linker" ON)
	if (USE_LD_GOLD)
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
		if ("${LD_VERSION}" MATCHES "GNU gold")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
		endif ()
	endif ()
endif ()
