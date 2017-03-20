
#include <iostream>
#include "Eigen/Dense"
#include <vector>
#include "fusion_ukf.h"
#include "measurement_package.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>

void check_arguments(int argc, char *argv[])
{
    std::string usage_instructions = "Usage instructions: ";
    usage_instructions += argv[0];
    usage_instructions += " path/to/input.txt output.txt";

    bool has_valid_args = false;

    // make sure the user has provided input and output files
    if (argc == 1)
    {
        std::cerr << usage_instructions << std::endl;
    }
    else if (argc == 2)
    {
        std::cerr << "Please include an output file.\n" << usage_instructions <<
                  std::endl;
    }
    else if (argc == 3)
    {
        has_valid_args = true;
    }
    else if (argc > 3)
    {
        std::cerr << "Too many arguments.\n" << usage_instructions << std::endl;
    }

    if (!has_valid_args)
    {
        exit(EXIT_FAILURE);
    }
}

void check_files(std::ifstream& in_file, std::string& in_name,
                 std::ofstream& out_file, std::string& out_name)
{
    if (!in_file.is_open())
    {
        std::cerr << "Cannot open input file: " << in_name << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!out_file.is_open())
    {
        std::cerr << "Cannot open output file: " << out_name << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{

    check_arguments(argc, argv);

    std::string in_file_name_ = argv[1];
    std::ifstream in_file_(in_file_name_.c_str(), std::ifstream::in);

    std::string out_file_name_ = argv[2];
    std::ofstream out_file_(out_file_name_.c_str(), std::ofstream::out);

    check_files(in_file_, in_file_name_, out_file_, out_file_name_);

    /**********************************************
     *  Set Measurements                          *
     **********************************************/

    std::vector<MeasurementPackage> measurement_pack_list;
    std::string line;

    // prep the measurement packages (each line represents a measurement at a
    // timestamp)
    while (std::getline(in_file_, line))
    {
        std::string sensor_type;
        MeasurementPackage meas_package;
        std::istringstream iss(line);
        long timestamp;

        // reads first element from the current line
        iss >> sensor_type;

        if (sensor_type.compare("L") == 0)
        {
            // laser measurement

            // read measurements at this timestamp
            meas_package.sensor_type_ = MeasurementPackage::LASER;
            meas_package.raw_measurements_ = Eigen::VectorXd(2);
            float px;
            float py;
            iss >> px;
            iss >> py;
            meas_package.raw_measurements_ << px, py;
            iss >> timestamp;
            meas_package.timestamp_ = timestamp;
            measurement_pack_list.push_back(meas_package);
        }
        else if (sensor_type.compare("R") == 0)
        {
            // radar measurement

            // read measurements at this timestamp
            meas_package.sensor_type_ = MeasurementPackage::RADAR;
            meas_package.raw_measurements_ = Eigen::VectorXd(3);
            float ro;
            float theta;
            float ro_dot;
            iss >> ro;
            iss >> theta;
            iss >> ro_dot;
            meas_package.raw_measurements_ << ro, theta, ro_dot;
            iss >> timestamp;
            meas_package.timestamp_ = timestamp;
            measurement_pack_list.push_back(meas_package);
        }
    }

    // Create a UKF instance
    FusionUKF fusion_ukf = FusionUKF();

    size_t number_of_measurements = measurement_pack_list.size();

    // start filtering from the second frame (the speed is unknown in the first
    // frame)
    for (size_t k = 0; k < number_of_measurements; ++k)
    {
        // Call the UKF-based fusion
        fusion_ukf.processMeasurement(measurement_pack_list[k]);

        // output the estimation
        const Eigen::VectorXd& x = fusion_ukf.getState();
        out_file_ << x(0) << "\t"; // pos1 - est
        out_file_ << x(1) << "\t"; // pos2 - est
        out_file_ << x(2) << "\t"; // vel_abs -est
        out_file_ << x(3) << "\t"; // yaw_angle -est
        out_file_ << x(4) << "\t"; // yaw_rate -est

        // output the measurements
        if (measurement_pack_list[k].sensor_type_ == MeasurementPackage::LASER)
        {
            // output the estimation

            // p1 - meas
            out_file_ << measurement_pack_list[k].raw_measurements_(0) << "\t";

            // p2 - meas
            out_file_ << measurement_pack_list[k].raw_measurements_(1) << "\t";
        }
        else if (measurement_pack_list[k].sensor_type_ == MeasurementPackage::RADAR)
        {
            // output the estimation in the cartesian coordinates
            float ro = measurement_pack_list[k].raw_measurements_(0);
            float phi = measurement_pack_list[k].raw_measurements_(1);
            out_file_ << ro *cos(phi) << "\t";  // p1_meas
            out_file_ << ro *sin(phi) << "\t";  // p2_meas
        }

        out_file_ << "\n";
    }

    // close files
    if (out_file_.is_open())
    {
        out_file_.close();
    }

    if (in_file_.is_open())
    {
        in_file_.close();
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
