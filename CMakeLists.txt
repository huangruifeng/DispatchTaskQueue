cmake_minimum_required(VERSION 3.6.0)

if(APPLE)
  set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK YES)
endif()

project(DispatchQueue)

set(CMAKE_OBJECT_PATH_MAX 512)
file(READ "version" VSRSION)
set(CMAKE_CXX_STANDARD 14)
if(WIN32)
	set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  /MP")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG /OPT:REF /OPT:ICF")
endif()

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang")
	message(STATUS "Complie on apple computer use clang.")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(ANDROID)
	string(REPLACE "-fno-rtti" "" FIXED_C_FLAGS ${CMAKE_C_FLAGS})
	string(REPLACE "-fno-exceptions" "" FIXED_C_FLAGS ${FIXED_C_FLAGS})
	if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7-a")
		string(REPLACE "-mfpu=vfpv3-d16" "" FIXED_C_FLAGS ${FIXED_C_FLAGS})
	endif()
	set(CMAKE_C_FLAGS "${FIXED_C_FLAGS}  -D__ANDROID_API__=21")

	string(REPLACE "-fno-rtti" "" FIXED_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	string(REPLACE "-fno-exceptions" "" FIXED_CXX_FLAGS ${FIXED_CXX_FLAGS})
	if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7-a")
		string(REPLACE "-mfpu=vfpv3-d16" "" FIXED_CXX_FLAGS ${FIXED_CXX_FLAGS})
	endif()
	set(CMAKE_CXX_FLAGS "${FIXED_CXX_FLAGS} -D__ANDROID_API__=21")
endif()

if(APPLE)
	if(IOS)
		set(CMAKE_SYSTEM_PROCESSOR aarch64)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=arm64-apple-darwin-eabi-as")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=arm64-apple-darwin-eabi-as")
		set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --target=arm64-apple-darwin-eabi-as")
	else()
		set(CMAKE_SYSTEM_PROCESSOR x86_64)
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG=3")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG=4")
endif()

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_CODE_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND  git rev-list --count HEAD --no-merges
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_COUNT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set_property(GLOBAL PROPERTY COMPILE_DEFINITIONS "-DSDK_COMMIT_VERSION=${GIT_CODE_VERSION}")
set_property(GLOBAL PROPERTY COMPILE_DEFINITIONS "-DVERSION=${VERSION}")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

list(APPEND SRCLIST
test/main.cpp
src/DispatchQueue.h
src/DispatchQueue.cpp)

add_executable(DispatchQueue ${SRCLIST})

find_package(Threads REQUIRED)
target_link_libraries(DispatchQueue Threads::Threads)


#include_directories() header files

#add_subdirectory() subproject 

#configure_file() configure

#add_executable(TESTPROJECT WIN32)

#target_link_libraries(TESTPROJECT )
