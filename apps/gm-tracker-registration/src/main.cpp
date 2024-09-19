

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/RuntimeException.hh>
#include <gmCore/FileResolver.hh>

#include <gmCore/io_eigen.hh>
#include <gmTrack/Controller.hh>
#include <gmTrack/PoseRegistrationEstimator.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <sstream>
#include <fstream>

using namespace gramods;

namespace {
  int process(std::istream &fin,
              std::ostream &fout,
              Eigen::Matrix4f &Mat,
              Eigen::Matrix4f &Unit) {

    Eigen::Matrix4f Mati = Mat.inverse();

    Eigen::AngleAxisf Rot(Unit.block<3,3>(0,0));
    Eigen::Quaternionf Quat(Unit.block<3,3>(0,0));

    Eigen::Matrix4f Uniti = Unit.inverse();

    Eigen::AngleAxisf Roti(Uniti.block<3,3>(0,0));
    Eigen::Quaternionf Quati(Uniti.block<3,3>(0,0));

    char cand;
    while (fin.get(cand)) {

      if (cand != '%') {
        fout.put(cand);
        continue;
      }

      char btype;
      if (!fin.get(btype)) {
        std::cerr << std::endl
                  << "Parse error: unexpected end-of-file after '%'" << std::endl
                  << std::endl;
        return 6;
      }

      if (btype == '%') {
        fout.put('%');
        continue;
      }

      char bcol;
      if (!fin.get(bcol)) {
        std::cerr << std::endl
                  << "Parse error: unexpected end-of-file after '%'" << std::endl
                  << std::endl;
        return 6;
      }

      bool use_inverse = false;

      if (bcol == 'i') {
        use_inverse = true;
        if (!fin.get(bcol)) {
          std::cerr << std::endl
                    << "Parse error: unexpected end-of-file after '%'" << std::endl
                    << std::endl;
          return 6;
        }
      }

      switch (btype) {

      case 'M':{

        char brow;
        if (!fin.get(brow)) {
          std::cerr << std::endl
                    << "Parse error: unexpected end-of-file after '%'" << std::endl
                    << std::endl;
          return 6;
        }

        size_t col;
        switch(bcol) {
        case '0':
          col = 0;
          break;
        case '1':
          col = 1;
          break;
        case '2':
          col = 2;
          break;
        case '3':
          col = 3;
          break;
        default:
          std::cerr << std::endl
                    << "Parse error: unknown matrix vector '" << bcol << "'" << std::endl
                    << std::endl;
          return 6;
        }

        size_t row;
        switch(brow) {
        case 'x':
          row = 0;
          break;
        case 'y':
          row = 1;
          break;
        case 'z':
          row = 2;
          break;
        default:
          std::cerr << std::endl
                    << "Parse error: unknown matrix vector dimension '" << brow << "'" << std::endl
                    << std::endl;
          return 6;
        }

        fout << (use_inverse ? Mati : Mat)(row, col);

        break;
      }

      case 'R':
        switch (bcol) {
        case 'x':
          fout << (use_inverse ? Roti : Rot).axis().x();
          break;
        case 'y':
          fout << (use_inverse ? Roti : Rot).axis().y();
          break;
        case 'z':
          fout << (use_inverse ? Roti : Rot).axis().z();
          break;
        case 'a':
          fout << (use_inverse ? Roti : Rot).angle();
          break;
        case 'd':
          fout << 57.29577951308232087679815481410517033240547246656432 *
                   (use_inverse ? Roti : Rot).angle();
          break;
        default:
          std::cerr << std::endl
                    << "Parse error: unknown rotation component '" << bcol << "'" << std::endl
                    << std::endl;
          return 6;
        }
        break;

      case 'Q':
        switch (bcol) {
        case 'x':
          fout << (use_inverse ? Quati : Quat).x();
          break;
        case 'y':
          fout << (use_inverse ? Quati : Quat).y();
          break;
        case 'z':
          fout << (use_inverse ? Quati : Quat).z();
          break;
        case 'w':
          fout << (use_inverse ? Quati : Quat).w();
          break;
        default:
          std::cerr << std::endl
                    << "Parse error: unknown rotation component '" << bcol << "'" << std::endl
                    << std::endl;
          return 6;
        }
        break;

      default:
        std::cerr << std::endl
                  << "Parse error: unknown placeholder '%" << btype << "'" << std::endl
                  << std::endl;
        return 6;
      }
    }

    return 0;
  }
}

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a tracker registration estimator for gramods. A configuration file or string must be specified and this must contain the definition of a controller (gmTrack::Controller).");

  TCLAP::MultiArg<std::string> arg_config_dummy
    ("", "config",
     "Configuration file(s) load.",
     false, "file");
  cmd.add(arg_config_dummy);
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration(s) to load.",
     false, "string");
  cmd.add(arg_xml_dummy);

  TCLAP::MultiArg<std::string> arg_param_dummy
    ("", "param",
     "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
     false, "identifier=value");
  cmd.add(arg_param_dummy);

  TCLAP::MultiArg<std::string> arg_apoint
    ("p", "a-point",
     "Specification of a point in the registered coordinate system.",
     false, "x,y,z");
  cmd.add(arg_apoint);

  TCLAP::MultiArg<std::string> arg_tpoint
    ("q", "t-point",
     "Specification of a point in the tracker coordinate system.",
     false, "x,y,z");
  cmd.add(arg_tpoint);

  TCLAP::MultiArg<std::string> arg_outputfile
    ("o", "output-file",
     "The file(s) to write the results to.",
     false, "file");
  cmd.add(arg_outputfile);

  TCLAP::MultiArg<std::string> arg_outputtemplate
    ("t", "output-template",
     "Template file(s) that will be filled with the resulting values and written to the specified output file(s). If multiple templates are specified, then an equal number of output files must also be specified.\nIn the template file the following keys are replaced with their corresponding values:\n"
     "%M0x..%M3z   the corresponding cell of the resulting 4x4 matrix.\n"
     "%Rx..%Ra     the rotation part expressed as axis angle rotation with %Ra expressed in radians and %Rd in degrees.\n"
     "%Qx..%Qw     the rotation part expressed in quaternion format.\n"
     "%M, %R and %Q can also be replaced with %Mi, %Ri and %Qi, respectively, for use of the inverse transform instead of the forward registration transform.",
     false, "file");
  cmd.add(arg_outputtemplate);

  TCLAP::SwitchArg arg_normalize
    ("n", "normalize",
     "Remove scaling from the final result. This can be used if input and output data are defined in the same units.",
     cmd, false);

  TCLAP::SwitchArg arg_test_output
    ("", "test-output",
     "Do not perform registration, but write unit transform to the output file using the specified template.",
     cmd, false);

  TCLAP::ValueArg<float> arg_pos_inlier(
      "", "position-inlier-threshold",
      "Set the maximum positional distance from the average allowed for a"
      " sample to be included in the average. Default is to include all"
      " samples.",
      false, -1, "D");
  cmd.add(arg_pos_inlier);

  TCLAP::ValueArg<float> arg_ori_inlier(
      "", "orientation-inlier-threshold",
      "Set the maximum orientational distance (in radians) from the average"
      " allowed for a sample to be included in the average. Default is to"
      " include all samples.",
      false, -1, "D");
  cmd.add(arg_ori_inlier);

  try {
    cmd.parse(argc, argv);
  } catch (const TCLAP::ArgException &e) {
    std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  std::unique_ptr<gmCore::Configuration> config;
  if (!arg_config_dummy.getValue().empty() ||
      !arg_xml_dummy.getValue().empty()) {
    try {
      config = std::make_unique<gmCore::Configuration>(argc, argv);
    }
    catch (const gmCore::RuntimeException &e) {
      std::cerr << "Error: Configuration error: " << e.what << std::endl;
    }
    catch (...) {
      std::cerr << "Error: Unknown internal error while creating Configuration instance." << std::endl;
    }
  } else {
    std::shared_ptr<gmCore::OStreamMessageSink> osms =
      std::make_shared<gmCore::OStreamMessageSink>();
    osms->setUseAnsiColor(true);
    osms->setLevel(1);
    osms->initialize();
  }

  std::shared_ptr<gmTrack::PoseRegistrationEstimator> registrator =
    std::make_shared<gmTrack::PoseRegistrationEstimator>();
  registrator->setInlierThreshold(arg_pos_inlier.getValue());
  registrator->setOrientationInlierThreshold(arg_ori_inlier.getValue());

  std::shared_ptr<gmTrack::Controller> controller;
  if (config && config->getObject(controller))
    registrator->setController(controller);

  if (! arg_apoint.getValue().empty()) {

    auto str_points = arg_apoint.getValue();

    for (auto str_point : str_points) {
      std::replace(str_point.begin(), str_point.end(), ',', ' ');
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        registrator->addActualPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point << "' as a 3D point." << std::endl;
    }

  } else if (config && config->hasParam("point")) {

    std::vector<std::string> str_points;
    config->getAllParams("point", str_points);

    for (auto str_point : str_points) {
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        registrator->addActualPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point << "' as a 3D point." << std::endl;
    }

  } else if (!arg_test_output.getValue()) {
    std::cerr << std::endl
              << "Error: No points available for registering coordinate systems. Add 'point' parameters to the configuration or use the 'point' command line argument." << std::endl
              << std::endl;
    return 4;
  }

  if (! arg_tpoint.getValue().empty()) {

    auto str_points = arg_tpoint.getValue();

    for (auto str_point : str_points) {
      std::replace(str_point.begin(), str_point.end(), ',', ' ');
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        registrator->addTrackerPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point << "' as a 3D point." << std::endl;
    }

  } else if (config && config->hasParam("trackerPoint")) {

    std::vector<std::string> str_points;
    config->getAllParams("trackerPoint", str_points);

    for (auto str_point : str_points) {
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        registrator->addTrackerPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point << "' as a 3D point." << std::endl;
    }
  }

  Eigen::Matrix4f Raw = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f Unit = Eigen::Matrix4f::Identity();

  if (!arg_test_output.getValue())
    while (true) {
      gmCore::Updateable::updateAll();

      if(registrator->getRegistration(&Raw, &Unit))
        break;
    }

  Eigen::Matrix4f Mat = arg_normalize.getValue() ?
    Unit : Raw;

  auto output_files = arg_outputfile.getValue();
  auto input_files = arg_outputtemplate.getValue();

  if (output_files.empty()) {
    std::cerr << "Error: no output file specified" << std::endl;
    return 5;
  }

  if (output_files.size() < input_files.size()) {
    std::cerr << "Warning: too few output files (" << output_files.size()
              << ") compared to template files (" << input_files.size()
              << "); will ignore " << (input_files.size() - output_files.size())
              << " templates!" << std::endl
              << std::endl;
  } else if (output_files.size() > input_files.size()) {
    std::cerr << "Warning: too few template files (" << input_files.size()
              << ") compared to output files (" << output_files.size()
              << "); will use standard template!" << std::endl
              << std::endl;
  }

  for (size_t file_idx = 0; file_idx < output_files.size(); ++file_idx) {
    std::ofstream fout;
    auto output_file = output_files[file_idx];
    try {
      std::filesystem::path output_file_path =
          gmCore::FileResolver::getDefault()->resolve(
              output_file, gmCore::FileResolver::Check::WritableFile);
      fout = std::ofstream(output_file_path);
      if (!fout) {
        std::cerr << "Warning: could not open output file '" << output_file_path
                  << "' (" << output_file << ")\n\n";
        continue;
      }
    } catch (const gmCore::InvalidArgument &e) {
      std::cerr << "Exception while resolving output file: " << e.what
                << "\n\n";
    }

    if (file_idx >= input_files.size()) {
      static std::string default_template = R"lang=xml(
<data
    matrixForward="%M0x %M1x %M2x %M3x
                   %M0y %M1y %M2y %M3y
                   %M0z %M1z %M2z %M3z
                   0 0 0 1"
    matrixInverse="%Mi0x %Mi1x %Mi2x %Mi3x
                   %Mi0y %Mi1y %Mi2y %Mi3y
                   %Mi0z %Mi1z %Mi2z %Mi3z
                   0 0 0 1"
    rotationForward="%Rx %Ry %Rz %Ra"
    rotationInverse="%Rix %Riy %Riz %Ria"
    quaternionForward="%Qx %Qy %Qz %Qw"
    quaternionInverse="%Qix %Qiy %Qiz %Qiw">
</data>
)lang=xml";

      std::stringstream fin(default_template);

      int res = process(fin, fout, Mat, Unit);

      if (res) {
        std::cerr << "Could not write data" << std::endl;
        return res;
      }
      else {
        std::cout << "Results written to " << output_file << std::endl;
      }
    } else {
      std::ifstream fin;
      auto input_file = input_files[file_idx];
      try {
        std::filesystem::path input_file_path =
            gmCore::FileResolver::getDefault()->resolve(
                input_file, gmCore::FileResolver::Check::ReadableFile);
        fin = std::ifstream(input_file_path);
        if (!fin) {
          std::cerr << "Warning: could not open template file '"
                    << input_file_path << "' (" << input_file << ")\n\n";
          continue;
        }
      } catch (const gmCore::InvalidArgument &e) {
        std::cerr << "Exception while resolving template file: " << e.what
                  << "\n\n";
        continue;
      }

      int res = process(fin, fout, Mat, Unit);

      if (res) {
        std::cerr << "Could not write data" << std::endl;
        return res;
      } else {
        std::cout << "Results written to " << output_file << std::endl;
      }
    }
  }

  return 0;
}
