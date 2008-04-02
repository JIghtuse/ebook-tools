include_directories (${EBOOK-TOOLS_SOURCE_DIR}/src/libepub ${LIBXML2_INCLUDE_DIR} ${LIBZIP_INCLUDE_DIR})
link_directories (${EBOOK-TOOLS_BINARY_DIR}/libs ${LIBZIP_LIBRARY} ${LIBXML2_LIBRARIES} )
add_library (epub epub.c ocf.c opf.c linklist.c list.c)
target_link_libraries (epub ${LIBZIP_LIBRARY} ${LIBXML2_LIBRARIES})

set_target_properties (epub PROPERTIES VERSION 0.1 SOVERSION 0)

install ( TARGETS epub DESTINATION lib )
install ( FILES epub.h epub_shared.h DESTINATION include )
