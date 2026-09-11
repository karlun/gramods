
#ifndef GRAMODS_TRACK_STDKEY
#define GRAMODS_TRACK_STDKEY

#include <gmTrack/config.hh>
#include <gmTrack/export.hh>

#include <string>

BEGIN_NAMESPACE_GMTRACK;

/**
   Standard keys for trackers.
*/
struct StdKey {

public:

  /// trigger
  static gmTrack_API const std::string TRIGGER;

  /// joy
  static gmTrack_API const std::string JOY;

  /// head_pose
  static gmTrack_API const std::string HEAD_POSE;
  /// primary_wand_pose
  static gmTrack_API const std::string PRIMARY_WAND;
  /// secondary_wand_pose
  static gmTrack_API const std::string SECONDARY_WAND;

  /// main_button
  static gmTrack_API const std::string MAIN_BUTTON;
  /// second_button
  static gmTrack_API const std::string SECOND_BUTTON;
  /// menu_button
  static gmTrack_API const std::string MENU_BUTTON;
};

END_NAMESPACE_GMTRACK;

#endif
