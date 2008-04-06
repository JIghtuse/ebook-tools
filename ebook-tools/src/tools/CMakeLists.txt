include_directories (${EBOOK-TOOLS_SOURCE_DIR}/src/libepub)
add_executable (einfo einfo.c)
target_link_libraries (einfo epub)    

install ( TARGETS einfo DESTINATION bin )
install ( PROGRAMS lit2epub DESTINATION bin )