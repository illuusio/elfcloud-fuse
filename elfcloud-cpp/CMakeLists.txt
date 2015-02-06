include_directories (${PROJECT_BINARY_DIR}/elfcloud-cpp/src
                     ${JSONCPP_INCLUDE_DIR}/jsoncpp
                     ${PROJECT_SOURCE_DIR}/lib/rapidxml-1.13)

add_library (elfcloud-cpp
             src/Client.cpp
             src/Config.cpp
             src/CryptoHelper.cpp
             src/Exception.cpp
             src/Key.cpp
             src/KeyImpl.cpp
             src/Object.cpp            
             src/Vault.cpp
             src/Cluster.cpp
             src/Container.cpp
             src/DataItem.cpp
             src/IllegalParameterException.cpp
             src/KeyHint.cpp
             src/KeyRing.cpp  
             src/ServerConnection.cpp
)

target_link_libraries (elfcloud-cpp
    ${JSONCPP_LIBRARY}
    ${CRYPTOPP_LIBRARIES}
    ${CURL_LIBRARIES}
)

add_executable (ec-test-cpp
    testprog/Main.cpp
)

target_link_libraries (ec-test-cpp
    elfcloud-cpp
)

