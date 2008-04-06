include_directories (${EBOOK-TOOLS_SOURCE_DIR}/src/libepub)
add_executable (einfo einfo.c)
target_link_libraries (einfo epub)    

install ( TARGETS einfo DESTINATION bin )
if(NOT WIN32)
  install ( PROGRAMS lit2epub DESTINATION bin )
endif(NOT WIN32)
