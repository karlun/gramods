
#ifndef GRAMODS_NETWORK_SYNCSDATA
#define GRAMODS_NETWORK_SYNCSDATA

#include <gmNetwork/SyncData.hh>
#include <gmCore/InvalidArgument.hh>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Simple, single value synchronizeable data container, without
   support for pointers or types containing pointers.
*/
template<class TYPE>
class SyncSData
  : public SyncData {

public:

  /**
     Initializes without specifying value.
  */
  SyncSData() { back = front = {}; }

  /**
     Initializes the SyncSData to the specified value.
  */
  SyncSData(TYPE val) { back = front = val; }

  SyncSData(SyncSData<TYPE> &&s) {
    back = s.back;
    front = s.front;
  }

  /**
     Retrieves the front value of the container.
  */
  operator TYPE () const {
    TYPE retval;
    {
      std::lock_guard<std::mutex> guard(const_cast<SyncSData<TYPE>*>(this)->lock);
      retval = front;
    }
    return retval;
  }

  /**
     Retrieves the front value of the container.
  */
  TYPE operator*() const {
    TYPE retval;
    {
      std::lock_guard<std::mutex> guard(const_cast<SyncSData<TYPE>*>(this)->lock);
      retval = front;
    }
    return retval;
  }

  /**
     Sets the back value of the container and immediately sends it to
     the back value of connected peers.
  */
  SyncSData<TYPE>& operator= (TYPE val) {
    {
      std::lock_guard<std::mutex> guard(lock);
      back = val;
    }
    SyncData::pushValue();
    return *this;
  }

protected:

  /**
     Encodes the back data of the container into the specified vector,
     using vector indices 1-N and leaving the zeroth cell empty.
  */
  void encode(std::vector<char> &d) override {
    std::lock_guard<std::mutex> guard(lock);

    d.resize(1 + sizeof(TYPE));

    char * m = reinterpret_cast<char*>(&back);

    for (size_t idx = 0; idx < sizeof(TYPE); ++idx) {
      d[idx + 1] = m[idx];
    }
  }

  /**
     Decodes the specified vector into the back data of the container,
     using vector indices 1-N.
  */
  void decode(std::vector<char> d) override {
    std::lock_guard<std::mutex> guard(lock);

    if (d.size() != 1 + sizeof(TYPE))
      throw gmCore::InvalidArgument("incorrect data size for decoding");

    TYPE value;
    char * m = reinterpret_cast<char*>(&value);

    for (size_t idx = 0; idx < sizeof(TYPE); ++idx)
      m[idx] = d[idx + 1];

    back = value;
  }

  /**
     Copies the back value to the front.
  */
  void update() override {
    std::lock_guard<std::mutex> guard(lock);
    front = back;
  }

private:

  /**
     Front value, the visible container value.
  */
  TYPE front;

  /**
     Background value, set, sent and retrieved over network.
  */
  TYPE back;

  std::mutex lock;

};

typedef SyncSData<bool> SyncSBool;
typedef SyncSData<int32_t> SyncSInt32;
typedef SyncSData<uint32_t> SyncSUInt32;
typedef SyncSData<int64_t> SyncSInt64;
typedef SyncSData<uint64_t> SyncSUInt64;
typedef SyncSData<float> SyncSFloat32;
typedef SyncSData<double> SyncSFloat64;

END_NAMESPACE_GMNETWORK;

#endif
