
#ifndef GRAMODS_NETWORK_CONNECTION
#define GRAMODS_NETWORK_CONNECTION

#include <gmNetwork/config.hh>

#include <gmCore/Object.hh>
#include <chrono>
#include <vector>

BEGIN_NAMESPACE_GMNETWORK;

/**
   The base of connections.

   @startuml
   object Client
   object VariableA
   object VariableB
   object VariableC
   object StateSyncProtocol
   object BarrierProtocol
   object Connection

   Client --> VariableA
   Client --> VariableB
   Client --> VariableC
   Client --> BarrierProtocol

   StateSyncProtocol --> VariableA
   StateSyncProtocol --> VariableB
   StateSyncProtocol --> VariableC

   BarrierProtocol <--> Connection
   StateSyncProtocol <--> Connection
   @enduml
*/
class Connection
  : public gmCore::Object {

public:

  enum struct Status {
    DISCONNECTED,
    DYSFUNCTIONAL,
    PARTIALLY_CONNECTED,
    FULLY_CONNECTED
  };
  
  typedef std::chrono::steady_clock clock;

  virtual bool send(std::vector<char> data) = 0;
  virtual bool receive(std::vector<char> &data) = 0;

  virtual Status getStatus() = 0;
};

END_NAMESPACE_GMNETWORK;

#endif
