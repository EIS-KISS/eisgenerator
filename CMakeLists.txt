cmake_minimum_required(VERSION 3.20)

project(eisgenerator)

set(SRC_FILES
	componant.cpp
	cap.cpp
	resistor.cpp
	constantphase.cpp
	warburg.cpp
	model.cpp
	tokenize.cpp
	paralellseriel.cpp
)

set(API_HEADERS
	eisgenerator/componant.h
	eisgenerator/cap.h
	eisgenerator/resistor.h
	eisgenerator/constantphase.h
	eisgenerator/warburg.h
	eisgenerator/model.h
	eisgenerator/tokenize.h
	eisgenerator/paralellseriel.h
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++20 -Wall -O3 -march=native -g" LINK_FLAGS "-flto")

set(CMAKE_INSTALL_PREFIX "/usr")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${API_HEADERS} DESTINATION include/${PROJECT_NAME})

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.cpp)
set(LIBS_TEST -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_test ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_test ${LIBS_TEST})
target_include_directories(${PROJECT_NAME}_test PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME}_test PROPERTIES COMPILE_FLAGS "-std=c++20 -Wall -O3 -march=native -g" LINK_FLAGS "-flto")

install(TARGETS ${PROJECT_NAME}_test DESTINATION bin)
