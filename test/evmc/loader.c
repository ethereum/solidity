/* EVMC: Ethereum Client-VM Connector API.
 * Copyright 2018-2019 The EVMC Authors.
 * Licensed under the Apache License, Version 2.0.
 */

#include <evmc/loader.h>

#include <evmc/evmc.h>
#include <evmc/helpers.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(EVMC_LOADER_MOCK)
#include "../../test/unittests/loader_mock.h"
#elif _WIN32
#include <Windows.h>
#define DLL_HANDLE HMODULE
#define DLL_OPEN(filename) LoadLibrary(filename)
#define DLL_CLOSE(handle) FreeLibrary(handle)
#define DLL_GET_CREATE_FN(handle, name) (evmc_create_fn)(uintptr_t) GetProcAddress(handle, name)
#define DLL_GET_ERROR_MSG() NULL
#else
#include <dlfcn.h>
#define DLL_HANDLE void*
#define DLL_OPEN(filename) dlopen(filename, RTLD_LAZY)
#define DLL_CLOSE(handle) dlclose(handle)
#define DLL_GET_CREATE_FN(handle, name) (evmc_create_fn)(uintptr_t) dlsym(handle, name)
#define DLL_GET_ERROR_MSG() dlerror()
#endif

#ifdef __has_attribute
#if __has_attribute(format)
#define ATTR_FORMAT(archetype, string_index, first_to_check) \
    __attribute__((format(archetype, string_index, first_to_check)))
#endif
#endif

#ifndef ATTR_FORMAT
#define ATTR_FORMAT(...)
#endif

#if !_WIN32
/*
 * Provide strcpy_s() for GNU libc.
 * The availability check might need to adjusted for other C standard library implementations.
 */
static void strcpy_s(char* dest, size_t destsz, const char* src)
{
    size_t len = strlen(src);
    if (len > destsz - 1)
        len = destsz - 1;
    memcpy(dest, src, len);
    dest[len] = 0;
}
#endif

#define PATH_MAX_LENGTH 4096

static const char* last_error_msg = NULL;

#define LAST_ERROR_MSG_BUFFER_SIZE 511

// Buffer for formatted error messages.
// It has one null byte extra to avoid buffer read overflow during concurrent access.
static char last_error_msg_buffer[LAST_ERROR_MSG_BUFFER_SIZE + 1];

ATTR_FORMAT(printf, 2, 3)
static enum evmc_loader_error_code set_error(enum evmc_loader_error_code error_code,
                                             const char* format,
                                             ...)
{
    va_list args;
    va_start(args, format);
    if (vsnprintf(last_error_msg_buffer, LAST_ERROR_MSG_BUFFER_SIZE, format, args) <
        LAST_ERROR_MSG_BUFFER_SIZE)
        last_error_msg = last_error_msg_buffer;
    va_end(args);
    return error_code;
}


evmc_create_fn evmc_load(const char* filename, enum evmc_loader_error_code* error_code)
{
    last_error_msg = NULL;  // Reset last error.
    enum evmc_loader_error_code ec = EVMC_LOADER_SUCCESS;
    evmc_create_fn create_fn = NULL;

    if (!filename)
    {
        ec = set_error(EVMC_LOADER_INVALID_ARGUMENT, "invalid argument: file name cannot be null");
        goto exit;
    }

    const size_t length = strlen(filename);
    if (length == 0)
    {
        ec = set_error(EVMC_LOADER_INVALID_ARGUMENT, "invalid argument: file name cannot be empty");
        goto exit;
    }
    else if (length > PATH_MAX_LENGTH)
    {
        ec = set_error(EVMC_LOADER_INVALID_ARGUMENT,
                       "invalid argument: file name is too long (%d, maximum allowed length is %d)",
                       (int)length, PATH_MAX_LENGTH);
        goto exit;
    }

    DLL_HANDLE handle = DLL_OPEN(filename);
    if (!handle)
    {
        // Get error message if available.
        last_error_msg = DLL_GET_ERROR_MSG();
        if (last_error_msg)
            ec = EVMC_LOADER_CANNOT_OPEN;
        else
            ec = set_error(EVMC_LOADER_CANNOT_OPEN, "cannot open %s", filename);
        goto exit;
    }

    // Create name buffer with the prefix.
    const char prefix[] = "evmc_create_";
    const size_t prefix_length = strlen(prefix);
    char prefixed_name[sizeof(prefix) + PATH_MAX_LENGTH];
    strcpy_s(prefixed_name, sizeof(prefixed_name), prefix);

    // Find filename in the path.
    const char* sep_pos = strrchr(filename, '/');
#if _WIN32
    // On Windows check also Windows classic path separator.
    const char* sep_pos_windows = strrchr(filename, '\\');
    sep_pos = sep_pos_windows > sep_pos ? sep_pos_windows : sep_pos;
#endif
    const char* name_pos = sep_pos ? sep_pos + 1 : filename;

    // Skip "lib" prefix if present.
    const char lib_prefix[] = "lib";
    const size_t lib_prefix_length = strlen(lib_prefix);
    if (strncmp(name_pos, lib_prefix, lib_prefix_length) == 0)
        name_pos += lib_prefix_length;

    char* base_name = prefixed_name + prefix_length;
    strcpy_s(base_name, PATH_MAX_LENGTH, name_pos);

    // Trim the file extension.
    char* ext_pos = strrchr(prefixed_name, '.');
    if (ext_pos)
        *ext_pos = 0;

    // Replace all "-" with "_".
    char* dash_pos = base_name;
    while ((dash_pos = strchr(dash_pos, '-')) != NULL)
        *dash_pos++ = '_';

    // Search for the built function name.
    while ((create_fn = DLL_GET_CREATE_FN(handle, prefixed_name)) == NULL)
    {
        // Shorten the base name by skipping the `word_` segment.
        const char* shorter_name_pos = strchr(base_name, '_');
        if (!shorter_name_pos)
            break;

        memmove(base_name, shorter_name_pos + 1, strlen(shorter_name_pos) + 1);
    }

    if (!create_fn)
        create_fn = DLL_GET_CREATE_FN(handle, "evmc_create");

    if (!create_fn)
    {
        DLL_CLOSE(handle);
        ec = set_error(EVMC_LOADER_SYMBOL_NOT_FOUND, "EVMC create function not found in %s",
                       filename);
    }

exit:
    if (error_code)
        *error_code = ec;
    return create_fn;
}

const char* evmc_last_error_msg()
{
    const char* m = last_error_msg;
    last_error_msg = NULL;
    return m;
}

struct evmc_instance* evmc_load_and_create(const char* filename,
                                           enum evmc_loader_error_code* error_code)
{
    // First load the DLL. This also resets the last_error_msg;
    evmc_create_fn create_fn = evmc_load(filename, error_code);

    if (!create_fn)
        return NULL;

    struct evmc_instance* instance = create_fn();
    if (!instance)
    {
        *error_code = set_error(EVMC_LOADER_INSTANCE_CREATION_FAILURE,
                                "creating EVMC instance of %s has failed", filename);
        return NULL;
    }

    if (!evmc_is_abi_compatible(instance))
    {
        *error_code = set_error(EVMC_LOADER_ABI_VERSION_MISMATCH,
                                "EVMC ABI version %d of %s mismatches the expected version %d",
                                instance->abi_version, filename, EVMC_ABI_VERSION);
        return NULL;
    }

    return instance;
}
