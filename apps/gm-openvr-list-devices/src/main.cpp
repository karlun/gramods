
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>

#include <gmCore/OpenVR.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>

using namespace gramods;

void print_openvr_data(gmCore::OpenVR *openvr);

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a simple utility for listing devices connected to OpenVR.");

  TCLAP::ValueArg<size_t> arg_loop_count(
      "l",
      "loop",
      "Query repeatedly this many times. Set to zero to loop infinitely.",
      false,
      0,
      "times");
  cmd.add(arg_loop_count);

  TCLAP::ValueArg<size_t> arg_delay(
      "d",
      "delay",
      "Delay between each query, or before the first if there is no "
      "loop. Defaults to 10 seconds",
      false,
      10,
      "secs");
  cmd.add(arg_delay);

  TCLAP::ValueArg<std::string> arg_manifest(
      "m", "manifest",
      "The manifest file to read action set data from. Defaults to"
      "'urn:gramods:config/openvr_manifest/standard_actionset.json'.",
      false,
      "urn:gramods:config/openvr_manifest/standard_actionset.json",
      "path");
  cmd.add(arg_manifest);

  TCLAP::ValueArg<std::string> arg_actionset(
      "a", "actionset",
      "The actionset to use in the specified manifest. Defaults to"
      "'/actions/std'.",
      false, "/actions/std", "id");
  cmd.add(arg_actionset);

  try {
    cmd.parse(argc, argv);
  } catch (const TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return -1;
  }

  std::shared_ptr<gmCore::OpenVR> openvr = std::make_shared<gmCore::OpenVR>();
  openvr->setManifestPath(arg_manifest.getValue());
  openvr->setActionSet(arg_actionset.getValue());
  openvr->initialize();

  if (!arg_loop_count.isSet()) {
    if (arg_delay.isSet())
      std::this_thread::sleep_for(std::chrono::seconds(arg_delay.getValue()));

    gmCore::Updateable::updateAll();
    print_openvr_data(openvr.get());

  } else {
    if (arg_loop_count.getValue() == 0) {
      while (true) {

        gmCore::Updateable::updateAll();
        print_openvr_data(openvr.get());

        if (arg_delay.isSet())
          std::this_thread::sleep_for(
              std::chrono::seconds(arg_delay.getValue()));
      }
    } else {

      for (size_t idx = 0; idx < arg_loop_count.getValue(); idx++) {

        gmCore::Updateable::updateAll();
        print_openvr_data(openvr.get());

        if (arg_delay.isSet())
          std::this_thread::sleep_for(
              std::chrono::seconds(arg_delay.getValue()));
      }
    }
  }

  return 0;
}

void print_openvr_data(gmCore::OpenVR *openvr) {
  auto pose_list = openvr->getPoseList();
  for (size_t idx = 0; idx < pose_list->size(); ++idx) {

    auto &item = (*pose_list)[idx];
    if (!item.bDeviceIsConnected) continue;

    std::cout << idx << " " << openvr->getSerial(idx) << " ";
    std::cout << gmCore::OpenVR::typeToString(openvr->getType(idx)) << " ";
    std::cout << gmCore::OpenVR::roleToString(openvr->getRole(idx)) << " ";

    if (!item.bPoseIsValid) {
      std::cout << "(no pose) ";
    } else {
      Eigen::Matrix4f M =
          gmCore::OpenVR::convert(item.mDeviceToAbsoluteTracking);
      Eigen::Quaternionf Q_rot(M.block<3, 3>(0, 0));
      Eigen::Vector3f P(M.block<3, 1>(0, 3));

      std::stringstream ss;
      ss << std::setprecision(2) << std::fixed;
      ss << "P=" << P.transpose() << " ";
      ss << "eX=" << (Q_rot * Eigen::Vector3f::UnitX()).transpose() << " ";
      ss << "eY=" << (Q_rot * Eigen::Vector3f::UnitY()).transpose() << " ";
      ss << "eZ=" << (Q_rot * Eigen::Vector3f::UnitZ()).transpose() << " ";
      std::cout << ss.str();
    }

    std::cout << "\n";
  }
  std::cout << std::endl;
}
