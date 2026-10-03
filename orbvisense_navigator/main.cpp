#include <QApplication>

#include <ros/ros.h>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    ros::init(
        argc,
        argv,
        "orbvisense_navigator",
        ros::init_options::AnonymousName
    );

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
