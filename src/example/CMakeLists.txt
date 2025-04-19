add_executable(benchmark
    TPCH.cpp
    benchmark.cpp
)

add_executable(secyandemo
    TPCH.cpp
    secyandemo.cpp
)

add_executable(main
    main.cpp
)

add_executable(psi
    mine.cpp
)

add_executable(mpsi
    multipartypsi.cpp
    TPCH.cpp
)

target_link_libraries(benchmark 
    PUBLIC secyan
    PUBLIC ENCRYPTO_utils::encrypto_utils
    PUBLIC Boost::program_options)

target_link_libraries(secyandemo 
    PUBLIC secyan)
    
target_link_libraries(main 
    PUBLIC secyan)
    
target_link_libraries(psi 
    PUBLIC secyan)

target_link_libraries(mpsi
    PUBLIC secyan)

# Copy web frontend files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/index.html
     ${CMAKE_CURRENT_SOURCE_DIR}/frontend.js
     ${CMAKE_CURRENT_SOURCE_DIR}/server.js
     ${CMAKE_CURRENT_SOURCE_DIR}/package.json
     DESTINATION ${CMAKE_BINARY_DIR}/src/example)

# Custom target for installing npm dependencies
add_custom_target(web_dependencies
    COMMAND cd ${CMAKE_BINARY_DIR}/src/example && npm install
    DEPENDS ${CMAKE_BINARY_DIR}/src/example/package.json
    COMMENT "Installing web frontend dependencies")

# Make mpsi target depend on web_dependencies
add_dependencies(mpsi web_dependencies)

