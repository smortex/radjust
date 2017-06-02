cmake_minimum_required(VERSION 3.8)
cmake_policy(VERSION 3.8)

project(radjust LANGUAGES C)

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
