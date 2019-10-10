//
// Created by nima on 19/07/17.
//
#include <iosfwd>
#include <ros/ros.h>
#include <ros/console.h>
#include <geometry_msgs/PoseStamped.h>
#include <sensor_msgs/Joy.h>
#include <kdl/frames.hpp>
#include <kdl_conversions/kdl_msg.h>
#include <std_msgs/String.h>
#include <std_msgs/Int8.h>
#include "../ar_core/ControlEvents.h"
#include <std_msgs/Float32.h>

// This node simulates the slaves of the dvrk in a teleop mode and controls the
// behavior of the master console to mock that of the dvrk teleoperation mode.
// At the moment the dvrk does not publish the foot pedals status if the
// dvrk-console is not run in teleop mode. That's why we run the dvrk-console
// in a normal teleop mode, but Home the arms through this node (instead of
// the user interface) so that we can power up only the masters and not the
// slaves that are not needed here.



// ------------------------------------- global variables ---------------------------
bool clutch_pressed;
bool coag_pressed;
bool new_coag_msg;
bool new_clutch_msg;
bool new_master_pose[2];
KDL::Frame master_pose[2];
std::string master_state[2];
int8_t control_event;
std_msgs::Float32 gripper_angle[2];

// ------------------------------------- MTM forward kinematics function---------------

// This function should take in 7 joint angles and return end effector position and orientation (quaternion).



// ------------------------------------- callback functions ---------------------------
void ClutchCallback(const sensor_msgs::JoyConstPtr & msg){
    clutch_pressed = (bool)msg->buttons[0];
    new_clutch_msg = true;
}

void CoagCallback(const sensor_msgs::JoyConstPtr & msg){
    coag_pressed = (bool)msg->buttons[0];
    new_coag_msg = true;
    ROS_INFO_STREAM("stop stepping on me!");
}

void Master1PoseCurrentCallback(
    const geometry_msgs::PoseStamped::ConstPtr &msg){
    geometry_msgs::Pose pose = msg->pose;
    tf::poseMsgToKDL(msg->pose, master_pose[0]);
    new_master_pose[0] = true;

}
void Master2PoseCurrentCallback(
    const geometry_msgs::PoseStamped::ConstPtr &msg){
    geometry_msgs::Pose pose = msg->pose;
    tf::poseMsgToKDL(msg->pose, master_pose[1]);
    new_master_pose[1] = true;
}

void Master1StateCallback(
    const std_msgs::String::ConstPtr &msg){
    master_state[0] = msg->data;
    ROS_DEBUG( "Master 1 says: %s" , master_state[0].data());

}

void Master2StateCallback(
    const std_msgs::String::ConstPtr &msg){
    master_state[1] = msg->data;
    ROS_DEBUG( "Master 2 says: %s" , master_state[1].data());

}

void Master1GripperCallback(const std_msgs::Float32ConstPtr &msg) {
    gripper_angle[0].data = msg->data;
}

void Master2GripperCallback(const std_msgs::Float32ConstPtr &msg) {
    gripper_angle[1].data = msg->data;
}

void ControlEventsCallback(const std_msgs::Int8ConstPtr
                                             &msg) {
    
    control_event = msg->data;
    ROS_DEBUG("Received control event %d", control_event);

}


