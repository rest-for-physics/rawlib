set( LibraryVersion "1.3" )
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

COMPILELIB("")

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/macros/*")
INSTALL(FILES ${MAC} DESTINATION ./macros/raw)
