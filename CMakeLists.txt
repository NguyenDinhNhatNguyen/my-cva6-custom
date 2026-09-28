cmake_minimum_required(VERSION 3.10)
project(my_cva6_custom C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SYSTEMC_SEARCH_PATHS
    $ENV{SYSTEMC_HOME}
    "/usr/local/systemc-2.3.4"
    "/usr/local/systemc"
    "/opt/systemc"
    "/home/nguyn/systemc-2.3.4"
)

find_path(SYSTEMC_INCLUDE_DIR systemc.h PATHS ${SYSTEMC_SEARCH_PATHS} PATH_SUFFIXES include)
find_library(SYSTEMC_LIBRARY systemc PATHS ${SYSTEMC_SEARCH_PATHS} PATH_SUFFIXES lib-linux64 lib)

if(SYSTEMC_INCLUDE_DIR AND SYSTEMC_LIBRARY)
    message(STATUS "+++ DA TIM THAY SYSTEMC +++")

	if(NOT TARGET SystemC::systemc)
        add_library(SystemC::systemc UNKNOWN IMPORTED)
        set_target_properties(SystemC::systemc PROPERTIES
            IMPORTED_LOCATION "${SYSTEMC_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SYSTEMC_INCLUDE_DIR}"
        )
    endif()
else()
    message(FATAL_ERROR "--- KHONG TIM THAY SYSTEMC ---")
endif()

# add_subdirectory(third_party/riscv-vp/vp)
add_subdirectory(cpu_models/riscv_vp)

add_executable(my_vp.exe main.cpp)

target_include_directories(my_vp.exe PRIVATE 
    ${SYSTEMC_INCLUDE_DIR} 
    .
    cpu_models/include
    third_party/riscv-vp/vp/src
)

target_link_libraries(my_vp.exe PRIVATE riscv_vp SystemC::systemc)