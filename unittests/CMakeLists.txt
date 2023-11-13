cmake_minimum_required(VERSION 3.24)
project(radix_unittests)

include(FetchContent)
FetchContent_Declare(catch2
    DOWNLOAD_EXTRACT_TIMESTAMP true
    URL             https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.tar.gz
    URL_HASH        MD5=2c802a4938ed842e2942c60d1d231bb7
    SOURCE_DIR      ${CMAKE_SOURCE_DIR}/${ALP_EXTERN_DIR}/Catch2
)
FetchContent_MakeAvailable(catch2)

set(RADIX_UNITTESTS_SOURCES
    geometry.cpp
    iterator.cpp
    main.cpp
    quad_tree.cpp
    tile.cpp
    tile_heights.cpp
    height_encoding.cpp
)
if (ANDROID)
    qt_add_executable(unittests_radix ${RADIX_UNITTESTS_SOURCES})
    target_link_libraries(unittests_radix PUBLIC Qt::Core Qt::Gui)
    install(TARGETS unittests_radix
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    add_executable(unittests_radix ${RADIX_UNITTESTS_SOURCES})
endif()
target_link_libraries(unittests_radix PUBLIC radix Catch2::Catch2)

if (EMSCRIPTEN)
    set_target_properties(unittests_radix PROPERTIES SUFFIX ".html")
    set(ALP_INSTALL_FILES
        "$<TARGET_FILE_DIR:unittests_radix>/unittests_radix.js"
        "$<TARGET_FILE_DIR:unittests_radix>/unittests_radix.wasm"
        "$<TARGET_FILE_DIR:unittests_radix>/unittests_radix.html"
    )
    install(FILES ${ALP_INSTALL_FILES} DESTINATION ${ALP_WWW_INSTALL_DIR})
endif()
