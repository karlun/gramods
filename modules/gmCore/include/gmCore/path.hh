
#ifndef GRAMODS_CORE_PATH
#define GRAMODS_CORE_PATH

#include <gmCore/config.hh>
#include <filesystem>
#include <iostream>

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator reading into a path. This is typically used to read
   XML file or folder attributes. The only difference from reading as
   a simple stream is that you may specify environment variables or
   URN base path to automatically find installed resources.

    - "urn:gramods:gmGraphics/resources/fulldome180-coordinates.png"
    - "${GM_HOME}/gramods/gmTrack/config/aruco-complex-test.xml"
*/
std::istream& operator>> (std::istream &in, std::filesystem::path &path);

END_NAMESPACE_GRAMODS;

#endif
