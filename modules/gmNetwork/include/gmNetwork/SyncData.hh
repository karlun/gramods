
#ifndef GRAMODS_NETWORK_SYNCDATA
#define GRAMODS_NETWORK_SYNCDATA

#include <gmNetwork/config.hh>

#include <vector>
#include <memory>

BEGIN_NAMESPACE_GMNETWORK;

class DataSync;

/**
   Simple synchronizeable data container.
*/
class SyncData {

public:

  SyncData();

protected:

  /**
     Called by sub classes when the value has been set, to push the
     value to peers.
  */
  void pushValue();

  virtual void encode(std::vector<char> &d) = 0;
  virtual void decode(std::vector<char> d) = 0;

  virtual void update() = 0;

private:

  void setSynchronizer(std::shared_ptr<DataSync> sync);

  friend DataSync;

private:

  struct Impl;
  std::shared_ptr<Impl> _impl;

};

END_NAMESPACE_GMNETWORK;

#endif
