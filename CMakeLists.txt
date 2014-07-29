cmake_minimum_required(VERSION 2.8)
project( mahv )
find_package( OpenCV REQUIRED )

set (source_dir "src/")
set (source_ext ".cpp")
set (target_ext ".app")

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

include_directories(include/json11)
add_subdirectory(include/json11)

set(file_names 
    "main" 
    "gui-test")

foreach(f ${file_names})
    set(source_name "${source_dir}${f}${source_ext}")
    set(app_name "${f}${target_ext}")
    
    add_executable( ${app_name} ${source_name} )
    target_link_libraries( ${app_name} ${OpenCV_LIBS} ncurses json11)
endforeach(f)