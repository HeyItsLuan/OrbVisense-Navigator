# THIS IS THE ACTUAL COMPLETE REINSTALLATION GUIDE

**ORB-SLAM3 FORK + ROS1 + ROSBRIDGE + JPEG_TO_MONO + OrbVIsense Navigator**

This guide contains the complete procedure for installing **OrbVIsense Navigator** from scratch on **Ubuntu 20.04**, using **ROS 1 Noetic** and the **ORB-SLAM3 fork** used by the project.

Before starting, the following are required:

* **Ubuntu 20.04**
* **ROS 1 Noetic Desktop Full**
* The dependencies required to compile ORB-SLAM3, ROS, and OrbVIsense Navigator.
* OpenCV 4.4.0
* Pangolin v0.6

If the system does not yet have ROS 1 Noetic or the required dependencies, first complete the entire **Pre-installation** section.

## Pre-installation

### Installing ROS 1 Noetic on Ubuntu 20.04 (Desktop Full)

#### Register the repository and keys

```bash
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu focal main" > /etc/apt/sources.list.d/ros-latest.list'

sudo apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' \
--recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

sudo apt update
```

### 1. Change the Ubuntu download mirror

```bash
sudo sed -i 's/co.archive.ubuntu.com/archive.ubuntu.com/g' /etc/apt/sources.list
```

### 2. Enable the required repositories

```bash
sudo add-apt-repository universe -y
sudo add-apt-repository restricted -y
sudo add-apt-repository multiverse -y
```

### 3. Clean and update the repositories

```bash
sudo apt clean
sudo apt update
```

### 4. Install ROS Noetic

```bash
sudo apt --fix-broken install -y
sudo apt install -y ros-noetic-desktop-full
```

If the installation fails, try:

```bash
sudo apt update
sudo apt install -y --fix-missing ros-noetic-desktop-full
```

### 5. Finish configuring ROS

Add ROS Noetic to the environment:

```bash
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

Install the required tools:

```bash
sudo apt install -y \
python3-rosdep \
python3-rosinstall \
python3-rosinstall-generator \
python3-wstool \
build-essential
```

Initialize `rosdep`:

```bash
sudo rosdep init
rosdep update
```

### 6. BASIC DEPENDENCIES

```bash
sudo apt update

sudo apt install -y \
build-essential \
cmake \
unzip \
pkg-config \
libgtk2.0-dev \
libgtk-3-dev \
libavcodec-dev \
libavformat-dev \
libswscale-dev \
libv4l-dev \
libxvidcore-dev \
libx264-dev \
libjpeg-dev \
libpng-dev \
libtiff-dev \
gfortran \
libopenexr-dev \
libatlas-base-dev \
python3-dev \
python3-numpy
```

### 7. Prepare OpenCV 4.4

First check which OpenCV version is available through `pkg-config`:

```bash
pkg-config --modversion opencv4
```

There are two possible scenarios.

#### If it returns `4.2.0`

Run:

```bash
dpkg-query -W -f='${binary:Package}\t${Version}\n' \
'libopencv*' \
'opencv*' \
'python3-opencv' 2>/dev/null |
awk '$2 ~ /^4\.2\.0/ {print $1}' |
xargs -r sudo apt purge -y
```

Then:

```bash
sudo apt autoremove -y
sudo ldconfig
```

#### If `pkg-config` reports that `opencv4` does not exist

Do not uninstall anything and continue directly with the installation of OpenCV 4.4.0.

#### Download OpenCV 4.4.0

Now, regardless of the previous scenario:

```bash
cd "$HOME/Escritorio"

