cmake_minimum_required(VERSION 3.10)

project(orbvisense_navigator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(ORB_SLAM3_ROOT
    "$ENV{ORB_SLAM3_ROOT}"
)

if(NOT ORB_SLAM3_ROOT)
    message(FATAL_ERROR
        "ORB_SLAM3_ROOT no está definido. "
        "Use -DORB_SLAM3_ROOT=/ruta/a/ORB_SLAM3_fork"
    )
endif()

find_package(Qt5 REQUIRED COMPONENTS
    Widgets
    Network
    WebSockets
)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenSSL REQUIRED)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    geometry_msgs
)

include_directories(
    ${ORB_SLAM3_ROOT}
    ${ORB_SLAM3_ROOT}/include
    ${ORB_SLAM3_ROOT}/Thirdparty
    ${ORB_SLAM3_ROOT}/Thirdparty/DBoW2
    ${ORB_SLAM3_ROOT}/Thirdparty/g2o
    ${ORB_SLAM3_ROOT}/Thirdparty/Sophus
    ${ORB_SLAM3_ROOT}/include/CameraModels
    ${ORB_SLAM3_ROOT}/include/CameraModels/Pinhole

    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}

    ${catkin_INCLUDE_DIRS}
)

add_executable(
    orbvisense_navigator

    main.cpp

    MainWindow.h
    MainWindow.cpp

    AtlasEditor.h
    AtlasEditor.cpp

    NavigationWidget.h
    NavigationWidget.cpp
)

target_link_libraries(
    orbvisense_navigator

    Qt5::Widgets
    Qt5::Network
    Qt5::WebSockets

    ${ORB_SLAM3_ROOT}/lib/libORB_SLAM3.so

    ${OpenCV_LIBS}

    ${ORB_SLAM3_ROOT}/Thirdparty/DBoW2/lib/libDBoW2.so
    ${ORB_SLAM3_ROOT}/Thirdparty/g2o/lib/libg2o.so

    /usr/local/lib/libpangolin.so
    /lib/x86_64-linux-gnu/libOpenGL.so.0
    /lib/x86_64-linux-gnu/libGLEW.so.2.1

    boost_serialization
    ${OPENSSL_CRYPTO_LIBRARY}

    ${catkin_LIBRARIES}

    pthread
)
