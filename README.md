# ESTA ES LA VERDADERA GUÍA DE REINSTALACIÓN COMPLETA

**ORB-SLAM3 FORK + ROS1 + ROSBRIDGE + JPEG_TO_MONO + OrbVIsense Navigator**

Esta guía contiene el procedimiento completo para instalar **OrbVIsense Navigator** desde cero sobre **Ubuntu 20.04**, utilizando **ROS 1 Noetic** y el fork de **ORB-SLAM3** utilizado por el proyecto.

Antes de comenzar, es necesario contar con:

* **Ubuntu 20.04**
* **ROS 1 Noetic Desktop Full**
* Las dependencias necesarias para compilar ORB-SLAM3, ROS y OrbVIsense Navigator.
* OpenCV 4.4.0
* Pangolin v0.6

Si el sistema todavía no cuenta con ROS 1 Noetic ni con las dependencias necesarias, realizar primero toda la sección de **Preinstalación**.

## Preinstalación

### Instalación de ROS 1 Noetic en Ubuntu 20.04 (Desktop Full)

#### Registrar el repositorio y las claves

```bash
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu focal main" > /etc/apt/sources.list.d/ros-latest.list'

sudo apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' \
--recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

sudo apt update
```

### 1. Cambiar el espejo de descargas de Ubuntu

```bash
sudo sed -i 's/co.archive.ubuntu.com/archive.ubuntu.com/g' /etc/apt/sources.list
```

### 2. Habilitar los repositorios requeridos

```bash
sudo add-apt-repository universe -y
sudo add-apt-repository restricted -y
sudo add-apt-repository multiverse -y
```

### 3. Limpiar y actualizar los repositorios

```bash
sudo apt clean
sudo apt update
```

### 4. Instalar ROS Noetic

```bash
sudo apt --fix-broken install -y
sudo apt install -y ros-noetic-desktop-full
```

Si la instalación falla, intentar:

```bash
sudo apt update
sudo apt install -y --fix-missing ros-noetic-desktop-full
```

### 5. Finalizar la configuración de ROS

Agregar ROS Noetic al entorno:

```bash
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

Instalar las herramientas necesarias:

```bash
sudo apt install -y \
python3-rosdep \
python3-rosinstall \
python3-rosinstall-generator \
python3-wstool \
build-essential
```

Inicializar `rosdep`:

```bash
sudo rosdep init
rosdep update
```

### 6. DEPENDENCIAS BÁSICAS

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

### 7. Preparar OpenCV 4.4

Primero comprobar qué versión de OpenCV está disponible mediante `pkg-config`:

```bash
pkg-config --modversion opencv4
```

Existen dos escenarios posibles.

#### Si devuelve `4.2.0`

Ejecutar:

```bash
dpkg-query -W -f='${binary:Package}\t${Version}\n' \
'libopencv*' \
'opencv*' \
'python3-opencv' 2>/dev/null |
awk '$2 ~ /^4\.2\.0/ {print $1}' |
xargs -r sudo apt purge -y
```

Después:

```bash
sudo apt autoremove -y
sudo ldconfig
```

#### Si `pkg-config` indica que `opencv4` no existe

No desinstalar nada y continuar directamente con la instalación de OpenCV 4.4.0.

#### Descargar OpenCV 4.4.0

Ahora, independientemente del escenario anterior:

```bash
cd "$HOME/Escritorio"

wget -O opencv-4.4.0.zip \
https://github.com/opencv/opencv/archive/4.4.0.zip
```

Comprobar que el archivo se descargó correctamente:

```bash
ls -lh "$HOME/Escritorio/opencv-4.4.0.zip"
```

Extraer:

```bash
unzip -o "$HOME/Escritorio/opencv-4.4.0.zip"
```

Entrar al código fuente:

```bash
cd "$HOME/Escritorio/opencv-4.4.0"
```

Configurar una compilación limpia:

```bash
rm -rf build
mkdir build
cd build
```

Configurar CMake:

```bash
cmake .. \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=/usr/local
```

Compilar utilizando un solo núcleo para evitar saturar el sistema:

```bash
make -j1
```

Instalar:

```bash
sudo make install
sudo ldconfig
```

Comprobar la versión instalada:

```bash
grep -n "OpenCV_VERSION" \
/usr/local/lib/cmake/opencv4/OpenCVConfig-version.cmake
```

Debe devolver:

```text
4.4.0
```

### 8. Pangolin

Clonar específicamente la versión **v0.6**:

```bash
cd "$HOME"

