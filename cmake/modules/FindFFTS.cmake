INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(FFTS_INCLUDE_DIR ffts.h
	PATHS
	${FFTS_DIR}/include
	$ENV{FFTS_DIR}/include
	/usr/include
	/usr/local/include
	PATH_SUFFIXES 
	ffts
)

FIND_LIBRARY(FFTS_LIBRARIES
    NAMES
    ffts ffts.dll
    PATHS
	${FFTS_DIR}/lib
	$ENV{FFTS_DIR}/lib
    /usr/lib
    /usr/local/lib
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTS DEFAULT_MSG FFTS_LIBRARIES FFTS_INCLUDE_DIR)