wget -O opencv-4.4.0.zip \
https://github.com/opencv/opencv/archive/4.4.0.zip
```

Check that the file was downloaded correctly:

```bash
ls -lh "$HOME/Escritorio/opencv-4.4.0.zip"
```

Extract it:

```bash
unzip -o "$HOME/Escritorio/opencv-4.4.0.zip"
```

Enter the source directory:

```bash
cd "$HOME/Escritorio/opencv-4.4.0"
```

Configure a clean build:

```bash
rm -rf build
mkdir build
cd build
```

Configure CMake:

```bash
cmake .. \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=/usr/local
```

Compile using a single core to avoid overloading the system:

```bash
make -j1
```

Install:

```bash
sudo make install
sudo ldconfig
```

Check the installed version:

```bash
grep -n "OpenCV_VERSION" \
/usr/local/lib/cmake/opencv4/OpenCVConfig-version.cmake
```

It should return:

```text
4.4.0
```

### 8. Pangolin

Clone specifically version **v0.6**:

```bash
cd "$HOME"

rm -rf "$HOME/Pangolin"

git clone --branch v0.6 --depth 1 \
https://github.com/stevenlovegrove/Pangolin.git \
"$HOME/Pangolin"
```

Enter the directory:

```bash
cd "$HOME/Pangolin"
```

Prepare a clean build:

```bash
rm -rf build
mkdir build
cd build
```

Configure:

```bash
cmake .. \
-DCMAKE_BUILD_TYPE=Release \
-DBUILD_PANGOLIN_PYTHON=OFF
```

Compile:

```bash
make -j1
```

Install:

```bash
sudo make install
sudo ldconfig
```

After completing all the changes and dependencies above, it is possible to continue with the complete installation of the **ORB-SLAM3 fork**, provided that the system did not already have the **ROS 1 Noetic environment on Ubuntu 20.04** and its dependencies.

# 1. REINSTALLATION PACKAGE LOCATION

Download the `OrbVisense-Navigator` folder to the Desktop.

```bash
cd "$HOME/Escritorio"

git clone https://github.com/HeyItsLuan/OrbVisense-Navigator.git
```

The folder must be located at:

```text
$HOME/Escritorio/OrbVisense-Navigator
```

Contents:

```text
.
./ORB_SLAM3_fork_src_mod
./ORB_SLAM3_fork_src_mod/src
./dataset_to_rosbag.py
./jpeg_to_mono
./jpeg_to_mono/jpeg_to_mono
./orbvisense_navigator
./robot_pwm
./robot_pwm/robot_pwm
```

# 2. ENVIRONMENT USED

System:

```text
Ubuntu 20.04
```

ROS:

```text
ROS1 Noetic
```

Workspace:

```text
$HOME/ros1_ws
```

# 3. COMPILATION RULE

**IMPORTANT:**

Always use:

```bash
make -j1
```

and:

```bash
catkin_make -j1
```

The `-j1` parameter means that compilation will use a single process.

Even if the computer can handle parallel compilation, **always use `-j1` during the installation and compilation of the project** to avoid overloading system resources and reduce the risk of the computer freezing.

DO NOT use:

```bash
make -j4
make -j$(nproc)
catkin_make -j4
```

If a `build.sh` file uses:

```bash
make -j4
```

change it to:

```bash
make -j1
```

To edit it:

```bash
gedit "$HOME/Escritorio/ORB_SLAM3_fork/build.sh"
```

# 4. INSTALL ORB-SLAM3 FORK

This is the first actual installation step.

### 4.1. Clone the ORB-SLAM3 fork

```bash
cd "$HOME/Escritorio"

git clone https://github.com/Lab-of-AI-and-Robotics/ORB_SLAM3.git ORB_SLAM3_fork
```

This creates:

```text
$HOME/Escritorio/ORB_SLAM3_fork
```

### 4.2. Copy the OrbVIsense Navigator modifications

The `OrbVisense-Navigator` repository contains the modified `.cc` files for the ORB-SLAM3 fork.

Copy the files:

```bash
cp "$HOME/Escritorio/OrbVisense-Navigator/ORB_SLAM3_fork_src_mod/src/"*.cc \
   "$HOME/Escritorio/ORB_SLAM3_fork/src/"
