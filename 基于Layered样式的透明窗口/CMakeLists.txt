cmake_minimum_required(VERSION 3.18)
set(PROJECT_NAME "Direct2DSample")
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 20)


link_directories("D:\\sdk\\blend2d\\build")
include_directories("D:\\sdk\\blend2d\\src")
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE 
	"main.cpp" 
	 )

	 

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:UNICODE DEBUG NOMINMAX _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS BL_STATIC WIN32 _WINDOWS>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:UNICODE NDEBUG NOMINMAX _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS BL_STATIC WIN32 _WINDOWS>)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} blend2d kernel32 user32 shell32)