set(LibraryVersion "1.8")
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(excludes TRestRawMemoryBufferToSignalProcess)
endif (CMAKE_SYSTEM_NAME MATCHES "Windows")

set(deps detector)

if (NOT ${RESTLIB_DETECTOR} MATCHES "ON")
    list(REMOVE_ITEM deps detector)
endif ()

COMPILELIB(deps)

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/macros/*")
INSTALL(FILES ${MAC} DESTINATION ./macros/raw)

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/examples/*")
INSTALL(FILES ${MAC} DESTINATION ./examples/raw)

ADD_LIBRARY_TEST()
