#!/usr/bin/env python3

import os
import sys
import csv

import cv2
import rosbag
import rospy

from cv_bridge import CvBridge
from sensor_msgs.msg import Imu


# ============================================================
# COMPROBAR ARGUMENTO
# ============================================================

if len(sys.argv) != 2:
    print("Uso:")
    print("  python3 dataset_to_rosbag.py <ruta_dataset>")
    sys.exit(1)

dataset = os.path.abspath(os.path.expanduser(sys.argv[1]))

if not os.path.isdir(dataset):
    print("ERROR: el dataset no existe:")
    print(dataset)
    sys.exit(1)


# ============================================================
# DETERMINAR TIPO DE DATASET
# ============================================================

dataset_name = os.path.basename(os.path.normpath(dataset))

if dataset_name == "dataset_CAMIMU":
    bag_name = "camera_imu.bag"
    include_camera = True

elif dataset_name == "dataset_IMU":
    bag_name = "imu.bag"
    include_camera = False

else:
    print("ERROR: el dataset debe llamarse:")
    print("  dataset_CAMIMU")
    print("o")
    print("  dataset_IMU")
    sys.exit(1)


bagname = os.path.join(dataset, bag_name)


# ============================================================
# RUTAS
# ============================================================

camera_csv = os.path.join(
    dataset,
    "cam0",
    "data.csv"
)

camera_data = os.path.join(
    dataset,
    "cam0",
    "data"
)

imu_csv = os.path.join(
    dataset,
    "imu0",
    "data.csv"
)


# ============================================================
# COMPROBAR IMU
# ============================================================

if not os.path.isfile(imu_csv):

    print("ERROR: no existe:")
    print(imu_csv)

    sys.exit(1)


# ============================================================
# CREAR BAG
# ============================================================

print("Dataset:", dataset)
print("Creando:", bagname)

bag = rosbag.Bag(
    bagname,
    "w"
)


# ============================================================
# CÁMARA
# ============================================================

if include_camera:

    if not os.path.isfile(camera_csv):

        print("ERROR: no existe:")
        print(camera_csv)

        bag.close()
        sys.exit(1)

    bridge = CvBridge()

    with open(camera_csv) as f:

        reader = csv.reader(f)

        next(reader)

        for row in reader:

            timestamp = int(row[0])
            filename = row[1]

            img_path = os.path.join(
                camera_data,
                filename
            )

            img = cv2.imread(
                img_path,
                cv2.IMREAD_GRAYSCALE
            )

            if img is None:

                print(
                    "No pude leer:",
                    img_path
                )

                continue

            msg = bridge.cv2_to_imgmsg(
                img,
                encoding="mono8"
            )

            msg.header.stamp = rospy.Time.from_sec(
                timestamp / 1e9
            )

            msg.header.frame_id = "camera"

            bag.write(
                "/cam0/image_raw",
                msg,
                msg.header.stamp
            )

    print("Imágenes exportadas.")


# ============================================================
# IMU
# ============================================================

with open(imu_csv) as f:

    reader = csv.reader(f)

    next(reader)

    for row in reader:

        timestamp = int(row[0])

        msg = Imu()

        msg.header.stamp = rospy.Time.from_sec(
            timestamp / 1e9
        )

        msg.header.frame_id = "imu"

        msg.angular_velocity.x = float(row[1])
        msg.angular_velocity.y = float(row[2])
        msg.angular_velocity.z = float(row[3])

        msg.linear_acceleration.x = float(row[4])
        msg.linear_acceleration.y = float(row[5])
        msg.linear_acceleration.z = float(row[6])

        bag.write(
            "/imu0",
            msg,
            msg.header.stamp
        )

print("IMU exportada.")


# ============================================================
# FINALIZAR
# ============================================================

bag.close()

print()
print("Bag terminado correctamente.")
print("Archivo:", bagname)
