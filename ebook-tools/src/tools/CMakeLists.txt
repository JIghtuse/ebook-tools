include_directories (${EBOOK-TOOLS_SOURCE_DIR}/src/libepub ${LIBXML2_INCLUDE_DIR} LIBZIP_INCLUDE_DIR)
link_directories (${EBOOK-TOOLS_BINARY_DIR}/libs)
add_executable (einfo einfo.c)
target_link_libraries (einfo epub)    

install ( TARGETS einfo DESTINATION bin )
install ( PROGRAMS lit2epub DESTINATION bin )