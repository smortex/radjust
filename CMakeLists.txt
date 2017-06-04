cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

project(radjust C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Werror")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	add_definitions("-D_GNU_SOURCE")
endif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

find_library(MD_LIBRARY NAMES md)
find_path(MD_INCLUDE_DIR sha256.h)

if(MD_LIBRARY AND MD_INCLUDE_DIR)
	set(MESSAGE_DIGEST_LIBRARY ${MD_LIBRARY})
	set(MESSAGE_DIGEST_INCLUDE_DIR ${MD_INCLUDE_DIR})
	add_definitions("-DHAVE_MD")
else(MD_LIBRARY AND MD_INCLUDE_DIR)
	find_package(OpenSSL REQUIRED)
	set(MESSAGE_DIGEST_LIBRARY ${OPENSSL_CRYPTO_LIBRARY})
	set(MESSAGE_DIGEST_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR})
	add_definitions("-DHAVE_OPENSSL")
endif(MD_LIBRARY AND MD_INCLUDE_DIR)

subdirs(libadjust client server fakessh radjust)

add_custom_target(style DEPENDS style_fix_space style_astyle)
add_custom_target(style_fix_space COMMAND find . -name '*.[ch]' -exec perl -pi -e 's/[ \\t]+$$//' '{}' '\\;' COMMENT "Fixing invalid space sequences")
add_custom_target(style_astyle COMMAND find . -name '*.[ch]' -exec astyle --style=linux --indent=force-tab-x --lineend=linux {} '\\;' COMMENT "Checking source code style" DEPENDS style_fix_space)
