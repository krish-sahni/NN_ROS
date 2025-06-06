#include "depthai_ros_driver/param_handlers/stereo_param_handler.hpp"

#include "depthai-shared/common/CameraFeatures.hpp"
#include "depthai-shared/datatype/RawStereoDepthConfig.hpp"
#include "depthai/pipeline/datatype/StereoDepthConfig.hpp"
#include "depthai/pipeline/node/StereoDepth.hpp"
#include "depthai_ros_driver/utils.hpp"
#include "rclcpp/logger.hpp"
#include "rclcpp/node.hpp"

namespace depthai_ros_driver {
namespace param_handlers {
StereoParamHandler::StereoParamHandler(std::shared_ptr<rclcpp::Node> node, const std::string& name) : BaseParamHandler(node, name) {
    depthPresetMap = {{"HIGH_ACCURACY", dai::node::StereoDepth::PresetMode::HIGH_ACCURACY},
                      {"HIGH_DENSITY", dai::node::StereoDepth::PresetMode::HIGH_DENSITY},
                      {"DEFAULT", dai::node::StereoDepth::PresetMode::DEFAULT},
                      {"FACE", dai::node::StereoDepth::PresetMode::FACE},
                      {"HIGH_DETAIL", dai::node::StereoDepth::PresetMode::HIGH_DETAIL},
                      {"ROBOTICS", dai::node::StereoDepth::PresetMode::ROBOTICS}};

    disparityWidthMap = {
        {"DISPARITY_64", dai::StereoDepthConfig::CostMatching::DisparityWidth::DISPARITY_64},
        {"DISPARITY_96", dai::StereoDepthConfig::CostMatching::DisparityWidth::DISPARITY_96},
    };

    decimationModeMap = {{"PIXEL_SKIPPING", dai::StereoDepthConfig::PostProcessing::DecimationFilter::DecimationMode::PIXEL_SKIPPING},
                         {"NON_ZERO_MEDIAN", dai::StereoDepthConfig::PostProcessing::DecimationFilter::DecimationMode::NON_ZERO_MEDIAN},
                         {"NON_ZERO_MEAN", dai::StereoDepthConfig::PostProcessing::DecimationFilter::DecimationMode::NON_ZERO_MEAN}};

    temporalPersistencyMap = {
        {"PERSISTENCY_OFF", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::PERSISTENCY_OFF},
        {"VALID_8_OUT_OF_8", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_8_OUT_OF_8},
        {"VALID_2_IN_LAST_3", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_2_IN_LAST_3},
        {"VALID_2_IN_LAST_4", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_2_IN_LAST_4},
        {"VALID_2_OUT_OF_8", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_2_OUT_OF_8},
        {"VALID_1_IN_LAST_2", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_1_IN_LAST_2},
        {"VALID_1_IN_LAST_5", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_1_IN_LAST_5},
        {"VALID_1_IN_LAST_8", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::VALID_1_IN_LAST_8},
        {"PERSISTENCY_INDEFINITELY", dai::StereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode::PERSISTENCY_INDEFINITELY},
    };
}

StereoParamHandler::~StereoParamHandler() = default;

void StereoParamHandler::updateSocketsFromParams(dai::CameraBoardSocket& left, dai::CameraBoardSocket& right, dai::CameraBoardSocket& align) {
    int newLeftS = declareAndLogParam<int>("i_left_socket_id", static_cast<int>(left));
    int newRightS = declareAndLogParam<int>("i_right_socket_id", static_cast<int>(right));
    alignSocket = static_cast<dai::CameraBoardSocket>(declareAndLogParam<int>("i_board_socket_id", static_cast<int>(align)));
    if(newLeftS != static_cast<int>(left) || newRightS != static_cast<int>(right)) {
        RCLCPP_WARN(getROSNode()->get_logger(), "Left or right socket changed, updating stereo node");
        RCLCPP_WARN(getROSNode()->get_logger(), "Old left socket: %d, new left socket: %d", static_cast<int>(left), newLeftS);
        RCLCPP_WARN(getROSNode()->get_logger(), "Old right socket: %d, new right socket: %d", static_cast<int>(right), newRightS);
    }
    left = static_cast<dai::CameraBoardSocket>(newLeftS);
    right = static_cast<dai::CameraBoardSocket>(newRightS);
}

void StereoParamHandler::declareParams(std::shared_ptr<dai::node::StereoDepth> stereo) {
    declareAndLogParam<int>("i_max_q_size", 30);
    bool lowBandwidth = declareAndLogParam<bool>("i_low_bandwidth", false);
    declareAndLogParam<int>("i_low_bandwidth_quality", 50);
    declareAndLogParam<int>("i_low_bandwidth_profile", 4);
    declareAndLogParam<int>("i_low_bandwidth_frame_freq", 30);
    declareAndLogParam<int>("i_low_bandwidth_bitrate", 0);
    declareAndLogParam<std::string>("i_low_bandwidth_ffmpeg_encoder", "libx264");
    declareAndLogParam<bool>("i_output_disparity", false);
    declareAndLogParam<bool>("i_get_base_device_timestamp", false);
    declareAndLogParam<bool>("i_update_ros_base_time_on_ros_msg", false);
    declareAndLogParam<bool>("i_publish_topic", true);
    declareAndLogParam<bool>("i_add_exposure_offset", false);
    declareAndLogParam<int>("i_exposure_offset", 0);
    declareAndLogParam<bool>("i_enable_lazy_publisher", true);
    declareAndLogParam<bool>("i_reverse_stereo_socket_order", false);
    declareAndLogParam<bool>("i_publish_compressed", false);
    declareAndLogParam<std::string>("i_calibration_file", "");

    declareAndLogParam<bool>("i_publish_synced_rect_pair", false);
    declareAndLogParam<bool>("i_left_rect_publish_topic", false);
    declareAndLogParam<bool>("i_left_rect_low_bandwidth", false);
    declareAndLogParam<int>("i_left_rect_low_bandwidth_profile", 4);
    declareAndLogParam<int>("i_left_rect_low_bandwidth_frame_freq", 30);
    declareAndLogParam<int>("i_left_rect_low_bandwidth_bitrate", 0);
    declareAndLogParam<int>("i_left_rect_low_bandwidth_quality", 50);
    declareAndLogParam<std::string>("i_left_rect_low_bandwidth_ffmpeg_encoder", "libx264");
    declareAndLogParam<bool>("i_left_rect_add_exposure_offset", false);
    declareAndLogParam<int>("i_left_rect_exposure_offset", 0);
    declareAndLogParam<bool>("i_left_rect_enable_feature_tracker", false);
    declareAndLogParam<bool>("i_left_rect_synced", false);
    declareAndLogParam<bool>("i_left_rect_publish_compressed", false);

    declareAndLogParam<bool>("i_right_rect_publish_topic", false);
    declareAndLogParam<bool>("i_right_rect_low_bandwidth", false);
    declareAndLogParam<int>("i_right_rect_low_bandwidth_quality", 50);
    declareAndLogParam<int>("i_right_rect_low_bandwidth_profile", 4);
    declareAndLogParam<int>("i_right_rect_low_bandwidth_frame_freq", 30);
    declareAndLogParam<int>("i_right_rect_low_bandwidth_bitrate", 0);
    declareAndLogParam<std::string>("i_right_rect_low_bandwidth_ffmpeg_encoder", "libx264");
    declareAndLogParam<bool>("i_right_rect_enable_feature_tracker", false);
    declareAndLogParam<bool>("i_right_rect_add_exposure_offset", false);
    declareAndLogParam<int>("i_right_rect_exposure_offset", 0);
    declareAndLogParam<bool>("i_right_rect_synced", false);
    declareAndLogParam<bool>("i_right_rect_publish_compressed", false);

    declareAndLogParam<bool>("i_enable_spatial_nn", false);
    declareAndLogParam<std::string>("i_spatial_nn_source", "right");
    declareAndLogParam<bool>("i_synced", false);

    stereo->setLeftRightCheck(declareAndLogParam<bool>("i_lr_check", true));
    int width = 1280;
    int height = 720;
    std::string socketName;
    if(declareAndLogParam<bool>("i_align_depth", true)) {
        socketName = getSocketName(alignSocket);
        try {
            width = getROSNode()->get_parameter(socketName + ".i_width").as_int();
            height = getROSNode()->get_parameter(socketName + ".i_height").as_int();
        } catch(rclcpp::exceptions::ParameterNotDeclaredException& e) {
            RCLCPP_ERROR(getROSNode()->get_logger(), "%s parameters not set, defaulting to 1280x720 unless specified otherwise.", socketName.c_str());
        }
        declareAndLogParam<std::string>("i_socket_name", socketName);
        stereo->setDepthAlign(alignSocket);
    }

    if(declareAndLogParam<bool>("i_set_input_size", false)) {
        stereo->setInputResolution(declareAndLogParam<int>("i_input_width", 1280), declareAndLogParam<int>("i_input_height", 720));
    }
    width = declareAndLogParam<int>("i_width", width);
    height = declareAndLogParam<int>("i_height", height);
    stereo->setOutputSize(width, height);
    stereo->setDefaultProfilePreset(depthPresetMap.at(declareAndLogParam<std::string>("i_depth_preset", "HIGH_ACCURACY")));
    if(declareAndLogParam<bool>("i_enable_distortion_correction", false)) {
        stereo->enableDistortionCorrection(true);
    }
    if(declareAndLogParam<bool>("i_set_disparity_to_depth_use_spec_translation", false)) {
        stereo->setDisparityToDepthUseSpecTranslation(true);
    }

    stereo->initialConfig.setBilateralFilterSigma(declareAndLogParam<int>("i_bilateral_sigma", 0));
    stereo->initialConfig.setLeftRightCheckThreshold(declareAndLogParam<int>("i_lrc_threshold", 10));
    stereo->initialConfig.setMedianFilter(static_cast<dai::MedianFilter>(declareAndLogParam<int>("i_depth_filter_size", 5)));
    stereo->initialConfig.setConfidenceThreshold(declareAndLogParam<int>("i_stereo_conf_threshold", 240));
    if(declareAndLogParam<bool>("i_subpixel", true) && !lowBandwidth) {
        stereo->initialConfig.setSubpixel(true);
        stereo->initialConfig.setSubpixelFractionalBits(declareAndLogParam<int>("i_subpixel_fractional_bits", 3));
    } else {
        stereo->initialConfig.setSubpixel(false);
        RCLCPP_INFO(getROSNode()->get_logger(), "Subpixel disabled due to low bandwidth mode");
    }
    stereo->setRectifyEdgeFillColor(declareAndLogParam<int>("i_rectify_edge_fill_color", 0));
    if(declareAndLogParam<bool>("i_enable_alpha_scaling", false)) {
        stereo->setAlphaScaling(declareAndLogParam<float>("i_alpha_scaling", 0.0));
    }
    dai::RawStereoDepthConfig config = stereo->initialConfig.get();
    config.costMatching.disparityWidth = utils::getValFromMap(declareAndLogParam<std::string>("i_disparity_width", "DISPARITY_96"), disparityWidthMap);
    stereo->setExtendedDisparity(declareAndLogParam<bool>("i_extended_disp", false));
    config.costMatching.enableCompanding = declareAndLogParam<bool>("i_enable_companding", false);
    if(declareAndLogParam<bool>("i_enable_temporal_filter", false)) {
        config.postProcessing.temporalFilter.enable = true;
        config.postProcessing.temporalFilter.alpha = declareAndLogParam<float>("i_temporal_filter_alpha", 0.4);
        config.postProcessing.temporalFilter.delta = declareAndLogParam<int>("i_temporal_filter_delta", 20);
        config.postProcessing.temporalFilter.persistencyMode =
            utils::getValFromMap(declareAndLogParam<std::string>("i_temporal_filter_persistency", "VALID_2_IN_LAST_4"), temporalPersistencyMap);
    }
    if(declareAndLogParam<bool>("i_enable_speckle_filter", false)) {
        config.postProcessing.speckleFilter.enable = true;
        config.postProcessing.speckleFilter.speckleRange = declareAndLogParam<int>("i_speckle_filter_speckle_range", 50);
    }
    if(declareAndLogParam<bool>("i_enable_disparity_shift", false)) {
        config.algorithmControl.disparityShift = declareAndLogParam<int>("i_disparity_shift", 0);
    }

    if(declareAndLogParam<bool>("i_enable_spatial_filter", false)) {
        config.postProcessing.spatialFilter.enable = true;
        config.postProcessing.spatialFilter.holeFillingRadius = declareAndLogParam<int>("i_spatial_filter_hole_filling_radius", 2);
        config.postProcessing.spatialFilter.alpha = declareAndLogParam<float>("i_spatial_filter_alpha", 0.5);
        config.postProcessing.spatialFilter.delta = declareAndLogParam<int>("i_spatial_filter_delta", 20);
        config.postProcessing.spatialFilter.numIterations = declareAndLogParam<int>("i_spatial_filter_iterations", 1);
    }
    if(declareAndLogParam<bool>("i_enable_threshold_filter", false)) {
        config.postProcessing.thresholdFilter.minRange = declareAndLogParam<int>("i_threshold_filter_min_range", 400);
        config.postProcessing.thresholdFilter.maxRange = declareAndLogParam<int>("i_threshold_filter_max_range", 15000);
    }
    if(declareAndLogParam<bool>("i_enable_brightness_filter", false)) {
        config.postProcessing.brightnessFilter.minBrightness = declareAndLogParam<int>("i_brightness_filter_min_brightness", 0);
        config.postProcessing.brightnessFilter.maxBrightness = declareAndLogParam<int>("i_brightness_filter_max_brightness", 256);
    }
    if(declareAndLogParam<bool>("i_enable_decimation_filter", false)) {
        config.postProcessing.decimationFilter.decimationMode =
            utils::getValFromMap(declareAndLogParam<std::string>("i_decimation_filter_decimation_mode", "PIXEL_SKIPPING"), decimationModeMap);
        config.postProcessing.decimationFilter.decimationFactor = declareAndLogParam<int>("i_decimation_filter_decimation_factor", 1);
        int decimatedWidth = width / config.postProcessing.decimationFilter.decimationFactor;
        int decimatedHeight = height / config.postProcessing.decimationFilter.decimationFactor;
        RCLCPP_INFO(getROSNode()->get_logger(),
                    "Decimation filter enabled with decimation factor %d. Previous width and height: %d x %d, after decimation: %d x %d",
                    config.postProcessing.decimationFilter.decimationFactor,
                    width,
                    height,
                    decimatedWidth,
                    decimatedHeight);
        stereo->setOutputSize(decimatedWidth, decimatedHeight);
        declareAndLogParam("i_width", decimatedWidth, true);
        declareAndLogParam("i_height", decimatedHeight, true);
    }
    stereo->initialConfig.set(config);
}
dai::CameraControl StereoParamHandler::setRuntimeParams(const std::vector<rclcpp::Parameter>& /*params*/) {
    dai::CameraControl ctrl;
    return ctrl;
}
}  // namespace param_handlers
}  // namespace depthai_ros_driver
