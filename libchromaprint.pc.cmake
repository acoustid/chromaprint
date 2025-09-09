prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_FULL_BINDIR@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: @PROJECT_NAME@
Description: Audio fingerprint library
URL: http://acoustid.org/chromaprint
Version: @PROJECT_VERSION@
Libs: -L${libdir} -lchromaprint
Cflags: -I${includedir}

