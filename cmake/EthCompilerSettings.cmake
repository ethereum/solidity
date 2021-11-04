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

if(NOT EMSCRIPTEN)
	eth_add_cxx_compiler_flag_if_supported(-fstack-protector-strong have_stack_protector_strong_support)
	if(NOT have_stack_protector_strong_support)
		eth_add_cxx_compiler_flag_if_supported(-fstack-protector)
	endif()
endif()

eth_add_cxx_compiler_flag_if_supported(-Wimplicit-fallthrough)

# Prevent the path of the source directory from ending up in the binary via __FILE__ macros.
eth_add_cxx_compiler_flag_if_supported("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/solidity")

# -Wpessimizing-move warns when a call to std::move would prevent copy elision
# if the argument was not wrapped in a call.  This happens when moving a local
# variable in a return statement when the variable is the same type as the
# return type or using a move to create a new object from a temporary object.
eth_add_cxx_compiler_flag_if_supported(-Wpessimizing-move)

# -Wredundant-move warns when an implicit move would already be made, so the
# std::move call is not needed, such as when moving a local variable in a return
# that is different from the return type.
eth_add_cxx_compiler_flag_if_supported(-Wredundant-move)

if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
	# Enables all the warnings about constructions that some users consider questionable,
	# and that are easy to avoid.  Also enable some extra warning flags that are not
	# enabled by -Wall.   Finally, treat at warnings-as-errors, which forces developers
	# to fix warnings as they arise, so they don't accumulate "to be fixed later".
	add_compile_options(-Wall)
	add_compile_options(-Wextra)
	add_compile_options(-Werror)
	add_compile_options(-pedantic)
	add_compile_options(-Wmissing-declarations)
	add_compile_options(-Wno-unknown-pragmas)
	add_compile_options(-Wimplicit-fallthrough)
	add_compile_options(-Wsign-conversion)
	add_compile_options(-Wconversion)

	eth_add_cxx_compiler_flag_if_supported(
		$<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
	)
	eth_add_cxx_compiler_flag_if_supported(-Wfinal-dtor-non-final-class)
	eth_add_cxx_compiler_flag_if_supported(-Wnewline-eof)
	eth_add_cxx_compiler_flag_if_supported(-Wsuggest-destructor-override)
	eth_add_cxx_compiler_flag_if_supported(-Wduplicated-cond)
	eth_add_cxx_compiler_flag_if_supported(-Wduplicate-enum)
	eth_add_cxx_compiler_flag_if_supported(-Wlogical-op)
	eth_add_cxx_compiler_flag_if_supported(-Wno-unknown-attributes)

	# Configuration-specific compiler settings.
	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g3 -DETH_DEBUG")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3")

	# Additional GCC-specific compiler settings.
	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
		# Check that we've got GCC 8.0 or newer.
		if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0))
			message(FATAL_ERROR "${PROJECT_NAME} requires g++ 8.0 or greater.")
		endif ()

		# Use fancy colors in the compiler diagnostics
		add_compile_options(-fdiagnostics-color)

	# Additional Clang-specific compiler settings.
	elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		# Check that we've got clang 7.0 or newer.
		if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0))
			message(FATAL_ERROR "${PROJECT_NAME} requires clang++ 7.0 or greater.")
		endif ()

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
			# Use fancy colors in the compiler diagnostics
			add_compile_options(-fcolor-diagnostics)

			# See "How to silence unused command line argument error with clang without disabling it?"
			# When using -Werror with clang, it transforms "warning: argument unused during compilation" messages
			# into errors, which makes sense.
			# http://stackoverflow.com/questions/21617158/how-to-silence-unused-command-line-argument-error-with-clang-without-disabling-i
			add_compile_options(-Qunused-arguments)
		elseif(EMSCRIPTEN)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --memory-init-file 0")
			# Leave only exported symbols as public and aggressively remove others
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections -fvisibility=hidden")
			# Optimisation level
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
			# Re-enable exception catching (optimisations above -O1 disable it)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
			# Remove any code related to exit (such as atexit)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXIT_RUNTIME=0")
			# Remove any code related to filesystem access
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s FILESYSTEM=0")
			# Allow memory growth, but disable some optimisations
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
			# Disable eval()
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s DYNAMIC_EXECUTION=0")
			# Disable greedy exception catcher
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s NODEJS_CATCH_EXIT=0")
			# Abort if linking results in any undefined symbols
			# Note: this is on by default in the CMake Emscripten module which we aren't using
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ERROR_ON_UNDEFINED_SYMBOLS=1")
			# Disallow deprecated emscripten build options.
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s STRICT=1")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s STRICT=1")
			# Export the Emscripten-generated auxiliary methods which are needed by solc-js.
			# Which methods of libsolc itself are exported is specified in libsolc/CMakeLists.txt.
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap','addFunction','removeFunction','UTF8ToString','lengthBytesUTF8','stringToUTF8','setValue']")
			# Build for webassembly target.
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1")
			# Set webassembly build to synchronous loading.
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM_ASYNC_COMPILATION=0")
			# Output a single js file with the wasm binary embedded as base64 string.
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s SINGLE_FILE=1")
			# Allow new functions to be added to the wasm module via addFunction.
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_TABLE_GROWTH=1")
			# Disable warnings about not being pure asm.js due to memory growth.
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-almost-asm")
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
	add_compile_options(/utf-8)						# enable utf-8 encoding (solves warning 4819)
	add_compile_options(-DBOOST_REGEX_NO_LIB)		# disable automatic boost::regex library selection
	add_compile_options(-D_REGEX_MAX_STACK_COUNT=200000L)	# increase std::regex recursion depth limit
	add_compile_options(/permissive-)				# specify standards conformance mode to the compiler

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
	string(TOLOWER "${SANITIZE}" sanitizer)
	# -fno-omit-frame-pointer gives more informative stack trace in case of an error
	# -fsanitize-address-use-after-scope throws an error when a variable is used beyond its scope
	if (sanitizer STREQUAL "address")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope")
	elseif (sanitizer STREQUAL "undefined")
		# The following flags not used by fuzzer but used by us may create problems, so consider
		# disabling them: alignment, pointer-overflow.
		# The following flag is not used by us to reduce terminal noise
		# i.e., warnings printed on stderr: unsigned-integer-overflow
		# Note: The C++ standard does not officially consider unsigned integer overflows
		# to be undefined behavior since they are implementation independent.
		# Flags are alphabetically sorted and are for clang v10.0
		list(APPEND undefinedSanitizerChecks
			alignment
			array-bounds
			bool
			builtin
			enum
			float-divide-by-zero
			function
			integer-divide-by-zero
			null
			object-size
			pointer-overflow
			return
			returns-nonnull-attribute
			shift
			signed-integer-overflow
			unreachable
			vla-bound
			vptr
		)
		list(JOIN undefinedSanitizerChecks "," sanitizerChecks)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${sanitizerChecks} -fno-sanitize-recover=${sanitizerChecks}")
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
if(UNIX AND NOT APPLE)
	option(USE_Z3_DLOPEN "Dynamically load the Z3 SMT solver instead of linking against it." OFF)
endif()
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
