cmake_minimum_required(VERSION 3.19)

project(eisgenerator LANGUAGES C CXX)

set(SRC_FILES
	componant.cpp
	cap.cpp
	inductor.cpp
	resistor.cpp
	constantphase.cpp
	warburg.cpp
	model.cpp
	paralellseriel.cpp
	log.cpp
	normalize.cpp
	basicmath.cpp
	log.cpp
	normalize.cpp
	basicmath.cpp
	finitetr.cpp
	eistype.cpp
	strops.cpp
	translators.cpp
	randomgen.cpp
	compile.cpp
)

set(API_HEADERS_CPP_DIR eisgenerator/)

set(API_HEADERS_CPP
	${API_HEADERS_CPP_DIR}/componant.h
	${API_HEADERS_CPP_DIR}/cap.h
	${API_HEADERS_CPP_DIR}/resistor.h
	${API_HEADERS_CPP_DIR}/constantphase.h
	${API_HEADERS_CPP_DIR}/warburg.h
	${API_HEADERS_CPP_DIR}/model.h
	${API_HEADERS_CPP_DIR}/log.h
	${API_HEADERS_CPP_DIR}/paralellseriel.h
	${API_HEADERS_CPP_DIR}/basicmath.h
	${API_HEADERS_CPP_DIR}/eistype.h
	${API_HEADERS_CPP_DIR}/normalize.h
	${API_HEADERS_CPP_DIR}/inductor.h
	${API_HEADERS_CPP_DIR}/translators.h
)

set(API_HEADERS_C_DIR eisgenerator/c/)

set(API_HEADERS_C ${API_HEADERS_CPP_DIR}/model.h)

message("Platform " ${CMAKE_SYSTEM_NAME})
if(WIN32)
	message("Building on Windows")
	file(GLOB_RECURSE ARGP_SRC ${PROJECT_SOURCE_DIR}/argp/*.*)
	set(SRC_FILES ${SRC_FILES} ${ARGP_SRC})
	include_directories(./argp)
else()
	message("Building on UNIX")
endif(WIN32)


add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${API_HEADERS_CPP})
target_link_libraries(${PROJECT_NAME} ${LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++2a -Wall -O2 -march=native -g" LINK_FLAGS "-flto -ltbb -pthread")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

configure_file(pkgconfig/libeisgenerator.pc.in pkgconfig/libeisgenerator.pc @ONLY)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${API_HEADERS_CPP} DESTINATION include/${PROJECT_NAME})
install(FILES ${API_HEADERS_CPP} DESTINATION include/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/libeisgenerator.pc DESTINATION lib/pkgconfig)
install(FILES eisgenerator_plot DESTINATION bin PERMISSIONS WORLD_EXECUTE WORLD_READ
OWNER_READ OWNER_WRITE OWNER_EXECUTE
GROUP_READ GROUP_EXECUTE)
install(FILES kissplotcsv DESTINATION bin PERMISSIONS WORLD_EXECUTE WORLD_READ
OWNER_READ OWNER_WRITE OWNER_EXECUTE
GROUP_READ GROUP_EXECUTE )

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP test.cpp)
set(LIBS_TEST -L. -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_test ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_test ${LIBS_TEST})
target_include_directories(${PROJECT_NAME}_test PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME}_test PROPERTIES COMPILE_FLAGS "-std=c++2a -Wall -O2 -march=native -g" LINK_FLAGS "-flto")
install(TARGETS ${PROJECT_NAME}_test DESTINATION bin)

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.cpp)
set(LIBS_TEST -L. -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_export ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_export ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_export ${LIBS_TEST})
target_include_directories(${PROJECT_NAME}_export PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME}_export PROPERTIES COMPILE_FLAGS "-std=c++2a -Wall -O2 -march=native -g" LINK_FLAGS "-flto")
install(TARGETS ${PROJECT_NAME}_export DESTINATION bin)
