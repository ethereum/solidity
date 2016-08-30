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

get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(CACHE_DIR "${ROOT_DIR}/deps/cache")
set(INSTALL_DIR "${ROOT_DIR}/deps/install")
download_and_unpack("http://winsvega.com/files/eth.tar.gz" ${INSTALL_DIR})
