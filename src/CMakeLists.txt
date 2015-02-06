include_directories (${PROJECT_BINARY_DIR}/elfcloud-cpp/src
                     ${JSONCPP_INCLUDE_DIR}/jsoncpp
                     ${JSONCPP_INCLUDE_DIR}/rapidxml)

add_library (elfcloud-fs
    elfcloudfs-cache.cpp
    elfcloudfs-dircache.cpp
    elfcloudfs.cpp
    fusewrap.cpp
)

target_link_libraries (elfcloud-fs
    elfcloud-cpp
    ${CMAKE_THREAD_LIBS_INIT}
)

