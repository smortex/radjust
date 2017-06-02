add_executable(client client.c)
include_directories(../libadjust)
target_link_libraries(client adjust ${MESSAGE_DIGEST_LIBRARY})