```

The modified files are:

```text
FrameDrawer.cc
ImuTypes.cc
MapDrawer.cc
Optimizer.cc
Tracking.cc
```

The OrbVIsense Navigator repository contains only these modifications inside:

```text
ORB_SLAM3_fork_src_mod/src/
```

### 4.3. Verify the OpenCV configuration

The main ORB-SLAM3 `CMakeLists.txt` must use **OpenCV 4.4**.

Check which version it currently requests:

```bash
grep -n "find_package(OpenCV" \
"$HOME/Escritorio/ORB_SLAM3_fork/CMakeLists.txt"
```

It should show:

```cmake
find_package(OpenCV 4.4)
```

If another version appears, open the file:

```bash
gedit "$HOME/Escritorio/ORB_SLAM3_fork/CMakeLists.txt"
```

and change only the OpenCV version so that it contains:

```cmake
find_package(OpenCV 4.4)
```

### 4.4. Clean previous builds

Before recompiling, remove previous libraries and build directories:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork"

rm -rf lib/*
rm -rf Thirdparty/DBoW2/lib/*
rm -rf Thirdparty/DBoW2/build
rm -rf Thirdparty/g2o/config.h
rm -rf Thirdparty/g2o/build
rm -rf build
```

### 4.5. Compile DBoW2

Enter DBoW2:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/DBoW2"
```

Clean and create the build directory:

```bash
rm -rf build
mkdir build
cd build
```

Configure:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Compile using a single process:

```bash
make -j1
```

### 4.6. Compile g2o

Enter g2o:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/g2o"
```

Clean and create the build directory:

```bash
rm -rf build
mkdir build
cd build
```

Configure:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Compile using a single process:

```bash
make -j1
```

### 4.7. Configure the ORB-SLAM3 fork to use C++14

Open the main `CMakeLists.txt`:

```bash
gedit "$HOME/Escritorio/ORB_SLAM3_fork/CMakeLists.txt"
```

The C++ standard configuration must use C++14:

```cmake
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    add_definitions(-DCOMPILEDWITHC14)
    message(STATUS "Using flag -std=c++14.")
```

Additionally, remove the RealSense-related block beginning with:

```cmake
# If RealSense SDK is found the library is added and its examples compiled
```

and continuing to the end of the file.

The comment is used as a reference to locate the block rather than relying on a specific line number.

### 4.8. Compile the ORB-SLAM3 fork

Clean the build:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork"

rm -rf build
mkdir build
cd build
```

Configure:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Compile using a single process:

```bash
make -j1
```

The compilation must finish successfully and generate the ORB-SLAM3 libraries.

# 5. PREPARE ROS1 WORKSPACE

Create the workspace:

```bash
mkdir -p "$HOME/ros1_ws/src"
```

Load ROS:

```bash
source /opt/ros/noetic/setup.bash
```

Initialize the workspace:

```bash
cd "$HOME/ros1_ws/src"
catkin_init_workspace
```

Initially, the workspace should contain only the `CMakeLists.txt` generated by `catkin`.

# 6. INSTALL ORB_SLAM3_ROS_WRAPPER

## 6.1. Download the repository

```bash
cd "$HOME/ros1_ws/src"

git clone https://github.com/thien94/orb_slam3_ros_wrapper.git
```

## 6.2. Edit the ORB-SLAM3 path

Open:

```bash
gedit "$HOME/ros1_ws/src/orb_slam3_ros_wrapper/CMakeLists.txt"
```

Change only:

```cmake
set(ORB_SLAM3_DIR
   $ENV{HOME}/Packages/ORB_SLAM3
)
```

to:

```cmake
set(ORB_SLAM3_DIR
   $ENV{HOME}/Escritorio/ORB_SLAM3_fork
)
```

## 6.3. Install ROS dependencies for the wrapper

```bash
sudo apt update

sudo apt install -y \
  ros-noetic-cv-bridge \
  ros-noetic-image-transport \
  ros-noetic-tf \
  ros-noetic-message-runtime \
  ros-noetic-sensor-msgs \
  ros-noetic-std-msgs \
  ros-noetic-roscpp \
  ros-noetic-rospy
