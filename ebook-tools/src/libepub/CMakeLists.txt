include_directories (${EBOOK-TOOLS_SOURCE_DIR}/src/libepub ${LIBXML2_INCLUDE_DIR} ${LIBZIP_INCLUDE_DIR})
add_library (epub SHARED epub.c ocf.c opf.c linklist.c list.c)
target_link_libraries (epub ${LIBZIP_LIBRARY} ${LIBXML2_LIBRARIES})

set_target_properties (epub PROPERTIES VERSION 0.2.1 SOVERSION 1)

install ( TARGETS epub RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )
install ( FILES epub.h epub_shared.h epub_version.h DESTINATION include )
