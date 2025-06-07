prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}/@CMAKE_INSTALL_BINDIR@
libdir=${prefix}/@CMAKE_INSTALL_LIBDIR@
includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@

Name: @PROJECT_NAME@
Description: Audio fingerprint library
URL: http://acoustid.org/chromaprint
Version: @PROJECT_VERSION@
Libs: -L${libdir} -lchromaprint
Cflags: -I${includedir}

