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

