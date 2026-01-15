cmake_minimum_required(VERSION 3.16)
project(dir2html LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dir2html)

target_sources(dir2html
    PRIVATE
        external/stb_image_write.cpp
        src/alphanum.cpp
        src/commands.cpp
        src/dirdocument.cpp
        src/main.cpp
        src/filetypes/audiofile.cpp
        src/filetypes/imagefile.cpp
        src/filetypes/properties.cpp
        src/filetypes/videofile.cpp
        src/filetypes/videothumbnailer.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            src/alphanum.h
            src/commands.h
            src/dirdocument.h
            src/documentconstants.h
            src/filetypes/audiofile.h
            src/filetypes/imagefile.h
            src/filetypes/properties.h
            src/filetypes/videofile.h
            src/filetypes/videothumbnailer.h)

# pkgconfig libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVFILTER REQUIRED libavfilter)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(MAGIC REQUIRED libmagic)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(TBB REQUIRED tbb)

target_include_directories(dir2html PRIVATE
    ${AVCODEC_INCLUDE_DIRS} ${AVFILTER_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS}
    ${MAGIC_INCLUDE_DIRS} ${TAGLIB_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})
target_link_libraries(dir2html PRIVATE
    ${AVCODEC_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES}
    ${MAGIC_LIBRARIES} ${TAGLIB_LIBRARIES} ${TBB_LIBRARIES})

## optional zlib-ng: will improve compression efficiency for stb
pkg_check_modules(ZLIB-NG QUIET zlib-ng)
if(ZLIB-NG_FOUND)
    target_include_directories(dir2html PRIVATE ${ZLIB-NG_INCLUDE_DIRS})
    target_link_libraries(dir2html PRIVATE ${ZLIB-NG_LIBRARIES})
    target_compile_definitions(dir2html PRIVATE ZLIB_NG_FOUND=1)
else()
    message(STATUS "zlib-ng not found. Thumbnail image compression will be less efficient.")
endif()

# external libraries
add_subdirectory(external/lexbor)
add_subdirectory(external/lexbor-cpp)
target_include_directories(dir2html PRIVATE
    external/base64 external/cxxopts/include external/expected-lite/include external/stb)
target_link_libraries(dir2html PRIVATE lexbor-cpp)

# add src as include dir
target_include_directories(dir2html PRIVATE src)

include(GNUInstallDirs)
install(TARGETS dir2html
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
