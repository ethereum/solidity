get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../deps" ABSOLUTE)

set(CACHE_DIR "${ROOT_DIR}/cache")
set(PACKAGES_DIR "${ROOT_DIR}/packages")

function(download URL DST_FILE STATUS)
  set(TMP_FILE "${DST_FILE}.part")

  get_filename_component(FILE_NAME ${DST_FILE} NAME)
  if (NOT EXISTS ${DST_FILE})
    message("Downloading ${FILE_NAME}")
    file(DOWNLOAD ${URL} ${TMP_FILE} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if (STATUS_CODE EQUAL 0)
      file(RENAME ${TMP_FILE} ${DST_FILE})
    else()
      file(REMOVE ${TMP_FILE})
      list(GET DOWNLOAD_STATUS 1 ERROR_MSG)

      message("ERROR! Downloading '${FILE_NAME}' failed.")
      message(STATUS "URL:   ${URL}")
      message(STATUS "Error: ${STATUS_CODE} ${ERROR_MSG}")
      set(STATUS FALSE PARENT_SCOPE)
      return()
    endif()
  else()
    message("Using cached ${FILE_NAME}")
  endif()
  set(STATUS TRUE PARENT_SCOPE)
endfunction(download)

function(download_and_unpack PACKAGE_URL DST_DIR)
  get_filename_component(FILE_NAME ${PACKAGE_URL} NAME)

  set(DST_FILE "${CACHE_DIR}/${FILE_NAME}")
  set(TMP_FILE "${DST_FILE}.part")

  file(MAKE_DIRECTORY ${CACHE_DIR})
  file(MAKE_DIRECTORY ${DST_DIR})

  download(${PACKAGE_URL} ${DST_FILE} STATUS)

  if (STATUS)
    message("Unpacking ${FILE_NAME} to ${DST_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${DST_FILE}
        WORKING_DIRECTORY ${DST_DIR})
  endif()
endfunction(download_and_unpack)

# Packs installed package binaries and headers into an archive.
function(create_package NAME DIR)
  message("Creating package ${NAME}")
  file(MAKE_DIRECTORY ${PACKAGES_DIR})

  # To create an archive without addicional top level directory
  # (like package-X.Y.Z) we need to know all top level files/dirs.
  # Usually it is just "win64" dir.
  file(GLOB TOP_FILES RELATIVE ${DIR} "${DIR}/*")

  set(PACKAGE_FILE "${PACKAGES_DIR}/${NAME}.tar.gz")
  execute_process(COMMAND ${CMAKE_COMMAND} -E
      tar -czf ${PACKAGE_FILE} ${TOP_FILES}
      WORKING_DIRECTORY ${DIR})
endfunction(create_package)

# Downloads the source code of the package and unpacks it to dedicated 'src'
# dir. Also creates 'build' and 'install' dir to be used by a build script.
function(prepare_package_source NAME VERSION URL)
  set(PACKAGE_NAME "${NAME}-${VERSION}")

  set(PACKAGE_DIR "${CACHE_DIR}/${PACKAGE_NAME}")
  set(SOURCE_DIR "${PACKAGE_DIR}/src")
  set(BUILD_DIR "${PACKAGE_DIR}/build")
  set(INSTALL_DIR "${PACKAGE_DIR}/install")

  if (NOT EXISTS ${SOURCE_DIR})
    download_and_unpack(${URL} ${PACKAGE_DIR} STATUS)
    file(GLOB ORIG_SOURCE_DIR_NAME "${PACKAGE_DIR}/*")
    file(RENAME ${ORIG_SOURCE_DIR_NAME} ${SOURCE_DIR})
  endif()

  file(MAKE_DIRECTORY ${BUILD_DIR})
  file(MAKE_DIRECTORY ${INSTALL_DIR})

  # Export names and dirs to be used by a package-specific build script.
  set(PACKAGE_NAME ${PACKAGE_NAME} PARENT_SCOPE)
  set(SOURCE_DIR   ${SOURCE_DIR}   PARENT_SCOPE)
  set(BUILD_DIR    ${BUILD_DIR}    PARENT_SCOPE)
  set(INSTALL_DIR  ${INSTALL_DIR}  PARENT_SCOPE)
endfunction()

set(INSTALL_DIR "${ROOT_DIR}/install")
set(SERVER "https://github.com/ethereum/cpp-dependencies/releases/download/vs2017/")

function(download_and_install PACKAGE_NAME)
  download_and_unpack("${SERVER}${PACKAGE_NAME}.tar.gz" ${INSTALL_DIR})
endfunction(download_and_install)

download_and_install("boost-1.67.0")