```

## 6.4. Prepare the vocabulary

Extract the ORB-SLAM3 vocabulary:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork/Vocabulary"

tar -xf ORBvoc.txt.tar.gz
```

Copy it to the wrapper:

```bash
cp ORBvoc.txt \
"$HOME/ros1_ws/src/orb_slam3_ros_wrapper/config/ORBvoc.txt"
```

## 6.5. Compile the wrapper

Clean the previous build:

```bash
cd "$HOME/ros1_ws"

rm -rf build devel
```

Load ROS:

```bash
source /opt/ros/noetic/setup.bash
```

Always compile using a single process:

```bash
catkin_make -j1 \
-DOpenCV_DIR=/usr/local/lib/cmake/opencv4
```

Load the workspace:

```bash
source "$HOME/ros1_ws/devel/setup.bash"
```

# 7. INSTALL jpeg_to_mono

Copy the package from the **OrbVIsense Navigator** repository:

```bash
cd "$HOME/ros1_ws/src"

cp -r "$HOME/Escritorio/OrbVisense-Navigator/jpeg_to_mono/jpeg_to_mono" .
```

Check:

```bash
ls -lah "$HOME/ros1_ws/src/jpeg_to_mono"
```

It must be located at:

```text
$HOME/ros1_ws/src/jpeg_to_mono
```

Compile:

```bash
cd "$HOME/ros1_ws"

source /opt/ros/noetic/setup.bash

catkin_make -j1 \
-DOpenCV_DIR=/usr/local/lib/cmake/opencv4
```

# 8. INSTALL rosbridge_suite

Download `rosbridge_suite`:

```bash
cd "$HOME/ros1_ws/src"

git clone --branch ros1 --depth 1 \
https://github.com/RobotWebTools/rosbridge_suite.git
```

Install the required dependency:

```bash
sudo apt update

sudo apt install -y ros-noetic-rosauth
```

Compile:

```bash
cd "$HOME/ros1_ws"

source /opt/ros/noetic/setup.bash

catkin_make -j1
```

# 9. LOAD THE WORKSPACE

Load ROS:

```bash
source /opt/ros/noetic/setup.bash
```

Load the workspace:

```bash
source "$HOME/ros1_ws/devel/setup.bash"
```

# 10. CHECK ROS PACKAGES

Load the environments:

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
```

Check `orb_slam3_ros_wrapper`:

```bash
rospack find orb_slam3_ros_wrapper
```

It should return:

```text
$HOME/ros1_ws/src/orb_slam3_ros_wrapper
```

Check `jpeg_to_mono`:

```bash
rospack find jpeg_to_mono
```

It should return:

```text
$HOME/ros1_ws/src/jpeg_to_mono
```

Check `rosbridge_server`:

```bash
rospack find rosbridge_server
```

It should return:

```text
$HOME/ros1_ws/src/rosbridge_suite/rosbridge_server
```

# 11. INSTALLATION VERIFICATION

Check that the ORB-SLAM3 library exists:

```bash
ls -lh "$HOME/Escritorio/ORB_SLAM3_fork/lib/libORB_SLAM3.so"
```

Check the wrapper executable:

```bash
ls -lh "$HOME/ros1_ws/devel/lib/orb_slam3_ros_wrapper/orb_slam3_ros_wrapper_mono_inertial"
```

Check that the wrapper uses the fork library:

```bash
ldd "$HOME/ros1_ws/devel/lib/orb_slam3_ros_wrapper/orb_slam3_ros_wrapper_mono_inertial" | grep ORB_SLAM3
```

It should point to:

```text
$HOME/Escritorio/ORB_SLAM3_fork/lib/libORB_SLAM3.so
```

Check all packages:

```bash
rospack find orb_slam3_ros_wrapper
rospack find jpeg_to_mono
rospack find rosbridge_server
rospack find robot_pwm
```

Check the new ROS message:

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"

rosmsg show robot_pwm/PWM
```

