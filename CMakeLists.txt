cmake_minimum_required(VERSION 3.23)

project(eisgenerator LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 20)

option(PROFILE_ENABLED "instrument for gprof" OFF)

set(CMAKE_PROJECT_VERSION_MAJOR 2)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)
add_compile_definitions(VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_compile_definitions(VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_compile_definitions(VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

find_package(Doxygen)

set(SRC_FILES
	componant/componant.cpp
	componant/cap.cpp
	componant/inductor.cpp
	componant/resistor.cpp
	componant/constantphase.cpp
	componant/warburg.cpp
	componant/paralellseriel.cpp
	componant/tro.cpp
	componant/trc.cpp
	model.cpp
	log.cpp
	normalize.cpp
	basicmath.cpp
	normalize.cpp
	basicmath.cpp
	eistype.cpp
	spectra.cpp
	strops.cpp
	translators.cpp
	randomgen.cpp
	compcache.cpp
	linearregession.cpp
)

set(API_HEADERS_CPP_DIR eisgenerator/)

set(API_HEADERS_CPP
	${API_HEADERS_CPP_DIR}/componant/componant.h
	${API_HEADERS_CPP_DIR}/componant/cap.h
	${API_HEADERS_CPP_DIR}/componant/resistor.h
	${API_HEADERS_CPP_DIR}/componant/constantphase.h
	${API_HEADERS_CPP_DIR}/componant/warburg.h
	${API_HEADERS_CPP_DIR}/componant/inductor.h
	${API_HEADERS_CPP_DIR}/componant/tro.h
	${API_HEADERS_CPP_DIR}/componant/trc.h
	${API_HEADERS_CPP_DIR}/componant/paralellseriel.h
	${API_HEADERS_CPP_DIR}/model.h
	${API_HEADERS_CPP_DIR}/log.h
	${API_HEADERS_CPP_DIR}/basicmath.h
	${API_HEADERS_CPP_DIR}/eistype.h
	${API_HEADERS_CPP_DIR}/spectra.h
	${API_HEADERS_CPP_DIR}/normalize.h
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
	include_directories(./dlfcn-win32)

	set(SRC_FILES ${SRC_FILES}
		compile-win.cpp
		./dlfcn-win32/dlfcn.c
	)

	set(COMMON_LINK_FLAGS "-flto -dl")

	if(NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package-win.sh ${CMAKE_CURRENT_BINARY_DIR}/package.sh @ONLY)
		add_custom_target(package
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/package.sh
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Createing release archive"
			VERBATIM)
	endif()

else()
	message("Building on UNIX")

	set(SRC_FILES ${SRC_FILES}
		compile.cpp
	)

	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
	endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

	set(COMMON_LINK_FLAGS "-flto -ltbb -pthread")
endif(WIN32)

set(COMMON_COMPILE_FLAGS "-Wall -O2 -march=native -g")

if(PROFILE_ENABLED)
	message("Profileing enabled")
	set(COMMON_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -pg -fno-omit-frame-pointer")
	set(COMMON_LINK_FLAGS "${COMMON_LINK_FLAGS} -pg -fno-omit-frame-pointer")
endif(PROFILE_ENABLED)

add_library(${PROJECT_NAME}_obj OBJECT ${SRC_FILES} ${API_HEADERS_CPP})
target_link_libraries(${PROJECT_NAME}_obj ${LIBS})
target_sources(${PROJECT_NAME}_obj INTERFACE FILE_SET header_set TYPE HEADERS BASE_DIRS ${API_HEADERS_CPP_DIR} FILES ${API_HEADERS_CPP})
target_include_directories(${PROJECT_NAME}_obj PUBLIC eisgenerator .)
set_target_properties(${PROJECT_NAME}_obj PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(${PROJECT_NAME}_obj PROPERTIES COMPILE_FLAGS ${COMMON_COMPILE_FLAGS} LINK_FLAGS ${COMMON_LINK_FLAGS})
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${COMMON_LINK_FLAGS})
add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
set_target_properties(${PROJECT_NAME}_static PROPERTIES LINK_FLAGS ${COMMON_LINK_FLAGS})

if(WIN32)
	set(EISGEN_LINK_FLAG ${PROJECT_NAME}_static)
else()
	set(EISGEN_LINK_FLAG ${PROJECT_NAME})
endif(WIN32)

configure_file(pkgconfig/libeisgenerator.pc.in pkgconfig/libeisgenerator.pc @ONLY)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(TARGETS ${PROJECT_NAME}_static DESTINATION lib)
install(TARGETS ${PROJECT_NAME}_obj FILE_SET header_set DESTINATION include/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/libeisgenerator.pc DESTINATION lib/pkgconfig)
install(FILES kissplotcsv DESTINATION bin PERMISSIONS WORLD_EXECUTE WORLD_READ
OWNER_READ OWNER_WRITE OWNER_EXECUTE
GROUP_READ GROUP_EXECUTE )

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP test.cpp)
set(LIBS_TEST -L. -l${EISGEN_LINK_FLAG})
add_executable(${PROJECT_NAME}_test ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_test ${LIBS_TEST})
target_include_directories(${PROJECT_NAME}_test PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME}_test PROPERTIES COMPILE_FLAGS ${COMMON_COMPILE_FLAGS} LINK_FLAGS ${COMMON_LINK_FLAGS})
install(TARGETS ${PROJECT_NAME}_test DESTINATION bin)

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.cpp)
set(LIBS_TEST -L. -l${EISGEN_LINK_FLAG})
add_executable(${PROJECT_NAME}_export ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_export ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_export ${LIBS_TEST})
target_include_directories(${PROJECT_NAME}_export PUBLIC eisgenerator)
set_target_properties(${PROJECT_NAME}_export PROPERTIES COMPILE_FLAGS ${COMMON_COMPILE_FLAGS} LINK_FLAGS ${COMMON_LINK_FLAGS})
install(TARGETS ${PROJECT_NAME}_export DESTINATION bin)

if (DOXYGEN_FOUND)
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/libeisgenerator.doxygen.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/libeisgenerator.doxygen)
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

	set(MAINPAGE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/mainpage.md.in)
	set(MAINPAGE_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/mainpage.md)
	configure_file(${MAINPAGE_IN} ${MAINPAGE_OUT} @ONLY)

	set(MODELS_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/modelspage.md.in)
	set(MODELS_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/modelspage.md)
	configure_file(${MODELS_IN} ${MODELS_OUT} @ONLY)

	set(README_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/readme.md.in)
	set(README_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/readme.md)
	configure_file(${README_IN} ${README_OUT} @ONLY)

	message("Doxygen build started")

	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM )
else (DOXYGEN_FOUND)
	message("Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
