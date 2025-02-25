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