It should show:

```text
int16 left
int16 right
```

You can also check that the message appears in ROS:

```bash
rosmsg list | grep robot_pwm
```

It should show:

```text
robot_pwm/PWM
```

Check the nodes:

```bash
rosnode list
```

Check the topics:

```bash
rostopic list
```

The topic used by the editor to send robot movement commands is:

```text
/robot/pwm
```

# 12. MAIN TOPICS

### Compressed image

```text
/cam0/image_raw/compressed
```

### Converted image

```text
/cam0/image_raw
```

### IMU

```text
/imu0
```

### Camera pose

```text
/orb_slam3/camera_pose
```

### Map

```text
/orb_slam3/map_points
```

The map uses the type:

```text
sensor_msgs/PointCloud2
```

The frame used by ORB-SLAM3 is:

```text
world
```

# 13. CHECK TOPICS

Complete list:

```bash
rostopic list
```

Check image frequency:

```bash
rostopic hz /cam0/image_raw
```

Check IMU frequency:

```bash
rostopic hz /imu0
```

Check the pose:

```bash
rostopic echo /orb_slam3/camera_pose
```

Check the map:

```bash
rostopic echo /orb_slam3/map_points
```

# 14. EXECUTION TERMINALS

## Terminal 1 — ROSCORE

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
roscore
```

This terminal keeps the ROS core active.

## Terminal 2 — ROSBRIDGE

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
roslaunch rosbridge_server rosbridge_websocket.launch
```

This enables communication through WebSocket.

## Terminal 3 — jpeg_to_mono

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
rosrun jpeg_to_mono jpeg_to_mono_node
```

Its function is to convert:

```text
/cam0/image_raw/compressed
```

into:

```text
/cam0/image_raw
```

## Terminal 4 — ORB-SLAM3 MONO-INERTIAL

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
roslaunch orb_slam3_ros_wrapper euroc_monoimu.launch
```

The system uses:

Image:

```text
/cam0/image_raw
```

IMU:

```text
/imu0
```

# 15. KEY FILE LOCATIONS

## Configuration files and map locations

### YAML configuration file

The `.yaml` files used by the wrapper are located in:

```text
~/ros1_ws/src/orb_slam3_ros_wrapper/config/
```

The corresponding YAML file for the camera and IMU configuration can be edited or replaced there.

To edit one:

```bash
gedit "$HOME/ros1_ws/src/orb_slam3_ros_wrapper/config/NOMBRE_CONFIGURACION.yaml"
```

### `.osa` map files

Maps saved by ORB-SLAM3 are normally stored in:

```text
~/.ros/
```

To locate the maps:

```bash
find ~/.ros -maxdepth 1 -type f -name "*.osa"
```

The `~/.ros` directory belongs to the user, not to the workspace.

Therefore, `.osa` maps **are not deleted when `~/ros1_ws` is deleted or rebuilt**.

In summary:

```text
~/ros1_ws/src/orb_slam3_ros_wrapper/config/
└── YAML configuration files

~/.ros/
└── .osa map/Atlas files
```

**Practical rule:** `.yaml` files are modified inside the wrapper's `config` directory; `.osa` files are searched for and managed inside `~/.ros`.

# 16. ADAPTED SYSTEM SUMMARY

### ORB-SLAM3 fork

```text
$HOME/Escritorio/ORB_SLAM3_fork
```

### ROS workspace

```text
$HOME/ros1_ws
```

### Wrapper

```text
$HOME/ros1_ws/src/orb_slam3_ros_wrapper
```

### JPEG converter

```text
$HOME/ros1_ws/src/jpeg_to_mono
```

### Rosbridge

```text
$HOME/ros1_ws/src/rosbridge_suite
```

# 17. INSTALLING ORBVISENSE NAVIGATOR

## 17.1. Installing the ROS `robot_pwm` message

