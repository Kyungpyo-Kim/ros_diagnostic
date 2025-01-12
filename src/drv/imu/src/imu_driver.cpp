#include <random>

#include "imu_driver.h"


using namespace diagnostic_updater;


void ImuDriver::init(
    ros::NodeHandle &nh,
    diagnostic_updater::Updater &updater
){
    state_ = true;

    ros::Publisher pub_imu = nh.advertise<sensor_msgs::Imu>("imu", 10);

    freq_ = 20;
    min_freq_ = 18;
    max_freq_ = 22;

    dpub_imu_ = std::make_shared<DiagnosedPublisher<sensor_msgs::Imu>>(
          pub_imu, updater,
          FrequencyStatusParam(&min_freq_, &max_freq_, 0.02, 10),
          TimeStampStatusParam(-1.0, 1.0));

    // updater
    updater_ = std::shared_ptr<Updater>(&updater);
    updater_->add(state_task_);

    // threading
    exec_thread_ = std::thread(&ImuDriver::execLoop, this);
    diag_thread_ = std::thread(&ImuDriver::diagnosticUpdate, this);
}


void ImuDriver::execLoop(){
  
  while (!exec_terminating_) {
    std::unique_lock<std::mutex> lock(exec_mutex_);

    // toggle state
    state_ = !state_;
    state_task_.setState(state_);

    // random
    std::random_device random_device;
    std::mt19937 random_generator(random_device());
    std::normal_distribution<> normal_time_distribution(0, 0.35);
    std::normal_distribution<> normal_freq_distribution(0, 5.00);

    // update publish data
    sensor_msgs::Imu imu_data;
    ros::Duration time_offset(normal_time_distribution(random_generator));
    imu_data.header.stamp = ros::Time::now() + time_offset;
    dpub_imu_->publish(imu_data);
    
    // periodic threding
    double freq = freq_ + normal_freq_distribution(random_generator);
    exec_cv_.wait_for(lock,
                      std::chrono::milliseconds(static_cast<int>(1000 / freq)),
                      [&] { return exec_terminating_.load(); });
  }
}


void ImuDriver::diagnosticUpdate(){
  
  while (!diag_terminating_) {
    
    // update
    updater_->update();
  
    std::unique_lock<std::mutex> lock(diag_mutex_);

    // periodic threding
    diag_cv_.wait_for(lock,
                      std::chrono::milliseconds(static_cast<int>(1000 / 50)),
                      [&] { return diag_terminating_.load(); });
  }
}