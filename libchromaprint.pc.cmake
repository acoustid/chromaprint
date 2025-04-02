prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${EXEC_INSTALL_PREFIX}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: ${PROJECT_NAME}
Description: Audio fingerprint library
URL: http://acoustid.org/chromaprint
Version: ${PROJECT_VERSION}
Libs: -L${LIB_INSTALL_DIR} -lchromaprint @CHROMAPRINT_ADDITIONAL_LIBS@
Cflags: -I${INCLUDE_INSTALL_DIR}

