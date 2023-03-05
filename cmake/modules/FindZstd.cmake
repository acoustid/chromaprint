find_path(ZSTD_SOURCE_DIR
    NAMES
        build/single_file_libs/zstd.c
    PATHS
        $ENV{ZSTD_ROOT}
        ${ZSTD_ROOT}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/zstd
    NO_DEFAULT_PATH
    DOC "Zstd source dir"
)
mark_as_advanced(ZSTD_SOURCE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Zstd DEFAULT_MSG ZSTD_SOURCE_DIR)

if(ZSTD_FOUND)
    SET(ZSTD_INCLUDE_DIRS
        ${ZSTD_SOURCE_DIR}/lib
    )
    SET(ZSTD_SOURCES
        ${ZSTD_SOURCE_DIR}/build/single_file_libs/zstd.c
    )
endif()