rm -rf "$HOME/Pangolin"

git clone --branch v0.6 --depth 1 \
https://github.com/stevenlovegrove/Pangolin.git \
"$HOME/Pangolin"
```

Entrar al directorio:

```bash
cd "$HOME/Pangolin"
```

Preparar una compilación limpia:

```bash
rm -rf build
mkdir build
cd build
```

Configurar:

```bash
cmake .. \
-DCMAKE_BUILD_TYPE=Release \
-DBUILD_PANGOLIN_PYTHON=OFF
```

Compilar:

```bash
make -j1
```

Instalar:

```bash
sudo make install
sudo ldconfig
```

Con todos los cambios y dependencias anteriores realizados, ya es posible continuar con la instalación completa de **ORBSLAM3 fork**, siempre que el sistema no contara previamente con el entorno de **ROS 1 Noetic en Ubuntu 20.04** y sus dependencias.

# 1. UBICACIÓN DEL PAQUETE DE REINSTALACIÓN

Descargar la carpeta `OrbVisense-Navigator` en el Escritorio.

```bash
cd "$HOME/Escritorio"

git clone https://github.com/HeyItsLuan/OrbVisense-Navigator.git
```

La carpeta debe quedar ubicada en:

```text
$HOME/Escritorio/OrbVisense-Navigator
```

Contenido:

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

# 2. ENTORNO UTILIZADO

Sistema:

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

# 3. REGLA DE COMPILACIÓN

**IMPORTANTE:**

Utilizar siempre:

```bash
make -j1
```

y:

```bash
catkin_make -j1
```

El parámetro `-j1` indica que la compilación utilizará un solo proceso.

Aunque el equipo pueda soportar compilaciones paralelas, se utilizará **siempre `-j1` durante la instalación y compilación del proyecto** para evitar saturar los recursos del sistema y reducir el riesgo de que el equipo se congele.

NO utilizar:

```bash
make -j4
make -j$(nproc)
catkin_make -j4
```

Si un archivo `build.sh` utiliza:

```bash
make -j4
```

cambiarlo por:

```bash
make -j1
```

Para editarlo:

```bash
gedit "$HOME/Escritorio/ORB_SLAM3_fork/build.sh"
```

# 4. INSTALAR ORB-SLAM3 FORK

Este es el primer paso real de la instalación.

### 4.1. Clonar el fork de ORB-SLAM3

```bash
cd "$HOME/Escritorio"

git clone https://github.com/Lab-of-AI-and-Robotics/ORB_SLAM3.git ORB_SLAM3_fork
```

Esto crea:

```text
$HOME/Escritorio/ORB_SLAM3_fork
```

### 4.2. Copiar las modificaciones de OrbVIsense Navigator

El repositorio `OrbVisense-Navigator` contiene los archivos `.cc` modificados del fork de ORB-SLAM3.

Copiar los archivos:

```bash
cp "$HOME/Escritorio/OrbVisense-Navigator/ORB_SLAM3_fork_src_mod/src/"*.cc \
   "$HOME/Escritorio/ORB_SLAM3_fork/src/"
```

Los archivos modificados son:

```text
FrameDrawer.cc
ImuTypes.cc
MapDrawer.cc
Optimizer.cc
Tracking.cc
```

El repositorio de OrbVIsense Navigator contiene únicamente estas modificaciones dentro de:

```text
ORB_SLAM3_fork_src_mod/src/
```

### 4.3. Verificar la configuración de OpenCV

El `CMakeLists.txt` principal de ORB-SLAM3 debe utilizar **OpenCV 4.4**.

Comprobar qué versión solicita actualmente:

```bash
grep -n "find_package(OpenCV" \
"$HOME/Escritorio/ORB_SLAM3_fork/CMakeLists.txt"
```

Debe aparecer:

```cmake
find_package(OpenCV 4.4)
```

Si aparece otra versión, abrir el archivo:

```bash
gedit "$HOME/Escritorio/ORB_SLAM3_fork/CMakeLists.txt"
```

y cambiar únicamente la versión de OpenCV para dejar:

```cmake
find_package(OpenCV 4.4)
```

### 4.4. Limpiar compilaciones anteriores

Antes de recompilar, eliminar las librerías y directorios de compilación anteriores:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork"

rm -rf lib/*
rm -rf Thirdparty/DBoW2/lib/*
rm -rf Thirdparty/DBoW2/build
rm -rf Thirdparty/g2o/config.h
rm -rf Thirdparty/g2o/build
rm -rf build
```

