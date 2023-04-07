
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>

#include <gmCore/OpenVR.hh>

#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>

using namespace gramods;

int main(int argc, char *argv[]) {

  std::shared_ptr<gmCore::OpenVR> openvr = std::make_shared<gmCore::OpenVR>();
  openvr->setManifestPath("urn:gramods:config/openvr_manifest/standard_actionset.json");
  openvr->setActionSet("/actions/std");
  openvr->initialize();

  std::this_thread::sleep_for(std::chrono::seconds(2));

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

  return 0;
}
