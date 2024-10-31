

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>
#include <gmCore/TimeTools.hh>

#include <gmCore/io_eigen.hh>
#include <gmTrack/SampleCollector.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <sstream>
#include <fstream>

using namespace gramods;

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a tracker pivot point estimator for gramods. A configuration file or string must be specified and this must contain the definition of a controller (gmTrack::Controller).");

  TCLAP::MultiArg<std::string> arg_config_dummy
    ("", "config",
     "Configuration file(s) load.",
     false, "file");
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration(s) to load.",
     false, "string");
  cmd.xorAdd(arg_config_dummy, arg_xml_dummy);

  TCLAP::MultiArg<std::string> arg_param_dummy
    ("", "param",
     "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
     false, "identifier=value");
  cmd.add(arg_param_dummy);

  TCLAP::ValueArg<size_t> arg_count(
      "n", "count",
      "Number of samples to wait for before estimating the pivot"
      " point. Default is the minimum, 3.",
      false, 3, "N");
  cmd.add(arg_count);

  TCLAP::ValueArg<float> arg_pos_inlier(
      "", "position-inlier-distance",
      "Set the maximum positional distance from the average allowed for a"
      " sample to be included in the average. Default is to include all"
      " samples.",
      false, -1, "D");
  cmd.add(arg_pos_inlier);

  TCLAP::ValueArg<float> arg_ori_inlier(
      "", "orientation-inlier-distance",
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

  if (arg_config_dummy.getValue().empty() && arg_xml_dummy.getValue().empty())
    return 2;

  if (arg_count.getValue() < 3) {
    std::cerr << "Incorrect number of samples to collect: "
              << arg_count.getValue() << " < 3\n";
    return 3;
  }

  std::unique_ptr<gmCore::Configuration> config;
  try {
    config = std::make_unique<gmCore::Configuration>(argc, argv);
  } catch (const gmCore::RuntimeException &e) {
    std::cerr << "Error: Configuration error: " << e.what << std::endl;
  } catch (...) {
    std::cerr
        << "Error: Unknown internal error while creating Configuration instance."
        << std::endl;
  }

  if (!config) return 2;

  std::shared_ptr<gmTrack::Controller> controller;
  if (!config->getObject(controller)) {
    std::cerr << "Error: No controller found." << std::endl;
    return 3;
  }

  std::shared_ptr<gmTrack::SampleCollector> collector =
      std::make_shared<gmTrack::SampleCollector>();
  collector->setController(controller);
  collector->setInlierThreshold(arg_pos_inlier.getValue());
  collector->setOrientationInlierThreshold(arg_ori_inlier.getValue());

  size_t n_collected = 0;
  while (collector->getTrackerPositions().size() < arg_count.getValue())
    gmCore::Updateable::updateAll();

  auto & positions = collector->getTrackerPositions();
  auto & orientations = collector->getTrackerOrientations();

  std::vector<Eigen::Matrix4f> transforms;
  transforms.reserve(positions.size());

  for (size_t idx = 0; idx < positions.size(); ++idx) {
    Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
    M.topLeftCorner<3, 3>() = Eigen::Matrix3f(orientations[idx]);
    M.topRightCorner<3, 1>() = positions[idx];
    transforms.push_back(M);
  }

  // We assume that all transforms have a common pivot point
  //
  // Ax - Bx = 0
  //   =>
  // (A - B)x = 0
  //
  // Put all transform differences into a single, big matrix and solve
  // in homogeneous coords to get the pivot point

  std::vector<Eigen::Matrix4f> diffs;
  diffs.reserve(transforms.size() * transforms.size() / 2);

  for (size_t idxA = 0; idxA < positions.size(); ++idxA)
    for (size_t idxB = idxA + 1; idxB < positions.size(); ++idxB)
      diffs.push_back(transforms[idxA] - transforms[idxB]);

  Eigen::MatrixXf D(3 * diffs.size() + 1, 4);
  for (size_t idx = 0; idx < diffs.size(); ++idx)
    D.block<3, 4>(3 * idx, 0) = diffs[idx].topRows(3);
  D.bottomRows(1) = Eigen::Vector3f::Zero().homogeneous().transpose();
  Eigen::VectorXf zero = Eigen::VectorXf::Zero(3 * diffs.size() + 1);
  zero.bottomRows(1)[0] = 1.f;

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(D, Eigen::ComputeThinU | Eigen::ComputeThinV);
  Eigen::Vector4f x = svd.solve(zero);
  x /= x(3, 0);

  std::cout << "\nEstimated relative pivot point: " << x.transpose() << std::endl;

  std::vector<Eigen::Vector3f> rel_pts;
  rel_pts.reserve(transforms.size());

  for(const auto transform : transforms)
    rel_pts.push_back((transform * x).hnormalized());

  float stddev, maxdev;
  size_t inlier_count;
  Eigen::Vector3f pt = gmTrack::SampleCollector::getAverage(
      rel_pts, &stddev, &maxdev, arg_pos_inlier.getValue(), &inlier_count);

  std::cout << "Estimated absolute pivot point: " << pt.transpose()
            << " (stddev: " << stddev << ", maxdev: " << maxdev << ")"
            << " using " << inlier_count << " of " << rel_pts.size()
            << " samples" << std::endl;

  return 0;
}
