/**
 * @file
 * @brief
 * TODO
 */
#define DLIB_JPEG_SUPPORT
#define DLIB_PNG_SUPPORT

#include <argagg/argagg.hpp>
#include <dlib/geometry.h>
#include <dlib/image_io.h>
#include <dlib/image_transforms.h>
#include <dlib/matrix.h>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>

#include <time.h>

#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;


inline uint64_t get_hr_time() {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return
    static_cast<uint64_t>(t.tv_sec) * 1e9 +
    static_cast<uint64_t>(t.tv_nsec);
}


namespace argagg {
namespace convert {
  template <>
  std::vector<double> arg(const char* s)
  {
    std::vector<double> ret {};
    if (std::strlen(s) == 0) {
      return ret;
    }
    while (true) {
      const char* token = std::strchr(s, ',');
      if (token == nullptr) {
        ret.emplace_back(std::atof(s));
        break;
      }
      std::size_t len = token - s;
      std::string str(s, len);
      ret.emplace_back(std::stof(str));
      s += len + 1;
    }
    return ret;
  }
} // namespace convert
} // namespace argagg


int main(
  int argc,
  const char** argv)
try {

  argagg::parser argparser {{
      {
        "help", {"-h", "--help"},
        "show this help message", 0},
      {
        "k", {"-k"},
        "comma separated list of k values", 1},
      {
        "minsize", {"-m", "--min-size"},
        "minimum size of proposal (default: 20)", 1},
      {
        "viz", {"--viz"},
        "visualization output path", 1},
      {
        "video", {"--video"},
        "treat the input as a video", 0},
      {
        "videostart", {"-s", "--start"},
        "start frame when processing a video (default: 0)", 1},
      {
        "videoend", {"-e", "--end"},
        "end frame when processing a video (default: -1, end of video)", 1},
      {
        "vizfps", {"-f", "--viz-fps"},
        "if visualization and video is enabled then this option specifics "
        "the output frame rate of the visualization video (default: 12)", 1},
      {
        "vizcodec", {"-c", "--viz-codec"},
        "if visualization and video is enabled then this option specifies "
        "the output codec of the visualization video using a FOURCC code "
        "(default: same codec as input video)", 1},
    }};

  // Generate usage text.
  ostringstream usage;
  usage
    << "Usage: " << argv[0] << " INPUT" << endl
    << endl
    << argparser;

  // Parse command line arguments.
  argagg::parser_results args;
  try {
    args = argparser.parse(argc, argv); 
  } catch (const std::exception& e) {
    cerr << usage.str() << "Encountered exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }
  
  // Print out the help message if specified.
  if (args["help"]) {
    cerr << usage.str();
    return EXIT_SUCCESS;
  }

  // Make sure an input argument has been specified.
  if (args.pos.size() != 1) {
    cerr
      << usage.str() << endl
      << "Missing input file." << endl;
    return EXIT_FAILURE;
  }

  // Locally prepare processing parameters.
  dlib::matrix<double> k_values = dlib::linspace(50, 200, 3);
  if (args["k"]) {
    k_values = dlib::mat(args["k"].as<vector<double>>());
  }
  const unsigned long min_size = args["minsize"].as<long>(20);

  // Define single image processing function.
  auto find_object_proposals = [&](const cv::Mat& cv_img) {
      auto dlib_img = dlib::cv_image<dlib::bgr_pixel>(cv_img);
      vector<dlib::rectangle> rects;
      dlib::find_candidate_object_locations(
        dlib_img, rects, k_values, min_size);
      return rects;
    };

  // Define visualization function.
  auto visualize_object_proposals = [](
    const cv::Mat& cv_img,
    const vector<dlib::rectangle>& rects) {
      auto dlib_img = dlib::cv_image<dlib::bgr_pixel>(cv_img);
      for (auto& rect : rects) {
        int i = std::rand();
        dlib::rgb_pixel* p = reinterpret_cast<dlib::rgb_pixel*>(&i);
        dlib::draw_rectangle(dlib_img, rect, *p);
      }
    };

  const string input_path(args.pos[0]);

  // Handle processing a video.
  if (args["video"]) {

    // Open the input as a video stream.
    cv::VideoCapture video_in(input_path);

    // Initialize the output video if configured to output a visualization.
    cv::VideoWriter video_out;
    if (args["viz"]) {
      auto output_name = args["viz"].as<string>();
      auto codec = static_cast<int>(video_in.get(CV_CAP_PROP_FOURCC));
      if (args["vizcodec"]) {
        string codec_name = args["vizcodec"];
        if (codec_name.length() == 4) {
          codec = CV_FOURCC(
            codec_name[0], codec_name[1], codec_name[2], codec_name[3]);
        } else {
          cerr
            << "Malformed FOURCC codec code '" << codec_name
            << "', defaulting to input video codec." << endl;
        }
      }
      auto fps = args["vizfps"].as<double>();
      auto size = cv::Size(
        video_in.get(CV_CAP_PROP_FRAME_WIDTH),
        video_in.get(CV_CAP_PROP_FRAME_HEIGHT));
      auto is_color = true;
      video_out.open(output_name, codec, fps, size, is_color);
      if (!video_out.isOpened()) {
        throw std::runtime_error(
          "could not open output video for visualization");
      }
    }

    // Define variables for per-frame video processing loop.
    cv::Mat img;
    bool visualize = args["viz"];
    auto frame_index = args["videostart"].as<int>(0);
    const int end_frame = args["videoend"].as<int>(-1);
    if (frame_index > 0) {
      video_in.set(CV_CAP_PROP_POS_FRAMES, static_cast<double>(frame_index));
    }

    // Process frames in the video.
    while (video_in.read(img) && frame_index <= end_frame) {
      uint64_t last_time = get_hr_time();
      auto rects = find_object_proposals(img);
      uint64_t new_time = get_hr_time();
      double elapsed_sec = static_cast<double>(new_time - last_time) / 1.0e9;
      cout
        << "frame " << frame_index << ": " << rects.size()
        << " proposals (" << elapsed_sec << " sec)" << endl;
      if (visualize) {
        if (!video_out.isOpened()) {
          cerr << "Problem with visualization output video, disabling "
            "visualization output" << endl;
          visualize = false;
        } else {
          visualize_object_proposals(img, rects);
          video_out.write(img);
        }
      }
      ++frame_index;
    }

  // Handle processing a single image.
  } else {
    auto img = cv::imread(input_path);
    auto rects = find_object_proposals(img);
    for (auto& rect : rects) {
      cout << rect << endl;
    }
    if (args["viz"]) {
      visualize_object_proposals(img, rects);
      cv::imwrite(args["viz"].as<string>(), img);
    }
  }

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  cerr << "Encountered unhandled exception: " << e.what() << endl;
  throw e;
}
