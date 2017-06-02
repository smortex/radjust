add_executable(server server.c)
include_directories(../libadjust)
target_link_libraries(server adjust ${MESSAGE_DIGEST_LIBRARY})
