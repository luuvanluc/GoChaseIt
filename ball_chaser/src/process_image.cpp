#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot

    // Request velocities
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the ball_chaser/command_robot service and pass the requested velocities
    if (!client.call(srv))
        ROS_ERROR("Failed to call service ball_chaser/command_robot");
}

enum IMAGE_REGION 
{
  IMAGE_REGION_NONE,
  IMAGE_REGION_LEFT,
  IMAGE_REGION_MID,
  IMAGE_REGION_RIGHT
};

// This fuction to check position of ball in image
IMAGE_REGION check_ball_position (const sensor_msgs::Image img)
{
    int white_pixel = 255;
    int stepLeft = img.step / 3;
    int stepRight = 2 * img.step / 3;
    int i = 0, j = 0;
    int k = 0; 
    // Check LEFT
    for (i = 0; i < stepLeft; i++)
    {
        for (j = 0; j < img.height; j++)
        {
            k = j * img.step + i;
            if ((img.data[k] & img.data[k + 1] & img.data[k + 2]) ==  white_pixel) {
                return IMAGE_REGION_LEFT;
            }
        }
    }

    // Check RIGHT
    for (i = stepRight; i < img.step - 2; i++)
    {
        for (j = 0; j < img.height; j++)
        {
            k = j * img.step + i;
            if ((img.data[k] & img.data[k + 1] & img.data[k + 2]) ==  white_pixel) {
                return IMAGE_REGION_RIGHT;
            }
        }
    }

    // Check MID
    for (i = stepLeft; i < stepRight; i++)
    {
        for (j = 0; j < img.height; j++)
        {
            k = j * img.step + i;
            if ((img.data[k] & img.data[k + 1] & img.data[k + 2]) ==  white_pixel) {
                return IMAGE_REGION_MID;
            }
        }
    }

    // NONE
    return IMAGE_REGION_NONE;
}


// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    IMAGE_REGION position_ball = check_ball_position(img);
    switch(position_ball)
    {
    case IMAGE_REGION_LEFT:
        drive_robot(0.0f,0.1f);
        break;

    case IMAGE_REGION_RIGHT:
        drive_robot(0.0f,-0.1f);
        break;

    case IMAGE_REGION_MID:
        drive_robot(0.1f,0.0f);
        break;

    case IMAGE_REGION_NONE:
    default:
        drive_robot(0.0f,0.0f);
        break;
    }

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
