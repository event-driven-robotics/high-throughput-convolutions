################################################################################
#                                                                              #
# Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# @author: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>

project(liteConv)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

file(GLOB source *.cpp)
file(GLOB header *.h)

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${EIGEN3_INCLUDE_DIR}
                    ${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${source} ${header})

target_link_libraries(${PROJECT_NAME} PRIVATE YARP::YARP_os
                                              YARP::YARP_init
                                              ev::event-driven
                                              ${OpenCV_LIBRARIES}
                                              stdc++fs)

if(LOG GREATER_EQUAL 0 AND LOG LESS_EQUAL 2)
    message(AUTHOR_WARNING "Event logging is: " ${LOG})
    add_definitions(-DLOG=${LOG})
endif()

if(VIS STREQUAL "ON")
    message(AUTHOR_WARNING "Visualisation is: " ${VIS})
    add_definitions(-DVIS)
endif()

if(PROFILE STREQUAL "ON")
    #add_definitions(-DPROFILE)
    message(AUTHOR_WARNING "Code profilling is " ${PROFILE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-inline -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -fno-inline -Wall")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g -fno-inline -Wall")
    message(AUTHOR_WARNING "module pos {cxx; exe; shared}_flags: ${CMAKE_CXX_FLAGS}; ${CMAKE_EXE_LINKER_FLAGS}; ${CMAKE_SHARED_LINKER_FLAGS}")
endif()


install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
