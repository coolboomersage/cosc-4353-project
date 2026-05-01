cmake_minimum_required(VERSION 3.10)

project(LocalhostServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set(ZLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/zlib132/zlib-1.3.2)
set(XLSXWRITER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/libxlsxwriter-1.2.4/libxlsxwriter-1.2.4)

add_library(zlib STATIC
    ${ZLIB_DIR}/adler32.c
    ${ZLIB_DIR}/compress.c
    ${ZLIB_DIR}/crc32.c
    ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/gzclose.c
    ${ZLIB_DIR}/gzlib.c
    ${ZLIB_DIR}/gzread.c
    ${ZLIB_DIR}/gzwrite.c
    ${ZLIB_DIR}/infback.c
    ${ZLIB_DIR}/inffast.c
    ${ZLIB_DIR}/inflate.c
    ${ZLIB_DIR}/inftrees.c
    ${ZLIB_DIR}/trees.c
    ${ZLIB_DIR}/uncompr.c
    ${ZLIB_DIR}/zutil.c
)

target_include_directories(zlib PUBLIC ${ZLIB_DIR})

if(APPLE)
    target_compile_definitions(zlib PRIVATE _LARGEFILE64_SOURCE)
    target_compile_options(zlib PRIVATE -Wno-implicit-function-declaration)
endif()

if(MSVC)
    target_compile_options(zlib PRIVATE /W0)
endif()

add_library(xlsxwriter STATIC
    ${XLSXWRITER_DIR}/src/app.c
    ${XLSXWRITER_DIR}/src/chart.c
    ${XLSXWRITER_DIR}/src/chartsheet.c
    ${XLSXWRITER_DIR}/src/comment.c
    ${XLSXWRITER_DIR}/src/content_types.c
    ${XLSXWRITER_DIR}/src/core.c
    ${XLSXWRITER_DIR}/src/custom.c
    ${XLSXWRITER_DIR}/src/drawing.c
    ${XLSXWRITER_DIR}/src/format.c
    ${XLSXWRITER_DIR}/src/hash_table.c
    ${XLSXWRITER_DIR}/src/metadata.c
    ${XLSXWRITER_DIR}/src/packager.c
    ${XLSXWRITER_DIR}/src/relationships.c
    ${XLSXWRITER_DIR}/src/rich_value.c
    ${XLSXWRITER_DIR}/src/rich_value_rel.c
    ${XLSXWRITER_DIR}/src/rich_value_structure.c
    ${XLSXWRITER_DIR}/src/rich_value_types.c
    ${XLSXWRITER_DIR}/src/shared_strings.c
    ${XLSXWRITER_DIR}/src/styles.c
    ${XLSXWRITER_DIR}/src/table.c
    ${XLSXWRITER_DIR}/src/theme.c
    ${XLSXWRITER_DIR}/src/utility.c
    ${XLSXWRITER_DIR}/src/vml.c
    ${XLSXWRITER_DIR}/src/workbook.c
    ${XLSXWRITER_DIR}/src/worksheet.c
    ${XLSXWRITER_DIR}/src/xmlwriter.c
    ${XLSXWRITER_DIR}/third_party/md5/md5.c
    ${XLSXWRITER_DIR}/third_party/minizip/ioapi.c
    ${XLSXWRITER_DIR}/third_party/minizip/zip.c
    ${XLSXWRITER_DIR}/third_party/tmpfileplus/tmpfileplus.c
)

target_include_directories(xlsxwriter PUBLIC
    ${XLSXWRITER_DIR}/include
    ${XLSXWRITER_DIR}/third_party/md5
)

# iowin32.c is only needed on Windows
if(WIN32)
    target_sources(xlsxwriter PRIVATE
        ${XLSXWRITER_DIR}/third_party/minizip/iowin32.c
    )
endif()

target_link_libraries(xlsxwriter PUBLIC zlib)



add_library(sqlite3 STATIC sqlite3_wrapper.c)

add_library(argon2 STATIC
    external/phc-winner-argon2-20190702/src/argon2.c
    external/phc-winner-argon2-20190702/src/core.c
    external/phc-winner-argon2-20190702/src/blake2/blake2b.c
    external/phc-winner-argon2-20190702/src/thread.c
    external/phc-winner-argon2-20190702/src/encoding.c
    external/phc-winner-argon2-20190702/src/ref.c
)

target_include_directories(argon2 PUBLIC 
    external/phc-winner-argon2-20190702/include
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

add_executable(server
    main.cpp
    "sub pages/export report.cpp"
)

target_link_libraries(server PRIVATE
    sqlite3
    argon2
    xlsxwriter
)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

target_compile_definitions(server PRIVATE CPPHTTPLIB_NO_UNIX_SOCKETS)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()

if(UNIX)
    target_link_libraries(server PRIVATE pthread)
endif()