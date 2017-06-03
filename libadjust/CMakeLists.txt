add_library(adjust error.c file.c file_info.c networking.c stats.c)
include_directories(${MESSAGE_DIGEST_INCLUDE_DIR})
