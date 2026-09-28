cmake_minimum_required(VERSION 3.10)
project(my_cva6_dummy_cpu C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SYSTEMC_INCLUDE_DIR "/usr/local/systemc-2.3.4/include")

if(EXISTS "/usr/local/systemc-2.3.4/lib-linux64/libsystemc.so")
    set(SYSTEMC_LIBRARY "/usr/local/systemc-2.3.4/lib-linux64/libsystemc.so")
elseif(EXISTS "/usr/local/systemc-2.3.4/lib-linux/libsystemc.so")
    set(SYSTEMC_LIBRARY "/usr/local/systemc-2.3.4/lib-linux/libsystemc.so")
else()
    set(SYSTEMC_LIBRARY "/usr/local/systemc-2.3.4/lib/libsystemc.so")
endif()

message(STATUS "+++ FIXED INCLUDE PATH: ${SYSTEMC_INCLUDE_DIR} +++")
message(STATUS "+++ FIXED LIBRARY PATH: ${SYSTEMC_LIBRARY} +++")

add_executable(my_vp.exe main.cpp)


target_include_directories(my_vp.exe PRIVATE 
    ${SYSTEMC_INCLUDE_DIR} 
    .
    IPs                 
    CPU                 
)

target_link_libraries(my_vp.exe PRIVATE ${SYSTEMC_LIBRARY})

set_target_properties(my_vp.exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/.."
)
