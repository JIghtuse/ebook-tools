project (EBOOK-TOOLS C)

cmake_minimum_required(VERSION 2.4.0)

#set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBRARY_OUTPUT_PATH ${EBOOK-TOOLS_BINARY_DIR}/libs)
set(EXECUTABLE_OUTPUT_PATH ${EBOOK-TOOLS_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(LibXml2 REQUIRED)
find_package(LibZip REQUIRED)

if(CMAKE_C_COMPILER_ID MATCHES GNU)
  set(CMAKE_C_FLAGS "-Wall -W -Wno-long-long -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -Wshadow -fno-common ${CMAKE_C_FLAGS}")
endif(CMAKE_C_COMPILER_ID MATCHES GNU)
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

INCLUDE("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)

add_subdirectory (src)
