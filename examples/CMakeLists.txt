cmake_minimum_required(VERSION 3.23)

project(eigenerator_examples CXX)
set (CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
find_package(Torch)
pkg_search_module(EIS REQUIRED libeisgenerator)

add_executable(${PROJECT_NAME}_simple simple.cpp)
target_link_libraries(${PROJECT_NAME}_simple ${EIS_LIBRARIES})
target_include_directories(${PROJECT_NAME}_simple PRIVATE ${EIS_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME}_simple PRIVATE "-Wall" "-g")

add_executable(${PROJECT_NAME}_compile compileing.cpp)
target_link_libraries(${PROJECT_NAME}_compile ${EIS_LIBRARIES})
target_include_directories(${PROJECT_NAME}_compile PRIVATE ${EIS_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME}_compile PRIVATE "-Wall" "-g")

if(Torch_FOUND)
	message("Found torch will compile torch example")
	add_executable(${PROJECT_NAME}_torch torchscript.cpp)
	target_link_libraries(${PROJECT_NAME}_torch ${EIS_LIBRARIES} ${TORCH_LIBRARIES})
	target_include_directories(${PROJECT_NAME}_torch PRIVATE ${EIS_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})
	target_compile_options(${PROJECT_NAME}_torch PRIVATE "-Wall" "-g")
endif()
