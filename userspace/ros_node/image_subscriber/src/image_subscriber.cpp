#include <cv_bridge/cv_bridge.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include "dynamixel_sdk/dynamixel_sdk.h"
#include "dynamixel_sdk_custom_interfaces/msg/set_position.hpp"
#include "xnn_inference.h"

#define ROTATION_MOTOR_ID 1
#define ANGLE_MOTOR_ID 0

#define RESIZED_IMG_WIDTH 20
#define RESIZED_IMG_HEIGHT 15

#define ROTATION_MOTOR_INIT_POS 400
#define ROTATION_MOTOR_MAX_POS 0
#define ROTATION_MOTOR_STEP (-1)

#define ANGLE_MOTOR_INIT_POS 512

#define DEGREES_TO_MOTOR_ANGLE 3.4

#define NN_CORRECT_LABEL 1 // Hexagonal bolt

// Canny filter to get the rotation angle of a screw / bolt
double find_rotation_angle(const cv::Mat& input_img);

class ImageSubscriber : public rclcpp::Node
{
    public:
        ImageSubscriber() : Node("image_subscriber") {
            RCLCPP_INFO(this->get_logger(), "Initializing ImageSubscriber node");

            int status = XNn_inference_Initialize(&ip_inst, "nn_inference");
            if (status != XST_SUCCESS) {
                RCLCPP_INFO(this->get_logger(), "Error: Could not initialize the IP core.");
                return;
            }

            current_rotation_motor_angle = ROTATION_MOTOR_INIT_POS;
            current_angle_motor_angle = ANGLE_MOTOR_INIT_POS;

            motor_publisher_ = this->create_publisher<dynamixel_sdk_custom_interfaces::msg::SetPosition>("set_position", 10);

            set_motor_position(ROTATION_MOTOR_ID, ROTATION_MOTOR_INIT_POS);
            set_motor_position(ANGLE_MOTOR_ID, ANGLE_MOTOR_INIT_POS);

            camera_subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
                "/image_raw",
                10,
                std::bind(&ImageSubscriber::onImageMsg, this, std::placeholders::_1)
            );