### 4.5. Compilar DBoW2

Entrar en DBoW2:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/DBoW2"
```

Limpiar y crear el directorio de compilación:

```bash
rm -rf build
mkdir build
cd build
```

Configurar:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Compilar utilizando siempre un solo proceso:

```bash
make -j1
```

### 4.6. Compilar g2o

Entrar en g2o:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/g2o"
```

Limpiar y crear el directorio de compilación:

```bash
rm -rf build
mkdir build
cd build
```

Configurar:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Compilar utilizando siempre un solo proceso:

```bash
make -j1
```

### 4.7. Configurar ORB-SLAM3 fork para utilizar C++14

Abrir el `CMakeLists.txt` principal:

```bash
gedit "$HOME/Escritorio/ORB_SLAM3_fork/CMakeLists.txt"
```

La configuración del estándar C++ debe utilizar C++14:

```cmake
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    add_definitions(-DCOMPILEDWITHC14)
    message(STATUS "Using flag -std=c++14.")
```

Además, eliminar el bloque relacionado con RealSense que comienza en:

```cmake
# If RealSense SDK is found the library is added and its examples compiled
```

y continúa hasta el final del archivo.

Se utiliza el comentario como referencia para localizar el bloque, en lugar de depender de un número de línea concreto.

### 4.8. Compilar ORB-SLAM3 fork

Limpiar la compilación:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork"

rm -rf build
mkdir build
cd build
```

Configurar:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Compilar utilizando siempre un solo proceso:

```bash
make -j1
```

La compilación debe finalizar correctamente y generar las librerías de ORB-SLAM3.

# 5. PREPARAR WORKSPACE ROS1

Crear el workspace:

```bash
mkdir -p "$HOME/ros1_ws/src"
```

Cargar ROS:

```bash
source /opt/ros/noetic/setup.bash
```

Inicializar el workspace:

```bash
cd "$HOME/ros1_ws/src"
catkin_init_workspace
```

El workspace debe contener inicialmente solamente el `CMakeLists.txt` generado por `catkin`.

# 6. INSTALAR ORB_SLAM3_ROS_WRAPPER

## 6.1. Descargar el repositorio

```bash
cd "$HOME/ros1_ws/src"

git clone https://github.com/thien94/orb_slam3_ros_wrapper.git
```

## 6.2. Editar la ruta de ORB-SLAM3

Abrir:

```bash
gedit "$HOME/ros1_ws/src/orb_slam3_ros_wrapper/CMakeLists.txt"
```

Cambiar únicamente:

```cmake
set(ORB_SLAM3_DIR
   $ENV{HOME}/Packages/ORB_SLAM3
)
```

por:

```cmake
set(ORB_SLAM3_DIR
   $ENV{HOME}/Escritorio/ORB_SLAM3_fork
)
```

## 6.3. Instalar dependencias ROS del wrapper

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

## 6.4. Preparar el vocabulario

Descomprimir el vocabulario de ORB-SLAM3:

```bash
cd "$HOME/Escritorio/ORB_SLAM3_fork/Vocabulary"

tar -xf ORBvoc.txt.tar.gz
```

Copiarlo al wrapper:

```bash
cp ORBvoc.txt \
"$HOME/ros1_ws/src/orb_slam3_ros_wrapper/config/ORBvoc.txt"
```

## 6.5. Compilar el wrapper

Limpiar la compilación anterior:

```bash
cd "$HOME/ros1_ws"

