#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include "image_transport/image_transport.hpp"
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <functional>
#include <stdlib.h>

using namespace std;


using Angle = std_msgs::msg::Float32;


class AimerGuy : public rclcpp::Node{
    public:
        AimerGuy();
        const std::string OPENCV_WINDOW = "Image window";

        rmw_qos_profile_t custom_qos = rmw_qos_profile_default;

    


    private:

    rclcpp::Time last_time;
    rclcpp::Time center_time;

    rclcpp::TimerBase::SharedPtr timer;
        rclcpp::TimerBase::SharedPtr timer2;


    double current_angle;
    double desired_angle;
    int points;

    bool angle_acquired;



    double center_x;
    double center_y;

    bool is_centered;

    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr angle_subscriber;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr angle_publisher;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr point_subscriber;
    image_transport::Subscriber image_subscriber;
    image_transport::Publisher image_publisher;

    cv::Mat image;

    bool image_acquired;


    void getCurrentAngle(Angle::SharedPtr angle);

    void getImage(const sensor_msgs::msg::Image::ConstSharedPtr &msg);

    void publishDesiredAngle();
                 



};