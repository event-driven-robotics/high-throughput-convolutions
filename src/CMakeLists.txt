# requires minimum cmake version
cmake_minimum_required(VERSION 3.5)

#produce the cmake var PROJECT_NAME
project(paper_convolution)

set(CONTEXT_DIR paper_convolution)

include(GNUInstallDirs)
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()

#needed packages
find_package(YCM REQUIRED)
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
                          INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
                          USE_LINK_PATH)

find_package(YARP COMPONENTS os sig math dev eigen REQUIRED)
find_package(event-driven REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(VTK REQUIRED)

add_subdirectory(refConv)
add_subdirectory(liteConv)

#message(WARNING  ${EVENT-DRIVEN_APPLICATIONS_INSTALL_DIR})
install(FILES ${PROJECT_SOURCE_DIR}/app/convolutions.xml DESTINATION ${EVENT-DRIVEN_APPLICATIONS_INSTALL_DIR})
