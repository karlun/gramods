
#ifndef GRAMODS_TRACK_STDKEY
#define GRAMODS_TRACK_STDKEY

#include <gmTrack/config.hh>

#include <string>

BEGIN_NAMESPACE_GMTRACK;

/**
   Standard keys for trackers.
*/
struct StdKey {

public:

  /// trigger
  static const std::string TRIGGER;

  /// joy
  static const std::string JOY;

  /// head_pose
  static const std::string HEAD_POSE;
  /// primary_wand_pose
  static const std::string PRIMARY_WAND;
  /// secondary_wand_pose
  static const std::string SECONDARY_WAND;

  /// main_button
  static const std::string MAIN_BUTTON;
  /// second_button
  static const std::string SECOND_BUTTON;
  /// menu_button
  static const std::string MENU_BUTTON;
};

END_NAMESPACE_GMTRACK;

#endif
