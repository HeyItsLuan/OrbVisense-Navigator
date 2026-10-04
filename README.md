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