Copy the package from the OrbVIsense Navigator repository:

```bash
cd "$HOME/ros1_ws/src"

cp -r "$HOME/Escritorio/OrbVisense-Navigator/robot_pwm/robot_pwm" .
```

Compile:

```bash
cd "$HOME/ros1_ws"

source /opt/ros/noetic/setup.bash

catkin_make -j1
```

Load the workspace:

```bash
source "$HOME/ros1_ws/devel/setup.bash"
```

Verify the package:

```bash
rospack find robot_pwm
```

It should return:

```text
$HOME/ros1_ws/src/robot_pwm
```

Verify the message contents:

```bash
rosmsg show robot_pwm/PWM
```

It should show:

```text
int16 left
int16 right
```

## 17.2. Installing OrbVIsense Navigator

Copy the program from the repository:

```bash
cp -r "$HOME/Escritorio/OrbVisense-Navigator/orbvisense_navigator" \
      "$HOME/Escritorio/"
```

Compile:

```bash
cd "$HOME/Escritorio/orbvisense_navigator"

export ORB_SLAM3_ROOT="$HOME/Escritorio/ORB_SLAM3_fork"

rm -rf build

cmake -S "$HOME/Escritorio/orbvisense_navigator" \
      -B "$HOME/Escritorio/orbvisense_navigator/build" \
      -DCMAKE_BUILD_TYPE=Release \
      -DOpenCV_DIR=/usr/local/lib/cmake/opencv4

cmake --build "$HOME/Escritorio/orbvisense_navigator/build" -j1
```

Before running the program, load ROS and the workspace:

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
```

Define the ORB-SLAM3 location:

```bash
export ORB_SLAM3_ROOT="$HOME/Escritorio/ORB_SLAM3_fork"
```

Configure the required libraries:

```bash
export LD_LIBRARY_PATH="$HOME/Escritorio/ORB_SLAM3_fork/lib:$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/DBoW2/lib:$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/g2o/lib:/usr/local/lib:/opt/ros/noetic/lib:${LD_LIBRARY_PATH:-}"
```

The command to start OrbVIsense Navigator is:

```bash
"$HOME/Escritorio/orbvisense_navigator/build/orbvisense_navigator"
```

## 17.3. Execution commands

### Terminal 1 — ROSCORE

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"

roscore
```

### Terminal 2 — OrbVIsense Navigator

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"

export ORB_SLAM3_ROOT="$HOME/Escritorio/ORB_SLAM3_fork"

export LD_LIBRARY_PATH="$HOME/Escritorio/ORB_SLAM3_fork/lib:$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/DBoW2/lib:$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/g2o/lib:/usr/local/lib:/opt/ros/noetic/lib:${LD_LIBRARY_PATH:-}"

"$HOME/Escritorio/orbvisense_navigator/build/orbvisense_navigator"
```

# 18. ALLOWED EDITS

## 18.1. Modifying the robot size

To modify the **robot size**, edit:

```bash
gedit "$HOME/Escritorio/orbvisense_navigator/NavigationWidget.h"
```

Find:

```cpp
static constexpr float mRobotScale = 0.10f;
```

Modify the value according to the desired size.

## 18.2. Modifying the PWM message transmission IP

To change the WebSocket IP address where the PWM message is published, edit:

```bash
gedit "$HOME/Escritorio/orbvisense_navigator/MainWindow.cpp"
```

Find:

```cpp
<< "ws://10.42.0.1:9090";
```

Modify the IP address according to the network configuration.

## 18.3. Recompilation

After any code modification:

```bash
cd "$HOME/Escritorio/orbvisense_navigator"

