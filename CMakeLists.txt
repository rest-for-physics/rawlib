set(LibraryVersion "1.5")
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

COMPILELIB("")

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/macros/*")
INSTALL(FILES ${MAC} DESTINATION ./macros/raw)

ADD_LIBRARY_TEST()
