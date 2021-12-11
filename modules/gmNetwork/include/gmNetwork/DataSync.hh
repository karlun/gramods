
#ifndef GRAMODS_NETWORK_DATASYNC
#define GRAMODS_NETWORK_DATASYNC

#include <gmNetwork/Protocol.hh>
#include <gmNetwork/SyncData.hh>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Synchronization of simple data, such as boolean, float or integer
   values, over network. OBSERVE: The data sharing currently assumes,
   but does not test, that the peers have the same endianness.

   The data synchronization applies these principles:

   1.  The data containers are double buffered - data are set to the
   back buffer and read off the front buffer.

   2.  Data are sent over network to all peers as soon as they are set
   and it is up to the client code to make sure that the peers do not
   overwrite each other's data.

   3.  It is up to the client code to call update() when it is done
   writing new data and want to use these data, and it is also up to
   the client code to make sure that all peers are in the same stage
   of execution so that this update leads to consistent behaviour.

   ## Typical usage

   Global variables:
   @code{.cpp}
   std::shared_ptr<gmNetwork::SyncNode> sync_node;
   std::shared_ptr<gmNetwork::SyncSFloat> shared_time;
   std::shared_ptr<gmNetwork::SyncSBool> shared_button;
   @endcode

   Initialize:
   @code{.cpp}
   shared_time = std::make_shared<gmNetwork::SyncSFloat>();
   shared_button = std::make_shared<gmNetwork::SyncSBool>();

   gmCore::Configuration config(argc, argv);

   if (!config.getObject(sync_node))
     exit(1);

   gmNetwork::DataSync * data_sync 
       = sync_node->getProtocol<gmNetwork::DataSync>();
   data_sync->addData(shared_time);
   data_sync->addData(shared_button);
   @endcode

   Use:
   @code{.cpp}
   // Set the back buffer and send data
   *shared_time = 14.32;
   *shared_button = false;

   sync_node->getProtocol<gmNetwork::RunSync>()
       ->waitForAll();
   sync_node->getProtocol<gmNetwork::DataSync>()
       ->update(); // <- moves received data to the front

   // Read off front buffer
   std::cerr << *shared_time << std::endl;
   @endcode
*/
class DataSync
  : public Protocol {

public:

  DataSync();
  virtual ~DataSync();

  /**
     Adds a data container to be synchronized by the
     instance. Subsequently setting the value of the specified
     container will send that value to the connected peers, and a
     received value will be set. Call update to make received values
     current.

     Observe that all connected peers must have the exact same
     associated data containers.
  */
  void addData(std::shared_ptr<SyncData> d);

  /**
     Adds a data container to be synchronized by the
     instance. Subsequently setting the value of the specified
     container will send that value to the connected peers, and a
     received value will be set. Call update to make received values
     current.

     Observe that all connected peers must have the exact same
     associated data containers.

     WARNING: the caller MUST make sure that the pointer is valid
     during the lifetime of this object.
  */
  void addData(SyncData * d);

  /**
     Exchanges old data with newly received data in all associated
     data containers.
  */
  void update();

  /**
     Receives data values and sets the associated container.
  */
  void processMessage(Message m) override;

  /**
     Synchronizes the specified data object. This method is
     automatically called when the value of a SyncData is set and
     should not be used by client code.
  */
  void send(SyncData * d);

  /**
     Returns the header byte associated with the protocol, sent in the
     header of messages to indicate which protocol instance to call
     for interpretation and processing.
  */
  char getProtocolFlag() override { return 11; }

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMNETWORK;

#endif
