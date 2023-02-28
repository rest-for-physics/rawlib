set(LibraryVersion "1.8")
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(excludes TRestRawMemoryBufferToSignalProcess)
endif (CMAKE_SYSTEM_NAME MATCHES "Windows")

set(deps detector)

if (NOT ${RESTLIB_DETECTOR} MATCHES "ON")
    list(REMOVE_ITEM deps detector)
endif ()

compilelib(deps)

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/macros/*")
install(FILES ${MAC} DESTINATION ./macros/raw)

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/examples/*")
install(FILES ${MAC} DESTINATION ./examples/raw)

add_library_test()
