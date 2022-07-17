cmake_minimum_required(VERSION 3.22)
project (JavaInjector)

set_property(GLOBAL PROPERTY C_STANDARD 23)

add_library(JavaInjector SHARED src/entry.c src/cheat.h src/cheat.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(JavaInjector PRIVATE Threads::Threads)
target_compile_options(JavaInjector PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_include_directories(JavaInjector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/jni)
target_include_directories(JavaInjector PRIVATE include/jni/linux)