cmake --build "$HOME/Escritorio/orbvisense_navigator/build" -j1
```

After recompiling, run again:

```bash
"$HOME/Escritorio/orbvisense_navigator/build/orbvisense_navigator"
```

# 19. GENERAL OPERATION OF ORBVISENSE NAVIGATOR

## 19.1. Requirements

To fully use **OrbVIsense Navigator**, the following elements are required.

### 1. Android phone

An Android phone with the **OrbVIsense** application installed is required.

The application is available in the **HeyItsLuan/OrbVIsense** GitHub repository.

### 2. Phone calibration

The phone must be calibrated beforehand.

The application itself can generate the datasets required to perform the calibration.

The calibration procedure is available in the **HeyItsLuan/OrbVIsense-calibration** GitHub repository.

Calibration must be performed before using the phone to obtain reliable results with ORB-SLAM3.

### 3. Network connection

The Android phone and the computer must be connected to the **same network**.

The computer's IP address must be known because the computer acts as the **server** for communication between the phone and ROS.

### 4. Robot and WebSocket communication

The robot must have the computer's IP address configured in its code.

The robot code is available in the **HeyItsLuan/OrbVIsense-robot** GitHub repository.

The IP configured in the robot code must correspond to the IP address of the computer acting as the server.

Additionally, the ESP32 must be in WebSocket communication mode.

To activate this mode, press the **BOOT** button on the ESP32.

## 19.2. Using ORB-SLAM3 independently

Once the previous steps have been completed, the **ORB-SLAM3 fork** can be used with the Android phone following this architecture:

```text
Android Phone
       │
       │ WebSocket
       ▼
   Computer
       │
       ▼
      ROS1
       │
       ▼
   ORB-SLAM3
