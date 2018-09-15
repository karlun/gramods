
#ifndef GRAMODS_CORE_OSTREAMMESSAGESINK
#define GRAMODS_CORE_OSTREAMMESSAGESINK

#include <gmCore/MessageSink.hh>

#include <gmCore/OFactory.hh>

#include <mutex>

BEGIN_NAMESPACE_GMCORE;

/**
*/
class OStreamMessageSink
  : public MessageSink {

public:

  OStreamMessageSink();

  /**
     Set the stream to use as output. Use this if you do not want
     smart memory management, for example if your output stream is
     std::cout.
  */
  void setStream(std::ostream *s) {
    std::lock_guard<std::mutex> guard(lock);
    rp_out = s;
    sp_out = nullptr;
  }

  /**
     Set the stream to use as output. Use this if you do want smart
     memory management, for example if the output stream is shared
     between objects and is not memory managed by the operating
     system.
  */
  void setStream(std::shared_ptr<std::ostream> s) {
    std::lock_guard<std::mutex> guard(lock);
    rp_out = nullptr;
    sp_out = s;
  }

  /**
     Set the stream to use as output, as string. Valid values are
     "out" and "err".
  */
  void setStream(std::string name);

  void output(Message msg);

  GM_OFI_DECLARE;

private:

  void outputLevelAndTag(Message msg);

  std::ostream *rp_out;
  std::shared_ptr<std::ostream> sp_out;
  std::mutex lock;
};

END_NAMESPACE_GMCORE;

#endif
