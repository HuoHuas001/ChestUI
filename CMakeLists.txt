cmake_minimum_required(VERSION 3.21)
project(plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

file(GLOB_RECURSE SRC_FILES_DIR
        ${PROJECT_SOURCE_DIR}/SDK/include/*.cpp
        ${PROJECT_SOURCE_DIR}/SDK/include/*.hpp
        ${PROJECT_SOURCE_DIR}/SDK/include/*.h
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.h
        )

include_directories(SDK/include)
include_directories(src)
include_directories(include)

link_directories(.)

add_definitions(-D"NDEBUG" -D"TEMPLATE_EXPORTS" -D"WIN32_LEAN_AND_MEAN" -D"_CRT_SECURE_NO_WARNINGS" -D"_WINDOWS"
        -D"_USRDLL" -D"_AMD64_" -D"NOMINMAX" -D"_WINDLL" -D"_UNICODE" -D"UNICODE")

add_compile_options(
        /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /sdl /Zc:inline /fp:precise /errorReport:prompt /WX-
        /Zc:forScope /Gd /Oi /MD /std:c++17 /FC /EHsc /nologo /diagnostics:column
)

add_link_options(
        /MANIFEST /LTCG:incremental /NXCOMPAT /DEBUG /DLL /MACHINE:X64 /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:WINDOWS
        /MANIFESTUAC:NO /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /DELAYLOAD:"bedrock_server.dll" /TLBID:1
)

add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC_FILES_DIR})

target_link_libraries(${CMAKE_PROJECT_NAME} 
	"${CMAKE_SOURCE_DIR}/SDK/lib/LiteLoader.lib"
	"${CMAKE_SOURCE_DIR}/SDK/lib/bedrock_server_api.lib"
	"${CMAKE_SOURCE_DIR}/SDK/lib/bedrock_server_var.lib"
	"${CMAKE_SOURCE_DIR}/SDK/lib/SymDBHelper.lib"
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
    COMMAND cmd /c ${PROJECT_SOURCE_DIR}/prepare_libraries.cmd ${PROJECT_SOURCE_DIR}
)
