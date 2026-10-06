#include "shape_detector.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// CLI11 header
#include <CLI/CLI.hpp>
#include <json.hpp>

int main(int argc, char *argv[]) {
    CLI::App app{"GoCoPP - Finding Good Configurations of Planar Primitives in "
                 "Unorganized Point Clouds"};

    // Program description
    app.description("Implementation of the paper 'Finding Good Configurations "
                    "of Planar Primitives in Unorganized Point Clouds' by "
                    "Mulin Yu and Florent Lafarge (CVPR 2022). For more "
                    "information, see: https://hal.inria.fr/hal-03621896");

    // Set version flag
    app.set_version_flag("--version", "0.1.0",
                         "Display program version and exit");

    // Positional argument: input file
    std::string path_point_cloud;
    CLI::Option *input_opt =
        app.add_option("input_file", path_point_cloud)
            ->description("The input PLY file should contain coordinates (x, "
                          "y, z). Normals (nx, ny, nz) are encouraged but will "
                          "be estimated if not provided.")
            ->required()
            ->check(CLI::ExistingFile);

    // Output directory
    std::optional<std::string> path_dir_output;
    app.add_option("--output_dir", path_dir_output)
        ->description("Directory where output files will be saved.")
        ->default_val(std::nullopt)
        ->default_str("Parent of the input file.");

    // Algorithm parameters group
    CLI::Option_group *alg_group = app.add_option_group(
        "Algorithm Parameters",
        "Parameters controlling the shape detection algorithm");

    // Epsilon parameter
    std::optional<double> pd_epsilon;
    alg_group->add_option("--epsilon", pd_epsilon)
        ->description("Maximal distance of an inlier to its supporting plane.")
        ->default_val(std::nullopt)
        ->default_str("0.4% * bbox diagonal");

    // Sigma parameter
    int pd_sigma;
    alg_group->add_option("--sigma", pd_sigma)
        ->description("Discards primitives with too few inliers.")
        ->default_val(50);

    // K-Nearest Neighbors
    int pd_nn;
    alg_group->add_option("--nn", pd_nn)
        ->description("k for k-nearest neighbor graph and normal estimation.")
        ->default_val(20);

    // Normal deviation threshold
    double pd_normal_deviation;
    alg_group->add_option("--normal_deviation", pd_normal_deviation)
        ->description("Minimum cosine of the angle between an inlier and its "
                      "supporting plane (for region growing) and between two "
                      "primitives that can be merged.")
        ->default_val(0.85);

    // Maximum iterations
    int stop_iterations;
    alg_group->add_option("--max_steps", stop_iterations)
        ->description("Maximum iterations of the exploration mechanism.")
        ->default_val(7);

    // Norm/fidelity metric
    std::string pd_norm;
    alg_group->add_option("--norm", pd_norm, "Fidelity metric")
        ->description("Fidelity metric to use: 'normal', 'L2', or 'hybrid'.\n"
                      "  - 'normal': Uses deviation between inliers' normals "
                      "and supporting plane normals.\n"
                      "  - 'L2': Uses Euclidean distance between inliers and "
                      "supporting planes.\n"
                      "  - 'hybrid': Uses L2 in priority queue and normal in "
                      "transfer operator.")
        ->default_val("hybrid")
        ->check([](const std::string &str) {
            if (str != "normal" && str != "L2" && str != "hybrid") {
                return "Invalid norm value. Must be 'normal', 'L2', or "
                       "'hybrid'.";
            }
            return "";
        });

    // Weight parameters group
    CLI::Option_group *weight_group = app.add_option_group(
        "Weight Parameters", "Parameters controlling the trade-off between "
                             "simplicity and completeness");

    double pd_s;
    weight_group->add_option("--s", pd_s)
        ->description("Weight for simplicity (0 < s + c < 3).")
        ->default_val(1.0);

    double pd_c;
    weight_group->add_option("--c", pd_c)
        ->description("Weight for completeness (0 < s + c < 3).")
        ->default_val(1.0);

    int pd_weight_mode;
    weight_group->add_option("--weight_mode", pd_weight_mode)
        ->description("Weight mode for the optimization.")
        ->default_val(0);

    // Constraint and validation parameters group
    CLI::Option_group *constraint_group = app.add_option_group(
        "Constraints and Validation",
        "Parameters controlling constraints and validation");

    bool pd_if_constraint;
    constraint_group->add_flag("--constraint", pd_if_constraint)
        ->description("Ensures the final configuration does not degrade "
                      "fidelity, simplicity, and completeness of the initial "
                      "configuration.")
        ->default_val(false);

    // Output options group
    CLI::Option_group *output_group = app.add_option_group(
        "Output Options", "Parameters controlling what outputs to generate");

    bool pd_out_vg;
    output_group->add_flag("--vg", pd_out_vg)
        ->description("Output results in Vertex Group (VG) format. See: "
                      "https://github.com/LiangliangNan/PolyFit#data")
        ->default_val(false);

    bool pd_out_alpha_shape;
    output_group->add_flag("--alpha", pd_out_alpha_shape)
        ->description("Generate primitives represented as alpha shapes.")
        ->default_val(true);

    std::optional<double> alpha_val;
    output_group->add_option("--alpha_val", alpha_val)
        ->description("Alpha value for alpha shapes.")
        ->default_val(std::nullopt)
        ->default_str("0.5% * bbox diagonal");

    bool pd_out_convex_hull;
    output_group->add_flag("--hull", pd_out_convex_hull)
        ->description("Generate primitives represented as convex hulls.")
        ->default_val(false);

    CLI11_PARSE(app, argc, argv);

    // Set default output directory if not specified
    if (!path_dir_output.has_value()) {
        path_dir_output =
            boost::filesystem::path(path_point_cloud).parent_path().string();
    }

    // Create Shape_Detector instance
    Shape_Detector *CS;

    if (!boost::filesystem::exists(path_point_cloud)) {
        std::cerr << "Error: Input file not found: " << path_point_cloud
                  << std::endl
                  << std::endl;
        return 1;
    }

    CS = new Shape_Detector(path_point_cloud, path_dir_output.value());
    CS->set_detection_parameters(pd_sigma, pd_nn, pd_normal_deviation);
    CS->set_max_steps(stop_iterations);
    CS->set_weight_m(pd_weight_mode);
    CS->set_constraint(pd_if_constraint);
    CS->set_alpha_val(alpha_val);
    CS->set_lambda_r(pd_s);
    CS->set_lambda_c(pd_c);

    if (!CS->load_points()) {
        std::stringstream error_message;
        error_message
            << "Error: The selected point cloud couldn't be processed."
            << std::endl
            << std::endl
            << "Please ensure the file doesn't have any unhandled properties. "
               "The expected properties are: x, y, z, nx, ny, nz."
            << std::endl;
        std::cerr << error_message.str();
        delete CS;
        return 1;
    }

    // Set epsilon: if 0, use default (0.4% of bounding box diagonal)
    if (!pd_epsilon.has_value()) {
        double pd_epsilon_0 = 0.004 * CS->get_bbox_diagonal();
        pd_epsilon = (round(1000 * pd_epsilon_0)) / 1000;
    }

    CS->set_epsilon(pd_epsilon.value());

    // Display configuration
    std::cout << "GoCoPP - Finding Good Configurations of Planar Primitives"
              << std::endl;
    std::cout << "=========================================================="
              << std::endl;
    std::cout << "Input file: " << path_point_cloud << std::endl;
    std::cout << "Output directory: " << path_dir_output.value() << std::endl;
    std::cout << std::endl;
    std::cout << "Algorithm Configuration:" << std::endl;
    std::cout << "  Epsilon: " << pd_epsilon.value() << " (fitting tolerance)"
              << std::endl;
    std::cout << "  Sigma: " << pd_sigma << " (minimal primitive size)"
              << std::endl;
    std::cout << "  KNN: " << pd_nn << " (k-nearest neighbors)" << std::endl;
    std::cout << "  Normal threshold: " << pd_normal_deviation << std::endl;
    std::cout << "  Norm metric: " << pd_norm << std::endl;
    std::cout << "  Max steps: " << stop_iterations << std::endl;
    std::cout << "  Constraint enabled: "
              << (pd_if_constraint ? "true" : "false") << std::endl;
    std::cout << std::endl;
    std::cout << "Weights:" << std::endl;
    std::cout << "  Simplicity (s): " << pd_s << std::endl;
    std::cout << "  Completeness (c): " << pd_c << std::endl;
    std::cout << "  Weight mode: " << pd_weight_mode << std::endl;
    std::cout << std::endl;
    std::cout << "Output Options:" << std::endl;
    std::cout << "  Alpha shapes: "
              << (pd_out_alpha_shape ? "enabled" : "disabled") << std::endl;
    std::cout << "  Convex hulls: "
              << (pd_out_convex_hull ? "enabled" : "disabled") << std::endl;
    std::cout << "  VG format: " << (pd_out_vg ? "enabled" : "disabled")
              << std::endl;
    if (alpha_val) {
        std::cout << "  Alpha value: " << *alpha_val << std::endl;
    }
    std::cout << std::endl;

    // Store the arguments in a JSON file
    std::ofstream arguments_file(path_dir_output.value() + "/arguments.json");
    nlohmann::json j;
    j["input_file"] = path_point_cloud;
    j["output_dir"] = path_dir_output.value();
    j["epsilon"] = pd_epsilon.value();
    j["sigma"] = pd_sigma;
    j["knn"] = pd_nn;
    j["normal_threshold"] = pd_normal_deviation;
    j["max_steps"] = stop_iterations;
    j["constraint"] = pd_if_constraint;
    j["weights"] = {
        {"simplicity", pd_s},
        {"completeness", pd_c},
        {"fidelity", 3 - pd_s - pd_c},
    };
    j["outputs"] = {
        {
            "alpha",
            {"enabled", pd_out_alpha_shape},
            {"alpha", *alpha_val},
        },
        {
            "convex_hull",
            {"enabled", pd_out_convex_hull},
        },
        {
            "vertex_group",
            {"enabled", pd_out_vg},
        },
    };
    arguments_file << std::setw(4) << j << std::endl;

    // Start shape detection
    CS->detect_shapes();
    CS->set_primitives_simple();

    // Run the appropriate detection method based on norm setting
    clock_t t_start = clock();

    if (pd_norm == "normal") {
        CS->planar_shape_detection_L1();
    } else if (pd_norm == "L2") {
        CS->planar_shape_detection_l2();
    } else if (pd_norm == "hybrid") {
        CS->planar_shape_detection_hybrid();
    }

    clock_t t_end = clock();
    double t_all = double(t_end - t_start) / CLOCKS_PER_SEC;

    CS->show_result(t_all);
    CS->set_primitives_simple();

    // Save outputs based on options
    if (pd_out_convex_hull) {
        CS->save_convex_hull();
    }
    if (pd_out_alpha_shape) {
        CS->save_alpha_shapes();
    }
    if (pd_out_vg) {
        CS->to_vg();
    }

    delete CS;
    return 0;
}