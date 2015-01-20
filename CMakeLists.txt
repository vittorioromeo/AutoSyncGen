cmake_minimum_required(VERSION 2.8)
project(AutoSyncGen)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extlibs/SSVCMake/cmake/modules/;${CMAKE_MODULE_PATH}")
include(SSVCMake)

SSVCMake_setDefaults()
SSVCMake_findSFML()
SSVCMake_findExtlib(SSVUtils)
SSVCMake_findExtlib(SSVStart)

add_executable(${PROJECT_NAME} ${SRC_LIST})
SSVCMake_linkSFML()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/_RELEASE/)
