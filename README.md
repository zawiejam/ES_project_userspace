# Userspace code for the Embedded Systems project (group 1)

This is the repository containing all the developed code for the Embedded Systems project:

* `./AI_training` contains the python file that was used to thain the nueral network. The exported weights are also there. However, the training images are not because there are too many (over 6000). To test, you need to add `./AI_training/data/x` folders, with `x` being the labels of the images located in the specific folder.

* `./HLS_IPs` contains the developed HLS IPs with Vitis HLS. There are two: one without DMA (this is the one used in the final version), and one with the DMA. However this last one ended up being only an attempt because of a lack of time. To test, you just have to create a new Vitis HLS project and add the three files as code / test bench (the test bench is the file ending in `/_tb.cpp`).

* `./bare_metal_test` contains the bare metal tests that have been performed on Vitis. There is one with the DMA alone (with no IP in the loop) that works fine. The other one (which is the one using the final version of the design) runs by directly writing in the neural network IP. To test, you have to create a Vivado project that implements the correct design (either with DMA alone or with the neural network directly connected with the CPU). Then generate the bitstream, create a Vitis project from it, use the helloworld template, and replace the `helloworld.c` file with one of the two in this folder, depending on the design you implemented.

* `./userspace` contains two things:
  * `./userspace/dma_test` contains a c++ project that was used to test the design with the DMA alone (with no IP in the loop). This works well. To test, you have to build the petalinux project and compile / run the cpp.
  * `./userspace/ros_node` contains the final ROS node used for this project. It works with the design that writes directly to the neural network IP. The node itself lies in the `./usersrpace/ros_node/image_subscriber` folder. The other folders in the `./userspace/ros_node` directory are the one being used by the Dynamixel motors. Particularly, the `./userspace/ros_node/dynamixel_sdk_custom_interfaces` contains the custom message types that have to be used with the motors.