rm -rf build devel
```

Cargar ROS:

```bash
source /opt/ros/noetic/setup.bash
```

Compilar siempre con un solo proceso:

```bash
catkin_make -j1 \
-DOpenCV_DIR=/usr/local/lib/cmake/opencv4
```

Cargar el workspace:

```bash
source "$HOME/ros1_ws/devel/setup.bash"
```

# 7. INSTALAR jpeg_to_mono

Copiar el paquete desde el repositorio de **OrbVIsense Navigator**:

```bash
cd "$HOME/ros1_ws/src"

cp -r "$HOME/Escritorio/OrbVisense-Navigator/jpeg_to_mono/jpeg_to_mono" .
```

Comprobar:

```bash
ls -lah "$HOME/ros1_ws/src/jpeg_to_mono"
```

Debe quedar ubicado en:

```text
$HOME/ros1_ws/src/jpeg_to_mono
```

Compilar:

```bash
cd "$HOME/ros1_ws"

source /opt/ros/noetic/setup.bash

catkin_make -j1 \
-DOpenCV_DIR=/usr/local/lib/cmake/opencv4
```

# 8. INSTALAR rosbridge_suite

Descargar `rosbridge_suite`:

```bash
cd "$HOME/ros1_ws/src"

git clone --branch ros1 --depth 1 \
https://github.com/RobotWebTools/rosbridge_suite.git
```

Instalar la dependencia necesaria:

```bash
sudo apt update

sudo apt install -y ros-noetic-rosauth
```

Compilar:

```bash
cd "$HOME/ros1_ws"

source /opt/ros/noetic/setup.bash

catkin_make -j1
```

# 9. CARGAR EL WORKSPACE

Cargar ROS:

```bash
source /opt/ros/noetic/setup.bash
```

Cargar el workspace:

```bash
source "$HOME/ros1_ws/devel/setup.bash"
```

# 10. COMPROBAR LOS PAQUETES ROS

Cargar los entornos:

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
```

Comprobar `orb_slam3_ros_wrapper`:

```bash
rospack find orb_slam3_ros_wrapper
```

Debe devolver:

```text
$HOME/ros1_ws/src/orb_slam3_ros_wrapper
```

Comprobar `jpeg_to_mono`:

```bash
rospack find jpeg_to_mono
```

Debe devolver:

```text
$HOME/ros1_ws/src/jpeg_to_mono
```

Comprobar `rosbridge_server`:

```bash
rospack find rosbridge_server
```

Debe devolver:

```text
$HOME/ros1_ws/src/rosbridge_suite/rosbridge_server
```

# 11. VERIFICACIÓN DE INSTALACIONES

Comprobar que existe la biblioteca de ORB-SLAM3:

```bash
ls -lh "$HOME/Escritorio/ORB_SLAM3_fork/lib/libORB_SLAM3.so"
```

Comprobar el ejecutable del wrapper:

```bash
ls -lh "$HOME/ros1_ws/devel/lib/orb_slam3_ros_wrapper/orb_slam3_ros_wrapper_mono_inertial"
```

Comprobar que el wrapper utiliza la biblioteca del fork:

```bash
ldd "$HOME/ros1_ws/devel/lib/orb_slam3_ros_wrapper/orb_slam3_ros_wrapper_mono_inertial" | grep ORB_SLAM3
```

Debe apuntar a:

```text
$HOME/Escritorio/ORB_SLAM3_fork/lib/libORB_SLAM3.so
```

Comprobar todos los paquetes:

```bash
rospack find orb_slam3_ros_wrapper
rospack find jpeg_to_mono
rospack find rosbridge_server
rospack find robot_pwm
```

Comprobar el nuevo mensaje ROS:

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"

rosmsg show robot_pwm/PWM
```

Debe mostrar:

```text
int16 left
int16 right
```

También se puede comprobar que el mensaje aparece en ROS:

```bash
rosmsg list | grep robot_pwm
```

Debe aparecer:

```text
robot_pwm/PWM
```

Comprobar los nodos:

```bash
rosnode list
```

Comprobar los topics:

```bash
rostopic list
```

El topic utilizado por el editor para enviar el movimiento del robot es:

```text
/robot/pwm
```

# 12. TOPICS PRINCIPALES

### Imagen comprimida

```text
/cam0/image_raw/compressed
```

### Imagen convertida

```text
/cam0/image_raw
```

### IMU

```text
/imu0
```

### Pose de cámara

```text
/orb_slam3/camera_pose
```

### Mapa

```text
/orb_slam3/map_points
```

El mapa utiliza el tipo:

```text
sensor_msgs/PointCloud2
```

El frame utilizado por ORB-SLAM3 es:

```text
world
```

# 13. COMPROBAR TOPICS

Lista completa:

```bash
rostopic list
```

Comprobar frecuencia de la imagen:

```bash
rostopic hz /cam0/image_raw
```

Comprobar frecuencia del IMU:

```bash
rostopic hz /imu0
```

Comprobar la pose:

```bash
rostopic echo /orb_slam3/camera_pose
```

Comprobar el mapa:

```bash
rostopic echo /orb_slam3/map_points
```

# 14. VENTANAS DE EJECUCIÓN

## Terminal 1 — ROSCORE

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
roscore
```

