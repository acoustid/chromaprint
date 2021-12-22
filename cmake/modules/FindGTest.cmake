# 
# Try to find GoogleTest library  
#
# Defines the following variables:
# 
#   GTEST_FOUND - Found the GoogleTest library
#   GTEST_INCLUDE_DIRS - Include directories
#   GTEST_SOURCES - Source code to include in your project
#
# Accepts the following variables as input:
#
#   GTEST_ROOT - (as CMake or environment variable)
#                  The root directory of GoogleTest sources
#
# =========================================================
#
# Copyright (C) 2014 Lukas Lalinsky <lalinsky@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#  * The names of Kitware, Inc., the Insight Consortium, or the names of
#    any consortium members, or of any contributors, may not be used to
#    endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_path(GTEST_SOURCE_DIR
    NAMES
        src/gtest-all.cc
    PATHS
        $ENV{GTEST_ROOT}
        ${GTEST_ROOT}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/googletest/googletest
    NO_DEFAULT_PATH
    DOC "GoogleTest tools headers"
)
mark_as_advanced(GTEST_SOURCE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTest DEFAULT_MSG GTEST_SOURCE_DIR)

if(GTEST_FOUND)
    SET(GTEST_INCLUDE_DIRS
        ${GTEST_SOURCE_DIR}/include
    )
    SET(GTEST_SOURCES
        ${GTEST_SOURCE_DIR}/src/gtest-all.cc
    )
endif()
