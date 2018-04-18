/**
   Copyright 2018, Karljohan Lundin Palmerius
   Copyright 2011, GManNickG @ stackoverflow.com
   
*/

#ifndef GRAMODS_CORE_SCOPEDOSTREAMREDIRECT
#define GRAMODS_CORE_SCOPEDOSTREAMREDIRECT

#include <gmCore/config.hh>

BEGIN_NAMESPACE_GMCORE;

/**
   Utility for temporary redirection of an ostream, for example to
   temporarily change the target of std::cout.
*/
class ScopedOStreamRedirect {

public:

  /**
     Sets up an object that, until destroyed, redirects the first
     argument to the target of the second argument.
  */
  ScopedOStreamRedirect(std::ostream & inOriginal, std::ostream & inRedirect) :
    mOriginal(inOriginal),
    mOldBuffer(inOriginal.rdbuf(inRedirect.rdbuf())) {}

  /**
     Sets back the original target of the redirected ostream.
  */
  ~ScopedOStreamRedirect() {
    mOriginal.rdbuf(mOldBuffer);
  }    

  ScopedOStreamRedirect(const ScopedOStreamRedirect&) = delete;

  ScopedOStreamRedirect& operator=(const ScopedOStreamRedirect&) = delete;

private:

  std::ostream & mOriginal;
  std::streambuf * mOldBuffer;
};

END_NAMESPACE_GMCORE;

#endif