Esta terminal mantiene activo el núcleo de ROS.

## Terminal 2 — ROSBRIDGE

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
roslaunch rosbridge_server rosbridge_websocket.launch
```

Esto permite la comunicación mediante WebSocket.

## Terminal 3 — jpeg_to_mono

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
rosrun jpeg_to_mono jpeg_to_mono_node
```

Su función es convertir:

```text
/cam0/image_raw/compressed
```

en:

```text
/cam0/image_raw
```

## Terminal 4 — ORB-SLAM3 MONO-INERTIAL

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
roslaunch orb_slam3_ros_wrapper euroc_monoimu.launch
```

El sistema utiliza:

Imagen:

```text
/cam0/image_raw
```

IMU:

```text
/imu0
```

# 15. DISTRIBUCIÓN DE ARCHIVOS CLAVE

## Ubicación de archivos de configuración y mapas

### Archivo YAML de configuración

Los archivos `.yaml` utilizados por el wrapper se encuentran en:

```text
~/ros1_ws/src/orb_slam3_ros_wrapper/config/
```

Ahí se puede editar o reemplazar el YAML correspondiente a la configuración de la cámara y el IMU.

Para editar uno:

```bash
gedit "$HOME/ros1_ws/src/orb_slam3_ros_wrapper/config/NOMBRE_CONFIGURACION.yaml"
```

### Archivos de mapas `.osa`

Los mapas guardados por ORB-SLAM3 se almacenan normalmente en:

```text
~/.ros/
```

Para localizar los mapas:

```bash
find ~/.ros -maxdepth 1 -type f -name "*.osa"
```

La carpeta `~/.ros` pertenece al usuario, no al workspace.

Por eso, los mapas `.osa` **no se eliminan al borrar o reconstruir `~/ros1_ws`**.

En resumen:

```text
~/ros1_ws/src/orb_slam3_ros_wrapper/config/
└── Archivos YAML de configuración

~/.ros/
└── Archivos .osa de mapas/Atlas
```

**Regla práctica:** los `.yaml` se modifican dentro del `config` del wrapper; los `.osa` se buscan y administran dentro de `~/.ros`.

# 16. RESUMEN DEL SISTEMA ADAPTADO

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
# 17. INSTALACIÓN DEL ORBVISENSE NAVIGATOR

## 17.1. Instalación del mensaje ROS `robot_pwm`

Copiar el paquete desde el repositorio de OrbVIsense Navigator:

```bash
cd "$HOME/ros1_ws/src"

cp -r "$HOME/Escritorio/OrbVisense-Navigator/robot_pwm/robot_pwm" .
```

Compilar:

```bash
cd "$HOME/ros1_ws"

source /opt/ros/noetic/setup.bash

catkin_make -j1
```

Cargar el workspace:

```bash
source "$HOME/ros1_ws/devel/setup.bash"
```

Verificar el paquete:

```bash
rospack find robot_pwm
```

Debe devolver:

```text
$HOME/ros1_ws/src/robot_pwm
```

Verificar el contenido del mensaje:

```bash
rosmsg show robot_pwm/PWM
```

Debe mostrar:

```text
int16 left
int16 right
```

## 17.2. Instalación de OrbVIsense Navigator

Copiar el programa desde el repositorio:

```bash
cp -r "$HOME/Escritorio/OrbVisense-Navigator/orbvisense_navigator" \
      "$HOME/Escritorio/"
