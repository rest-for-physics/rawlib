# Perhaps this could be a common cmake macro inside framework
execute_process(
    COMMAND git describe --abbrev=0 --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE LibraryVersion
    ERROR_VARIABLE gitstatuserr)

string(REPLACE "\n" "" LibraryVersion ${LibraryVersion})
message("Raw library version: ${LibraryVersion}")
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(excludes TRestRawMemoryBufferToSignalProcess)
endif (CMAKE_SYSTEM_NAME MATCHES "Windows")

set(deps)

compilelib(deps)

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/macros/*")
install(FILES ${MAC} DESTINATION ./macros/raw)

file(GLOB_RECURSE MAC "${CMAKE_CURRENT_SOURCE_DIR}/examples/*")
install(FILES ${MAC} DESTINATION ./examples/raw)

add_library_test()