// ------------------------------------- Main ---------------------------
int main(int argc, char * argv[]) {

    ros::init(argc, argv, "teleop_dummy");
    ros::NodeHandle n(ros::this_node::getName());

    if( ros::console::set_logger_level(
        ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info) )
        ros::console::notifyLoggerLevelsChanged();

    // ------------------------------------- Clutches---------------------------
    ros::Subscriber sub_clutch_clutch = n.subscribe(
        "/dvrk/footpedals/clutch", 1, ClutchCallback);

    ros::Subscriber sub_coag_clutch = n.subscribe(
        "/dvrk/footpedals/coag", 1, CoagCallback);

    // ------------------------------------- ARMS---------------------------
    std::string slave_names[2];
    std::string master_names[2];
    int num_arms = 0;

    if (n.getParam("slave_1_name", slave_names[0]))
        num_arms = 1;
    if (n.getParam("slave_2_name", slave_names[1]))
        num_arms = 2;

    if(num_arms==0)
        return 0;

    n.getParam("master_1_name", master_names[0]);
    n.getParam("master_2_name", master_names[1]);

    // ------------ MATERS POSE
    std::stringstream param_name;
    param_name << std::string("/dvrk/") << master_names[0]
               << "/position_cartesian_current";
    ros::Subscriber sub_master_1_current_pose =  n.subscribe(param_name.str(),
                                                             1, Master1PoseCurrentCallback);

    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[1]
               << "/position_cartesian_current";
    ros::Subscriber sub_master_2_current_pose =  n.subscribe(param_name.str(),
                                                             1, Master2PoseCurrentCallback);

    // ------------ MASTERS GET GRIPPER
    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[0]
               << "/gripper_position_current";
    ros::Subscriber sub_master_1_gripper = n.subscribe(param_name.str(), 1, Master1GripperCallback);
    
    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[1]
               << "/gripper_position_current";
    ros::Subscriber sub_master_2_gripper = n.subscribe(param_name.str(), 1, Master2GripperCallback);
    
    // ------------ MASTERS GET STATE
    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[0]
               << "/robot_state";
    ros::Subscriber sub_master_1_state =  n.subscribe(param_name.str(),
                                                      1, Master1StateCallback);

    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[1]
               << "/robot_state";
    ros::Subscriber sub_master_2_state =  n.subscribe(param_name.str(),
                                                      1, Master2StateCallback);

    // ------------ MATERS SET STATE
    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[0]
               << "/set_robot_state";
    ros::Publisher pub_master_1_state = n.advertise<std_msgs::String>(param_name.str(), 2, true);

    param_name.str("");
    param_name << std::string("/dvrk/") << master_names[1]
               << "/set_robot_state";
    ros::Publisher pub_master_2_state = n.advertise<std_msgs::String>(param_name.str(), 2);

    // ------------ SLAVE PUBLISH POSE
    param_name.str("");
    param_name << std::string("/dvrk/") << slave_names[0]
               << "/position_cartesian_current";
    ros::Publisher pub_slave_1_pose = n.advertise<geometry_msgs::PoseStamped>(param_name.str(), 2);

    param_name.str("");
    param_name << std::string("/dvrk/") << slave_names[1]
               << "/position_cartesian_current";
    ros::Publisher pub_slave_2_pose = n.advertise<geometry_msgs::PoseStamped>(param_name.str(), 2);
    
    // ------------ SLAVE PUBLISH Gripper
    param_name.str("");
    param_name << std::string("/dvrk/") << slave_names[0]
               << "/gripper_position_current";
    ros::Publisher pub_slave_1_gripper = n.advertise<std_msgs::Float32>(param_name.str(), 1);
    
    param_name.str("");
    param_name << std::string("/dvrk/") << slave_names[1]
               << "/gripper_position_current";
    ros::Publisher pub_slave_2_gripper = n.advertise<std_msgs::Float32>(param_name.str(), 1);
    
    // ------------ subscribe to control events that come from the GUI
    ros::Subscriber sub_control_events = n.subscribe("/atar/control_events", 1,
                                                     ControlEventsCallback);

    double scaling = 0.2;
    n.getParam("scaling", scaling);
    ROS_INFO(" Master to slave position scaling: %f", scaling);

    // spinning freq, publishing freq will be according to the freq of master poses received
    ros::Rate loop_rate(1000);

    ros::Rate loop_rate_slow(1);
    ros::spinOnce();

    loop_rate_slow.sleep();
    ros::spinOnce();

    std_msgs::String string_msg;
//    if(master_state[0].data() != std::string("DVRK_READY")) {
//        string_msg.data = "Home";
//        pub_master_1_state.publish(string_msg);
//        ROS_INFO( "Attempting to Home %s", master_names[0].c_str());
//    } else
//        ROS_INFO( "%s is alreade Homed.", master_names[0].c_str());
//
//    // second master arm
//    if(num_arms==2) {
//        if (master_state[1].data() != std::string("DVRK_READY")) {
//            string_msg.data = "Home";
//            pub_master_2_state.publish(string_msg);
//            ROS_INFO( "Attempting to Home %s", master_names[1].c_str());
//        } else
//            ROS_INFO( "%s is alreade Homed.", master_names[1].c_str());
//    }

    KDL::Vector master_position_at_clutch_instance[2];
    KDL::Vector slave_position_at_clutch_instance[2];
    KDL::Frame slave_pose[2];

    // get initial tool position
    std::vector<double> init_tool_position[2]= {{0., 0., 0.} , {0., 0., 0.}};
    n.getParam("initial_slave1_position", init_tool_position[0]);
    n.getParam("initial_slave2_position", init_tool_position[1]);

    slave_pose[0].p = KDL::Vector(init_tool_position[0][0],
                                  init_tool_position[0][1],
                                  init_tool_position[0][2]);
    slave_pose[1].p = KDL::Vector(init_tool_position[1][0],
                                  init_tool_position[1][1],
                                  init_tool_position[1][2]);

    while(ros::ok()){

        if(control_event==CE_HOME_MASTERS){
            control_event = -1;
            std_msgs::String string_msg;
            string_msg.data = "Home";

            if(master_state[0].data() != std::string("DVRK_READY")) {
                pub_master_1_state.publish(string_msg);
                ROS_INFO( "Attempting to Home %s", master_names[0].c_str());
            } else
                ROS_INFO( "%s is alreade Homed.", master_names[0].c_str());
            // second master arm
            if(num_arms==2) {
                if (master_state[1].data() != std::string("DVRK_READY")) {
                    pub_master_2_state.publish(string_msg);
                    ROS_INFO( "Attempting to Home %s", master_names[1].c_str());
                } else
                    ROS_INFO( "%s is alreade Homed.", master_names[1].c_str());
            }
        }
    
        if(control_event==CE_EXIT)
            ros::shutdown();

            
            if(new_coag_msg || new_clutch_msg){
            new_coag_msg = false;

            if(coag_pressed && !clutch_pressed){
                string_msg.data = "DVRK_EFFORT_CARTESIAN";
                pub_master_1_state.publish(string_msg);
                master_position_at_clutch_instance[0] = master_pose[0].p;
                slave_position_at_clutch_instance[0] = slave_pose[0].p;
                if(num_arms==2){
                    pub_master_2_state.publish(string_msg);
                    master_position_at_clutch_instance[1] = master_pose[1].p;
                    slave_position_at_clutch_instance[1] = slave_pose[1].p;
                }
            }

            if(!coag_pressed){
                string_msg.data = "DVRK_POSITION_CARTESIAN";
                pub_master_1_state.publish(string_msg);
                if(num_arms==2)
                    pub_master_2_state.publish(string_msg);
            }
        }
        if(new_clutch_msg){
            new_clutch_msg = false;

            if(coag_pressed&& clutch_pressed){
                string_msg.data = "DVRK_CLUTCH";
                pub_master_1_state.publish(string_msg);
                if(num_arms==2)
                    pub_master_2_state.publish(string_msg);
            }
        }

        // incremental slave position

        if(new_master_pose[0]) {
            new_master_pose[0] = false;

            // if operator present increment the slave position
            if(coag_pressed && !clutch_pressed){
                slave_pose[0].p = slave_position_at_clutch_instance[0] +
                    scaling * (master_pose[0].p - master_position_at_clutch_instance[0]);
                slave_pose[0].M = master_pose[0].M;
            }

            //publish pose
            geometry_msgs::PoseStamped pose;
            tf::poseKDLToMsg( slave_pose[0], pose.pose);
            pub_slave_1_pose.publish(pose);
        }

        if(num_arms==2) {
            if (new_master_pose[1]) {
                new_master_pose[1] = false;

                // if operator present increment the slave position
                if (coag_pressed && !clutch_pressed) {
                    slave_pose[1].p = slave_position_at_clutch_instance[1] +
                        scaling * (master_pose[1].p
                            - master_position_at_clutch_instance[1]);
                    slave_pose[1].M = master_pose[1].M;
                }
                //publish pose
                geometry_msgs::PoseStamped pose;
                tf::poseKDLToMsg(slave_pose[1], pose.pose);
                pub_slave_2_pose.publish(pose);

            }
        }

        // publish the gripper positions
        pub_slave_1_gripper.publish(gripper_angle[0]);
        pub_slave_2_gripper.publish(gripper_angle[1]);

        ros::spinOnce();
        loop_rate.sleep();


    }

}