            end = false;
        }

    private:
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr camera_subscription_;
        rclcpp::Publisher<dynamixel_sdk_custom_interfaces::msg::SetPosition>::SharedPtr motor_publisher_;
        XNn_inference ip_inst;
        int current_rotation_motor_angle;
        int current_angle_motor_angle;
        uint32_t nn_output;
        bool end;

        // Main loop
        void onImageMsg(const sensor_msgs::msg::Image::SharedPtr msg) 
        {
            if (end) { return; }
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, msg->encoding);
            cv::Mat img = cv_ptr->image;

            nn_output = get_nn_output(img);
            std::cout << "NN output at rotation angle " << current_rotation_motor_angle << ": " << nn_output << std::endl;
            if (nn_output == NN_CORRECT_LABEL) {
                std::cout << "Correct label found, now calculating the angle of the bolt" << std::endl;
                double bolt_rotation_angle = find_rotation_angle(img);
                std::cout << "Rotation angle found (in degrees): " << bolt_rotation_angle << std::endl;
                int32_t motor_angle = (int32_t)(bolt_rotation_angle * DEGREES_TO_MOTOR_ANGLE);
                std::cout << "Moving the angle motor to position " << motor_angle << "..." << std::endl;
                set_motor_position(ANGLE_MOTOR_ID, motor_angle);
                end = true;
                std::cout << "END" << std::endl;
                return;
            }
            current_rotation_motor_angle += ROTATION_MOTOR_STEP;
            if (current_rotation_motor_angle < ROTATION_MOTOR_MAX_POS) {
                current_rotation_motor_angle = ROTATION_MOTOR_MAX_POS;
            }
            std::cout << "The label does not match with the goal, moving the rotation motor to position " << current_rotation_motor_angle << "..." << std::endl;
            set_motor_position(ROTATION_MOTOR_ID, current_rotation_motor_angle + ROTATION_MOTOR_STEP);
        }

        void set_motor_position(uint8_t motor_id, int32_t angle)
        {
            dynamixel_sdk_custom_interfaces::msg::SetPosition new_pos;
            new_pos.id = motor_id;
            new_pos.position = angle;

            motor_publisher_->publish(new_pos);
        }

        // One neural network inference and return the output
        uint32_t get_nn_output(cv::Mat& camera_img)
        {
            cv::Mat img_rgb;
            cv::cvtColor(camera_img, img_rgb, cv::COLOR_YUV2RGB_YUY2);
            cv::Mat resized_img;
            cv::resize(camera_img, resized_img, cv::Size(RESIZED_IMG_WIDTH, RESIZED_IMG_HEIGHT));

            // Flatten and normalize
            std::vector<float> nn_input_img;
            for (int row = 0; row < resized_img.rows; row++) {
                for (int col = 0; col < resized_img.cols; col++) {
                    for (int color = 0; color < 3; color++) {
                        nn_input_img.push_back((float)img_rgb.at<cv::Vec3b>(row,col)[color] / (float)255);
                    }
                }
            }

            XNn_inference_Write_input_img_Words(&ip_inst, 0, (unsigned int *)nn_input_img.data(), RESIZED_IMG_WIDTH * RESIZED_IMG_HEIGHT * 3);
            XNn_inference_Start(&ip_inst);

            // Wait for the IP core to finish
            while (!XNn_inference_IsDone(&ip_inst));

            return nn_output = XNn_inference_Get_return(&ip_inst);
        }

        // Canny filter impelmentation to get the angle of a bole / screw
        double find_rotation_angle(const cv::Mat& camera_img)
        {
            cv::Mat img_grayscale;
            cv::cvtColor(camera_img, img_grayscale, cv::COLOR_YUV2GRAY_YUY2);

            cv::Mat thresh;
            cv::threshold(img_grayscale, thresh, 94, 500, cv::THRESH_BINARY_INV);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            if (contours.empty()) { return 0.0; }

            std::sort(contours.begin(), contours.end(), [](const auto& a, const auto& b) {
                return cv::contourArea(a) > cv::contourArea(b);
            });

            int num_lines = 0;
            uint32_t num = 0;
            auto& selected_contour = contours[num];

            while (num_lines < 2 || num_lines > 6) {
                if (num < contours.size()) {
                    selected_contour = contours[num];
                }
                else {
                    return 0.0;
                }
                cv::Mat bolt_contour_img = cv::Mat::zeros(img_grayscale.size(), img_grayscale.type());
                cv::drawContours(
                    bolt_contour_img,
                    std::vector<std::vector<cv::Point>>{selected_contour},
                    -1,
                    255,
                    cv::FILLED
                );

                cv::Mat edges;
                cv::Canny(bolt_contour_img, edges, 50, 100);

                std::vector<cv::Vec2f> lines;
                cv::HoughLines(edges, lines, 1, CV_PI / 60, 30);

                if (!lines.empty()) {
                    num_lines = static_cast<int>(lines.size());
                    double theta = 0;
                    if (num_lines >= 2 && num_lines <= 6) {
                        for (const auto& line : lines) {
                            theta = line[0];
                            if (
                                (cv::fastAtan2(static_cast<float>(std::sin(theta)), static_cast<float>(std::cos(theta))) >= 0) &&
                                (cv::fastAtan2(static_cast<float>(std::sin(theta)), static_cast<float>(std::cos(theta))) < 180)
                            ) {                               
                                if (cv::fastAtan2(static_cast<float>(std::sin(theta)), static_cast<float>(std::cos(theta))) <= 60) {
                                    return cv::fastAtan2(static_cast<float>(std::sin(theta)), static_cast<float>(std::cos(theta)));
                                }
                            }
                        }
                        if (cv::fastAtan2(static_cast<float>(std::sin(theta)), static_cast<float>(std::cos(theta))) > 60) {
                            float current_angle = cv::fastAtan2(static_cast<float>(std::sin(theta)), static_cast<float>(std::cos(theta)));
                            return ((uint32_t)(std::abs(static_cast<float>(current_angle)))) % 60;
                        }
                    } 
                    num++;
                }
            } 
            return 0.0;
        }
};



int main(int argc, char *argv[])
{
    setvbuf(stdout,NULL,_IONBF,BUFSIZ);

    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<ImageSubscriber>());

    rclcpp::shutdown();
    return 0;
}