cmake_minimum_required(VERSION 3.16)

# Host-side unit tests for the pure-math BSP modules (no ARM toolchain needed).
project(template_host_tests C)

enable_testing()

set(TEST_TARGETS filter pid ringbuf crc cli attitude)

foreach(t IN LISTS TEST_TARGETS)
    add_executable(test_${t} test_${t}.c "../BSP/Src/${t}.c")
    target_include_directories(test_${t} PRIVATE "../BSP/Inc")
    target_link_libraries(test_${t} m)
    add_test(NAME test_${t} COMMAND test_${t})
endforeach()
