#pragma once

// std
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

// ros
#include <ros/ros.h>
#include <diagnostic_updater/publisher.h>
#include <diagnostic_updater/diagnostic_updater.h>
#include <sensor_msgs/Imu.h>


#include <diagnostic_tasks.h>


class ImuDriver{

public:
    ImuDriver()
    :state_tast_(true){}

    ~ImuDriver(){}
    void init(
        ros::NodeHandle &nh,
        diagnostic_updater::Updater &updater
    );

private:
    void exceLoop();
    void stateUpdate();

    std::mutex exec_mutex_;
    std::condition_variable exec_cv_;
    std::atomic_bool terminating_;
    std::thread exec_thread_;

    std::shared_ptr<diagnostic_updater::Updater> updater_;
    std::shared_ptr<diagnostic_updater::DiagnosedPublisher<sensor_msgs::Imu>> dpub_imu_;

    double freq_, min_freq_, max_freq_;

    bool state_;

    StateTask state_tast_;
};