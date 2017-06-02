add_executable(radjust radjust.c)
include_directories(../libadjust)
target_link_libraries(radjust adjust ${MESSAGE_DIGEST_LIBRARY})

install(TARGETS radjust RUNTIME DESTINATION bin)
