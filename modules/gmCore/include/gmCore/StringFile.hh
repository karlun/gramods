
/**
   (c)2023 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_CORE_STRINGFILE
#define GRAMODS_CORE_STRINGFILE

#include <gmCore/config.hh>
#include <gmCore/PreConditionViolation.hh>

#include <optional>

#include <stdio.h>
#include <string.h>

BEGIN_NAMESPACE_GMCORE

/**
   This struct creates a FILE pointer to a temporary file and reads
   off this file's data into string when finalized. This is useful in
   communication with pure C libraries.
*/
struct StringFile {

  /**
     Closes the temporary file if this has not already been done.
  */
  ~StringFile() {
    if (file_ptr) fclose(*file_ptr);
  }

  /**
     Returns the FILE pointer to a temporary file after, if not
     already done, creating such a FILE object.
  */
  FILE * getFilePtr() {
    if (!file_ptr) file_ptr = tmpfile();
    return *file_ptr;
  }

  /**
     Rewinds and reads off the temporary file into a string, closes
     the FILE object and returns the string. This can only be called
     after getFilePtr and then only once.
  */
  std::string finalize() {
    if (!file_ptr) throw PreConditionViolation("Cannot finalize - not open");

    rewind(*file_ptr);

    char buffer [256];
    std::stringstream ss;

    while (!feof(*file_ptr)) {
      if (fgets(buffer, 256, *file_ptr) == NULL) break;
      ss << buffer;
    }

    fclose(*file_ptr);
    file_ptr = std::nullopt;

    return ss.str();
  }

private:
  std::optional<FILE*> file_ptr;
};

END_NAMESPACE_GMCORE

#endif
