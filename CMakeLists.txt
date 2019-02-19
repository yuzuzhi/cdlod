cmake_minimum_required (VERSION 2.6)
project (cdlod)
include_directories("C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")
link_directories("C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86")

add_definitions(-DUNICODE -D_UNICODE)

include_directories("${PROJECT_SOURCE_DIR}/DXUT/Core")
include_directories(DXUT/Optional)

add_subdirectory ("${PROJECT_SOURCE_DIR}/DXUT/Core")
add_subdirectory ("${PROJECT_SOURCE_DIR}/DXUT/Optional")
set (EXTRA_LIBS ${EXTRA_LIBS} DXUT)
set (EXTRA_LIBS ${EXTRA_LIBS} DXUTOpt)
set (EXTRA_LIBS ${EXTRA_LIBS} DXUTOpt)

set (EXTRA_LIBS ${EXTRA_LIBS} version d3dcompiler d3dx11d d3dx9d dxerr dxguid winmm comctl32 Imm32 dsound legacy_stdio_definitions)

#set (EXTRA_LIBS ${EXTRA_LIBS} DXUTOpt)


aux_source_directory(Src SRC_LIST)
add_executable (cdlod WIN32 ${SRC_LIST})
target_link_libraries (cdlod ${EXTRA_LIBS})

#install(TARGETS bin/Media DESTINATION ${EXECUTABLE_OUTPUT_PATH})
