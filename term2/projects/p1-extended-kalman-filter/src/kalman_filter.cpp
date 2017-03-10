#include "kalman_filter.h"

KalmanFilter::KalmanFilter(const std::size_t state_dimension):
    x_(Eigen::VectorXd::Zero(state_dimension)),
    P_(Eigen::MatrixXd::Constant(state_dimension, state_dimension, p0_)),
    I_(Eigen::MatrixXd::Identity(state_dimension, state_dimension))
{
}

void KalmanFilter::predict(const MotionModel& motion_model, const double delta_t)
{
    const Eigen::MatrixXd F = motion_model.getTransitionMatrix(delta_t);
    const Eigen::MatrixXd Q = motion_model.getProcessNoise(delta_t);

    x_ = F * x_;
    P_ = F * P_ * F.transpose() + Q;
}

void KalmanFilter::update(const MeasurementModel& sensor_model,
                          const Eigen::VectorXd& z)
{
    const Eigen::MatrixXd& H = sensor_model.getMeasurementMatrix();
    const Eigen::MatrixXd& R = sensor_model.getMeasurementNoise();

    const Eigen::VectorXd y = z - H * x_;

    const Eigen::MatrixXd Ht = H.transpose();

    const Eigen::MatrixXd S = H * P_ * Ht * R;
    const Eigen::MatrixXd K = P_ * Ht * S.inverse();

    x_ = x_ + K * y;
    P_ = (I_ - K * H) * P_;
}
