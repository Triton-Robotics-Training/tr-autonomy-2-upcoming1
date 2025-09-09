#include "AimerGuy.h"

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AimerGuy>());
  rclcpp::shutdown();
  return 0;
}

AimerGuy::AimerGuy() : Node("aimer"){
    rmw_qos_profile_t custom_qos = rmw_qos_profile_default;

    current_angle = 0.0;
    desired_angle = 0.0;
    points  = 0;
    angle_acquired = false;
    image_acquired = false;






    angle_publisher = this->create_publisher<std_msgs::msg::Float32>("/desired_angle", 10);
    
    image_subscriber = image_transport::create_subscription(this, "/robotcam", std::bind(&AimerGuy::getImage, this, std::placeholders::_1), "raw", custom_qos);

    angle_subscriber = this->create_subscription<std_msgs::msg::Float32>("/current_angle", 10, std::bind(&AimerGuy::getCurrentAngle, this, std::placeholders::_1));


    timer = this->create_wall_timer(100ms, std::bind(&AimerGuy::publishDesiredAngle, this));


}



void AimerGuy::getImage(const sensor_msgs::msg::Image::ConstSharedPtr &msg){
    cv_bridge::CvImagePtr cv_ptr;

    try
    {
        cv_ptr = cv_bridge::toCvCopy(msg, msg->encoding);
        image = cv_ptr->image.clone();
        image_acquired = true;
    }

    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge excetption: %s", e.what());
        return;
    }
    
}

void AimerGuy::getCurrentAngle(Angle::SharedPtr angle){
    current_angle = angle->data;

}

void AimerGuy::publishDesiredAngle(){
    if(!image_acquired){
        cout<< "Not nothing";
        return;


    }

    cv::Mat converted_image;

    cv::cvtColor(image, converted_image, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::Scalar lower_bound = cv::Scalar(0, 0, 0);
    cv::Scalar upper_bound = cv::Scalar(179, 109, 255);

    cv::inRange(converted_image, lower_bound, upper_bound, mask);


    cv::Mat inverted_mask;
    cv::bitwise_not(mask, inverted_mask);


    std::vector<std::vector<cv::Point>> contours;
    cv::Mat hierarchy;

    cv::findContours(inverted_mask, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    if(!contours.empty()){
        std::vector<cv::Point> target_contour = contours[0];

        cv::Moments moment = cv::moments(target_contour);

        center_x = moment.m10 / moment.m00;
        center_y = moment.m01 / moment.m00;
        
        float half = image.cols/2.0f;

        float pixel_error = center_x - half;

        float ratio_of_target = pixel_error / half;
        float predicted_angle = -1.0f * ratio_of_target * (M_PI/2.0f);

        desired_angle = current_angle + predicted_angle;

        if(desired_angle > M_PI/2.0) desired_angle =  M_PI/2.0;
        if(desired_angle < -M_PI/2.0) desired_angle = M_PI/2.0;

        bool target_centered = (abs(pixel_error) <= 60);

        if(target_centered){
            if(!is_centered){
                is_centered = true;
                center_time = this->now();
            }else{

                auto centered_duration = this->now() - center_time;
                if(centered_duration.seconds() >= 2.0){
                    points++;
                    RCLCPP_INFO(this->get_logger(), "POINT SCORED");
                    is_centered = false;
                    desired_angle = 0;
                }

            }

        }else{
            is_centered = false;
        }

        cv::circle(image, cv::Point(center_x, center_y), 10, cv::Scalar(0, 255, 0), 3);

        int image_center = image.cols/2;
        cv::rectangle(image, cv::Point(image_center - 60, 0), cv::Point(image_center + 60, image.rows), cv::Scalar(255, 255, 0), 2);

    }else{
        is_centered = false;
        desired_angle = 0.0f;
    }

    int center = image.cols / 2;

    cv::line(image, cv::Point(center, 0), cv::Point(center, image.rows), cv::Scalar(255, 255, 255), 2);

    cv::imshow(OPENCV_WINDOW, image);
    cv::waitKey(1);

    auto angle_message = std_msgs::msg::Float32();
    angle_message.data = desired_angle;

    angle_publisher->publish(angle_message);

}