```

The system can be used independently through **step 14** of this guide.

At this point, ORB-SLAM3 can be executed, the camera and IMU data from the phone can be received, and maps can be generated without using OrbVIsense Navigator.

## 19.3. Generating maps for navigation

If, in addition to using ORB-SLAM3, **autonomous navigation** is required, continue with the installation through OrbVIsense Navigator.

A map must already be available in order to navigate.

Maps can be generated using ORB-SLAM3 from step 14 of this guide or by using the system together with OrbVIsense Navigator.

To generate different maps or modify the configuration used by ORB-SLAM3, the following file can be modified:

```text
$HOME/ros1_ws/src/orb_slam3_ros_wrapper/config/euroc.yaml
```

Maps generated by ORB-SLAM3 must be saved in:

```text
.osa
```

format.

These files correspond to the Atlas files saved by ORB-SLAM3 and are the files that OrbVIsense Navigator can subsequently load.

# 19.4. Navigation workflow in OrbVIsense Navigator

Once a `.osa` map has been generated, OrbVIsense Navigator can be used for navigation.

### Step 1. Load the map

In OrbVIsense Navigator, use the **Load** button.

Select the `.osa` map that you want to use.

After selecting the file, wait a few seconds while the map is loaded.

### Step 2. Enable navigation

Once the map has been loaded, the **Navigation** option becomes available.

Press the button to access the navigation tools.

### Step 3. Configure the robot IP

Before starting navigation, verify the configuration described in section **18.2**.

The IP address used to publish the PWM message must correspond to the address of the computer acting as the WebSocket server.

## 19.5. Calibrating the robot representation on the map

When a map is loaded, OrbVIsense Navigator initially represents the robot at:

```text
X = 0.0
Y = 0.0
```

The initial representation of the robot size may not exactly correspond to its real dimensions.

For this reason, a calibration procedure based on physical measurements is used.

### Required data

The following are required for this calibration:

* A real-world metric measurement.
* The distance between two known positions.
* The actual length of the robot.
* The coordinates of two valid poses obtained through ORB-SLAM3.

### Obtaining the two poses

First, enable ORB-SLAM3 using the:

**Enable ORB-SLAM3**

button.

Once ORB-SLAM3 has started, select:

**Localization**

Then physically move the robot across the map.

ORB-SLAM3 will provide the estimated robot pose.

To display this pose inside OrbVIsense Navigator, enable:

**Enable Reception**

This allows the navigator to receive the pose topic published by ORB-SLAM3 and draw the robot position on the map.

Two physically different robot positions that produce a **valid pose** must be selected.

A pose is considered valid when its coordinates are not:

```text
X = 0.0
Y = 0.0
```

The two positions must be recorded and physically measured.

### Calculating the robot scale

The following values will then be available:

* Real metric distance between the two positions.
* Distance between the two positions in map coordinates.
* Actual length of the robot.

The distance between the two map poses is calculated from their coordinates.

The representative robot size in points is then obtained using:

```text
Robot representation =
(Actual robot length × distance between the two poses in coordinates)
/
Real metric distance between the two poses
```

This produces the size that the program must use to correctly represent the robot within the map.

The calculated value can be used to modify the corresponding parameter described in section **18.1**.

## 19.6. Map editing tools

After loading a map, OrbVIsense Navigator provides tools to prepare the map before calculating a route.

### Select Contour

The **Select Contour** button allows a region of the map to be selected.

After activating this tool, click on the map to select an internal contour.

The selected contour can be used to define a region that will later be considered during route calculation.

The coordinates corresponding to the selected point are also displayed.

### Delete Selected

The **Delete Selected** button allows points to be removed from the map.

This tool is useful when there are:

* False points.
* Isolated points.
* Points that generate nonexistent contacts.
* Points that prevent an internal contour from being correctly defined.

Points can be selected using the left mouse button.

It is also possible to hold the left mouse button to select a region of points.

### Removing extreme Z points

The map used for navigation primarily corresponds to the:

```text
X-Y
```

plane.

Therefore, the `Z` coordinate, which represents height, is not required for two-dimensional navigation.

However, points with extreme `Z` values may appear as obstacles or false contacts when projected onto the map.

OrbVIsense Navigator allows these points to be removed using the corresponding controls.

Using the arrows progressively removes the values located to the right or left of the established limits.

This makes it possible to reduce extreme values of the `Z` coordinate while keeping only the information relevant to navigation in the `X-Y` plane.

## 19.7. Full system activation

The:

**Enable ORB-SLAM3**

button allows the components required by the system to be started simultaneously:

* WebSocket.
* `jpeg_to_mono`.
* ORB-SLAM3 fork.

Once enabled, the navigation tools section becomes available in the left panel.

### Enable Reception

The **Enable Reception** button allows the robot pose topic from ORB-SLAM3 to be received.

The panel displays information about:

* Reception status.
* Robot position.
* Robot orientation.

The received pose is represented directly on the loaded map.

## 19.8. Selecting point B

The tool for marking **point B** allows the user to select the location where the robot should travel on the map.

After activating this tool, click on the desired destination within the map.

Once point B has been selected, OrbVIsense Navigator displays its coordinates on screen.

## 19.9. Route calculation

With the robot localized and point B selected, use:

**Calculate Route**

The program calculates a route using:

* The map boundaries.
* The current robot position.
* The robot size.
* The selected point B.
* The regions available for movement.

If a valid route exists, it is drawn on the map.

If no valid route exists, the program reports that the destination cannot be reached from the current position.

## 19.10. Starting navigation

When a valid route exists, press:

**Start Route**

The robot will begin moving from its current position toward point B by following the calculated route.

During movement, the system uses the pose provided by ORB-SLAM3 to determine the robot's current position and orientation.

## 19.11. Tolerances and route recalculation

The robot uses tolerances to determine:

* When it has reached the destination.
* When it has deviated from the calculated route.
* When it needs to correct its trajectory.

If the robot deviates from the route, the system waits approximately **2 seconds** and calculates a new route from the current position.

If the robot loses a valid pose, the system attempts to recover a valid pose by moving the robot.

Once a valid pose has been recovered, the system can calculate the route again from the new position.

In this way, navigation can adapt to robot deviations and changes in pose estimation.

## Video Demonstration

A demonstration of the OrbVisense Navigator autonomous navigation system is available on YouTube:

[Watch the video demonstration](https://youtu.be/H9FZ5tJ7xmQ)