```

Compilar:

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

Antes de ejecutar el programa, cargar ROS y el workspace:

```bash
source /opt/ros/noetic/setup.bash
source "$HOME/ros1_ws/devel/setup.bash"
```

Definir la ubicación de ORB-SLAM3:

```bash
export ORB_SLAM3_ROOT="$HOME/Escritorio/ORB_SLAM3_fork"
```

Configurar las bibliotecas necesarias:

```bash
export LD_LIBRARY_PATH="$HOME/Escritorio/ORB_SLAM3_fork/lib:$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/DBoW2/lib:$HOME/Escritorio/ORB_SLAM3_fork/Thirdparty/g2o/lib:/usr/local/lib:/opt/ros/noetic/lib:${LD_LIBRARY_PATH:-}"
```

El comando para iniciar OrbVIsense Navigator es:

```bash
"$HOME/Escritorio/orbvisense_navigator/build/orbvisense_navigator"
```

## 17.3. Comandos de ejecución

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

# 18. EDICIONES PERMITIDAS

## 18.1. Modificación del tamaño del robot

Para modificar el **tamaño del robot**, editar:

```bash
gedit "$HOME/Escritorio/orbvisense_navigator/NavigationWidget.h"
```

Buscar:

```cpp
static constexpr float mRobotScale = 0.10f;
```

Modificar el valor según el tamaño deseado.

## 18.2. Modificación de la IP de transmisión del mensaje PWM

Para cambiar la IP del WebSocket donde se publica el mensaje PWM, editar:

```bash
gedit "$HOME/Escritorio/orbvisense_navigator/MainWindow.cpp"
```

Buscar:

```cpp
<< "ws://10.42.0.1:9090";
```

Modificar la dirección IP según la configuración de la red.

## 18.3. Recompilación

Después de cualquier modificación del código:

```bash
cd "$HOME/Escritorio/orbvisense_navigator"

cmake --build "$HOME/Escritorio/orbvisense_navigator/build" -j1
```

Después de recompilar, volver a ejecutar:

```bash
"$HOME/Escritorio/orbvisense_navigator/build/orbvisense_navigator"
```

# 19. FUNCIONAMIENTO GENERAL DE ORBVISENSE NAVIGATOR

## 19.1. Requerimientos

Para utilizar completamente **OrbVIsense Navigator** se requieren los siguientes elementos.

### 1. Teléfono Android

Se requiere un teléfono Android con la aplicación **OrbVIsense** instalada.

La aplicación se encuentra en el repositorio:

[HeyItsLuan/OrbVIsense — GitHub](https://github.com/HeyItsLuan/OrbVIsense?utm_source=chatgpt.com)

### 2. Calibración del teléfono

El teléfono debe estar previamente calibrado.

La propia aplicación permite generar los datasets necesarios para realizar la calibración.

El procedimiento de calibración se encuentra en:

[HeyItsLuan/OrbVIsense-calibration — GitHub](https://github.com/HeyItsLuan/OrbVIsense-calibration?utm_source=chatgpt.com)

La calibración debe realizarse antes de utilizar el teléfono para obtener resultados confiables con ORB-SLAM3.

### 3. Conexión de red

El teléfono Android y el computador deben estar conectados a la **misma red**.

Se debe conocer la dirección IP del computador, ya que el computador funciona como **servidor** dentro de la comunicación entre el teléfono y ROS.

### 4. Robot y comunicación WebSocket

El robot debe tener configurada en su código la dirección IP del computador.

El código del robot se encuentra en:

[HeyItsLuan/OrbVIsense-robot — GitHub](https://github.com/HeyItsLuan/OrbVIsense-robot?utm_source=chatgpt.com)

La IP configurada en el código del robot debe corresponder con la IP del computador que funciona como servidor.

Además, la ESP32 debe encontrarse en modo de comunicación mediante WebSocket.

Para activar este modo se debe presionar el botón **BOOT** de la ESP32.

## 19.2. Uso de ORB-SLAM3 de forma independiente

Una vez completados los pasos anteriores, es posible utilizar el **ORB-SLAM3 fork** con el teléfono Android siguiendo la arquitectura:

```text
Teléfono Android
       │
       │ WebSocket
       ▼
   Computador
       │
       ▼
      ROS1
       │
       ▼
   ORB-SLAM3
