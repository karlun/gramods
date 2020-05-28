

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Updateable.hh>

#include <gmTypes/eigen.hh>
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
     true, "file");
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration(s) to load.",
     true, "string");
  cmd.xorAdd(arg_config_dummy, arg_xml_dummy);

  TCLAP::MultiArg<std::string> arg_param_dummy
    ("", "param",
     "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
     false, "identifier=value");
  cmd.add(arg_param_dummy);

  TCLAP::MultiArg<std::string> arg_point
    ("p", "point",
     "Specification of point in the registered coordinate system.",
     true, "x,y,z");
  cmd.add(arg_point);

  TCLAP::ValueArg<std::string> arg_outputfile
    ("o", "output-file",
     "The file to write the results to.",
     false, "output.xml", "file");
  cmd.add(arg_outputfile);

  TCLAP::ValueArg<std::string> arg_outputtemplate
    ("t", "output-template",
     "A template file that is filled with the resulting values and written to the specified output file. In the template file the following keys are replaced with their corresponding values:\n"
     "%M0x..%M3z   the corresponding cell of the resulting 4x4 matrix.\n"
     "%Rx..%Ra     the rotation part expressed as axis angle rotation with %Ra expressed in radians and %Rd in degrees.\n"
     "%Qx..%Qw     the rotation part expressed in quaternion format.\n"
     "%M, %R and %Q can also be replaced with %Mi, %Ri and %Qi, respectively, for use of the inverse transform instead of the forward registration transform.",
     false, "", "file");
  cmd.add(arg_outputtemplate);

  TCLAP::SwitchArg arg_normalize
    ("n", "normalize",
     "Remove scaling from the final result. This can be used if input and output data are defined in the same units.",
     cmd, false);

  TCLAP::SwitchArg arg_test_output
    ("", "test-output",
     "Do not perform registration, but write unit transform to the output file using the specified template.",
     cmd, false);

  try {
    cmd.parse(argc, argv);
  } catch (TCLAP::ArgException &e) {
    std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  std::unique_ptr<gmCore::Configuration> config;
  try {
    config = std::make_unique<gmCore::Configuration>(argc, argv);
  }
  catch (const std::exception &ex) {
    std::cerr << "Error: Configuration error: " << ex.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Error: Unknown internal error while creating Configuration instance." << std::endl;
  }

  if (!config) {
    std::cerr << std::endl
              << "Error: No valid configuration available." << std::endl
              << std::endl;
    return 2;
  }

  std::shared_ptr<gmTrack::Controller> controller;
  if (!config->getObject(controller)) {
    std::cerr << std::endl
              << "Error: No controller available in the specified configuration." << std::endl
              << std::endl;
    return 3;
  }

  std::shared_ptr<gmTrack::PoseRegistrationEstimator> registrator =
    std::make_shared<gmTrack::PoseRegistrationEstimator>();
  registrator->setController(controller);

  if (! arg_point.getValue().empty()) {

    auto str_points = arg_point.getValue();

    for (auto str_point : str_points) {
      std::replace(str_point.begin(), str_point.end(), ',', ' ');
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        registrator->addPoint(p);
      else
        std::cerr << "Warning: could not parse '" << str_point << "' as a 3D point." << std::endl;
    }

  } else if (config->hasParam("point")) {

    std::vector<std::string> str_points;
    std::size_t count = config->getAllParams("point", str_points);

    for (auto str_point : str_points) {
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        registrator->addPoint(p);
      else
        std::cerr << "Warning: could not parse '" << str_point << "' as a 3D point." << std::endl;
    }

  } else {
    std::cerr << std::endl
              << "Error: No points available for registering coordinate systems. Add 'point' parameters to the configuration or use the 'point' command line argument." << std::endl
              << std::endl;
    return 4;
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

  std::string output_file = arg_outputfile.getValue();
  std::ofstream fout(output_file);

  if (!fout) {
    std::cerr << std::endl
              << "Error: could not write to file '" << output_file << "'" << std::endl
              << std::endl;
    return 5;
  }

  std::string input_file = arg_outputtemplate.getValue();
  if (input_file.empty()) {

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

    return process(fin, fout, Mat, Unit);
  }

  std::ifstream fin(input_file);

  if (!fin) {
    std::cerr << std::endl
              << "Error: could not open template file '" << input_file << "'" << std::endl
              << std::endl;
    return 5;
  }

  return process(fin, fout, Mat, Unit);
}
