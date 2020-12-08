
#ifndef GRAMODS_CORE_NULLMESSAGESINK
#define GRAMODS_CORE_NULLMESSAGESINK

#include <gmCore/MessageSink.hh>

BEGIN_NAMESPACE_GMCORE;

/**
   Message sink that does nothing with the messages.

   Since Console writes to a default message sink if the configuration
   or client code has not added its own message sink, this can be used
   to suppress all type of output.
*/
class NullMessageSink
  : public MessageSink {

public:

  void output(Message) {}

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMCORE;

#endif