```

El sistema puede utilizarse de manera independiente hasta el **paso 14** de esta guía.

En este punto es posible ejecutar ORB-SLAM3, recibir la cámara y el IMU del teléfono y generar mapas sin utilizar OrbVIsense Navigator.

## 19.3. Generación de mapas para navegación

Si además de utilizar ORB-SLAM3 se desea realizar **navegación autónoma**, se debe continuar con la instalación hasta OrbVIsense Navigator.

Para navegar es necesario disponer previamente de un mapa.

Los mapas pueden generarse utilizando ORB-SLAM3 desde el paso 14 de esta guía o utilizando el sistema junto con OrbVIsense Navigator.

Para generar diferentes mapas o modificar la configuración utilizada por ORB-SLAM3, se puede modificar el archivo:

```text
$HOME/ros1_ws/src/orb_slam3_ros_wrapper/config/euroc.yaml
```

Los mapas generados por ORB-SLAM3 deben guardarse en formato:

```text
.osa
```

Estos archivos corresponden a los Atlas guardados por ORB-SLAM3 y son los que posteriormente puede cargar OrbVIsense Navigator.

# 19.4. Flujo de navegación en OrbVIsense Navigator

Una vez generado un mapa `.osa`, se puede utilizar OrbVIsense Navigator para realizar la navegación.

### Paso 1. Cargar el mapa

En OrbVIsense Navigator se debe utilizar el botón **Load**.

Se selecciona el mapa `.osa` que se desea utilizar.

Después de seleccionar el archivo se debe esperar unos segundos mientras el mapa es cargado.

### Paso 2. Activar la navegación

Una vez cargado el mapa estará disponible la opción **Navigation**.

Se debe presionar el botón para acceder a las herramientas de navegación.

### Paso 3. Configurar la IP del robot

Antes de comenzar la navegación se debe comprobar la configuración descrita en el apartado **18.2**.

La dirección IP utilizada para publicar el mensaje PWM debe corresponder con la dirección del computador que funciona como servidor WebSocket.

## 19.5. Calibración de la representación del robot sobre el mapa

Al cargar un mapa, OrbVIsense Navigator representa inicialmente el robot en la coordenada:

```text
X = 0.0
Y = 0.0
```

La representación inicial del tamaño del robot puede no corresponder exactamente con sus dimensiones reales.

Por esta razón se plantea una calibración utilizando mediciones realizadas físicamente.

### Datos necesarios

Para realizar esta calibración se necesitan:

* Una medida métrica real.
* La distancia entre dos posiciones conocidas.
* La longitud real del robot.
* Las coordenadas de dos poses válidas obtenidas mediante ORB-SLAM3.

### Obtener las dos poses

Primero se debe activar ORB-SLAM3 mediante el botón:

**Enable ORB-SLAM3**

Una vez iniciado ORB-SLAM3, se debe seleccionar el modo:

**Localization**

Después se mueve físicamente el robot sobre el mapa.

ORB-SLAM3 proporcionará la pose estimada del robot.

Para visualizar esta pose dentro de OrbVIsense Navigator se debe activar:

**Enable Reception**

Esto permite recibir el topic de pose publicado por ORB-SLAM3 y dibujar la posición del robot sobre el mapa.

Se deben seleccionar dos posiciones físicamente diferentes del robot que produzcan una **pose válida**.

Una pose se considera válida cuando sus coordenadas no corresponden a:

```text
X = 0.0
Y = 0.0
```

Las dos posiciones deben registrarse y medirse físicamente.

### Cálculo de la escala del robot

Se tendrán entonces:

* Distancia métrica real entre las dos posiciones.
* Distancia entre las dos posiciones en coordenadas del mapa.
* Longitud real del robot.

La distancia entre las dos poses del mapa se calcula a partir de sus coordenadas.

Después se obtiene el valor representativo del robot en puntos mediante:

```text
Representación del robot =
(Longitud real del robot × distancia entre las dos poses en coordenadas)
/
Distancia métrica real entre las dos poses
```

De esta manera se obtiene el tamaño que debe utilizar el programa para representar el robot correctamente dentro del mapa.

El valor calculado puede utilizarse para modificar el parámetro correspondiente descrito en el apartado **18.1**.

## 19.6. Herramientas de edición del mapa

Después de cargar un mapa, OrbVIsense Navigator proporciona herramientas para preparar el mapa antes de calcular una ruta.

### Select Contour

El botón **Select Contour** permite seleccionar una región del mapa.

Después de activar esta herramienta se puede hacer clic sobre el mapa para seleccionar un contorno interno.

El contorno seleccionado permite definir una región que será utilizada posteriormente durante el cálculo de la ruta.

También se muestran las coordenadas correspondientes al punto seleccionado.

### Delete Selected

El botón **Delete Selected** permite eliminar puntos del mapa.

Esta herramienta es útil cuando existen:

* Puntos falsos.
* Puntos aislados.
* Puntos que generan contactos inexistentes.
* Puntos que impiden definir correctamente un contorno interno.

Los puntos pueden seleccionarse mediante clic izquierdo.

También es posible mantener presionado el clic izquierdo para seleccionar una región de puntos.

### Eliminación de puntos extremos en Z

El mapa utilizado para la navegación corresponde principalmente al plano:

```text
X-Y
```

Por lo tanto, la coordenada `Z`, que representa la altura, no es necesaria para la navegación bidimensional.

Sin embargo, pueden existir puntos con valores de `Z` extremos que aparezcan como obstáculos o contactos falsos al proyectarse sobre el mapa.

OrbVIsense Navigator permite eliminar estos puntos mediante los controles correspondientes.

Al utilizar las flechas se eliminan progresivamente los valores situados a la derecha o a la izquierda de los límites establecidos.

Esto permite reducir los puntos extremos de la coordenada `Z` y conservar únicamente la información relevante para la navegación en el plano `X-Y`.

## 19.7. Activación completa del sistema

El botón:

**Enable ORB-SLAM3**

permite iniciar simultáneamente los componentes necesarios para utilizar el sistema:

* WebSocket.
* `jpeg_to_mono`.
* ORB-SLAM3 fork.

Una vez activado, se habilita en el panel izquierdo la sección de herramientas de navegación.

### Enable Reception

El botón **Enable Reception** permite recibir desde ORB-SLAM3 el topic correspondiente a la pose del robot.

El panel muestra información sobre:

* Estado de la recepción.
* Posición del robot.
* Orientación del robot.

La pose recibida se representa directamente sobre el mapa cargado.

## 19.8. Selección del punto B

El botón para marcar el **punto B** permite seleccionar sobre el mapa el lugar al que se desea desplazar el robot.

Después de activar esta herramienta, se debe hacer clic sobre la posición de destino dentro del mapa.

Una vez seleccionado el punto B, OrbVIsense Navigator muestra sus coordenadas en pantalla.

## 19.9. Cálculo de la ruta

Con el robot localizado y el punto B seleccionado se utiliza:

**Calculate Route**

El programa calcula una ruta utilizando:

* Los límites del mapa.
* La posición actual del robot.
* El tamaño del robot.
* El punto B seleccionado.
* Las regiones disponibles para desplazamiento.

Si existe una ruta válida, esta se dibuja sobre el mapa.

Si no existe una ruta posible, el programa informa que el destino no puede alcanzarse desde la posición actual.

## 19.10. Inicio de la navegación

Cuando existe una ruta válida se debe presionar:

**Start Route**

El robot comenzará a desplazarse desde su posición actual hacia el punto B siguiendo la ruta calculada.

Durante el desplazamiento, el sistema utiliza la pose proporcionada por ORB-SLAM3 para determinar la posición y orientación actual del robot.

## 19.11. Tolerancias y recálculo de ruta

El robot utiliza tolerancias para determinar:

* Cuándo ha llegado al punto de destino.
* Cuándo se ha desviado de la ruta calculada.
* Cuándo debe corregir su trayectoria.

Si el robot se desvía de la ruta, el sistema espera aproximadamente **2 segundos** y calcula una nueva ruta desde la posición actual.

Si el robot pierde la pose válida, el sistema intenta recuperar una pose válida mediante el movimiento del robot.

Una vez recuperada una pose válida, el sistema puede volver a calcular la ruta desde la nueva posición.

De esta manera, la navegación puede adaptarse a desviaciones del robot y a cambios en la estimación de la pose.
