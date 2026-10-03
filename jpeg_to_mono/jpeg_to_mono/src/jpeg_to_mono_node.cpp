#include <ros/ros.h>
#include <sensor_msgs/Image.h>

#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

class JPEGToMono
{
public:
    JPEGToMono()
    {
        sub_ = nh_.subscribe("/cam0/image_raw/compressed", 1, &JPEGToMono::callback, this);
        pub_ = nh_.advertise<sensor_msgs::Image>("/cam0/image_raw", 1);

        ROS_INFO("jpeg_to_mono iniciado.");
        ROS_INFO("Escuchando: /cam0/image_raw/compressed");
        ROS_INFO("Publicando: /cam0/image_raw");
    }

private:

    ros::NodeHandle nh_;
    ros::Subscriber sub_;
    ros::Publisher pub_;

    void callback(const sensor_msgs::ImageConstPtr& msg)
    {
        if(msg->encoding != "jpeg")
        {
            ROS_WARN_THROTTLE(2.0,
                "Encoding recibido: %s (esperaba jpeg)",
                msg->encoding.c_str());
            return;
        }

        try
        {
            cv::Mat jpegData(1,
                             msg->data.size(),
                             CV_8UC1,
                             const_cast<unsigned char*>(msg->data.data()));

            cv::Mat gray = cv::imdecode(jpegData, cv::IMREAD_GRAYSCALE);

            if(gray.empty())
            {
                ROS_WARN("No se pudo descomprimir JPEG");
                return;
            }

            cv_bridge::CvImage out;

            out.header = msg->header;
            out.encoding = "mono8";
            out.image = gray;

            pub_.publish(out.toImageMsg());

            ROS_INFO_THROTTLE(
                2.0,
                "JPEG %lu bytes -> mono8 %dx%d",
                msg->data.size(),
                gray.cols,
                gray.rows);
        }
        catch(const std::exception& e)
        {
            ROS_ERROR("%s", e.what());
        }
    }
};

int main(int argc,char** argv)
{
    ros::init(argc,argv,"jpeg_to_mono");

    JPEGToMono node;

    ros::spin();

    return 0;
}
