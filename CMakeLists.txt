set_library_version(LibraryVersion)
add_definitions(-DLIBRARY_VERSION="${LibraryVersion}")

find_package(yaml-cpp QUIET)

if (yaml-cpp_FOUND)
    message(STATUS "yaml-cpp found")
    set(external_libs ${external_libs} yaml-cpp)
else ()
    message(
        WARNING
            "yaml-cpp NOT found, TRestRawFemDAQToSignalProcess won't be compiled."
    )
     add_definitions(-DNO_YAML_CPP)
endif ()

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
