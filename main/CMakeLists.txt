include_directories (${PROJECT_BINARY_DIR}/src/
                     ${PROJECT_BINARY_DIR}/elfcloud-cpp/src)

link_directories (${PROJECT_BINARY_DIR}/src)
link_directories (${PROJECT_BINARY_DIR}/elfcloud-cpp)

add_executable (elfcloud-fuse
    main.c)

target_link_libraries (elfcloud-fuse
    elfcloud-cpp
    elfcloud-fs
    ${FUSE_LIBRARIES}
)

install (TARGETS elfcloud-fuse DESTINATION bin)

IF (APPLE)
      # Install needed Frameworks..
      # In future do this same in Windows and Linux!

      INSTALL(CODE "execute_process(COMMAND mkdir \"\${CMAKE_INSTALL_PREFIX}/libs\")")
      INSTALL(CODE "execute_process(COMMAND dylibbundler -d \"\${CMAKE_INSTALL_PREFIX}/libs\" -b -x \"\${CMAKE_INSTALL_PREFIX}/bin/elfcloud-fuse\")")

ENDIF (APPLE)


