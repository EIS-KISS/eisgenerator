cmake_minimum_required(VERSION 3.20)

project(ehigeneratorsa)

set(SRC_FILES
	main.cpp
	cap.cpp
	resistor.cpp
	constantphase.cpp
	model.cpp
	tokenize.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SPICE REQUIRED ngspice)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${SPICE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SPICE_CFLAGS} .)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++20 -Wall -O3 -march=native -g" LINK_FLAGS "-flto")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
