
#ifndef GRAMODS_NETWORK_SYNCMDATA
#define GRAMODS_NETWORK_SYNCMDATA

#include <gmNetwork/SyncData.hh>
#include <gmCore/InvalidArgument.hh>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Simple, multi value (vector) synchronizeable data container,
   without support for pointers or types containing pointers.

   Observe that this container is not optimized for synchronization of
   large amounts of data.
*/
template<class TYPE>
class SyncMData
  : public SyncData {

public:

  /**
     Initializes the SyncMData to an empty list.
  */
  SyncMData() {}

  /**
     Initializes the SyncMData to the specified value.
  */
  SyncMData(std::vector<TYPE> val) { back = front = val; }

  SyncMData(SyncMData<TYPE> &&s) {
    back = s.back;
    front = s.front;
  }

  /**
     Retrieves the front vector of the container.
  */
  operator std::vector<TYPE> () const {
    std::vector<TYPE> retval;
    {
      std::lock_guard<std::mutex> guard(const_cast<SyncMData<TYPE>*>(this)->lock);
      retval = front;
    }
    return retval;
  }

  /**
     Retrieves the indexed values from the front vector of the container.
  */
  TYPE operator[] (size_t idx) const {
    TYPE retval;
    {
      std::lock_guard<std::mutex> guard(const_cast<SyncMData<TYPE>*>(this)->lock);
      retval = front[idx];
    }
    return retval;
  }

  /**
     Sets the back value of the container and immediately sends it to the
     back value of connected peers.
  */
  SyncMData<TYPE>& operator= (std::vector<TYPE> val) {
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

    d.resize(1 + back.size() * sizeof(TYPE));

    char * m = reinterpret_cast<char*>(&back[0]);

    for (size_t idx = 0; idx < back.size() * sizeof(TYPE); ++idx) {
      d[idx + 1] = m[idx];
    }
  }

  /**
     Decodes the specified vector into the back data of the container,
     using vector indices 1-N.
  */
  void decode(std::vector<char> d) override {
    std::lock_guard<std::mutex> guard(lock);

    if ((d.size() - 1)%sizeof(TYPE) != 0)
      throw gmCore::InvalidArgument("incorrect data size for decoding");

    size_t vector_size = (d.size() - 1)/sizeof(TYPE);

    std::vector<TYPE> value;
    value.resize(vector_size);
    char * m = reinterpret_cast<char*>(&value[0]);

    for (size_t idx = 0; idx < vector_size * sizeof(TYPE); ++idx)
      m[idx] = d[idx + 1];

    back = std::move(value);
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
     Front data vector, the visible container data.
  */
  std::vector<TYPE> front;

  /**
     Background data vector, set, sent and retrieved over network.
  */
  std::vector<TYPE> back;

  std::mutex lock;

};

typedef SyncMData<char> SyncMBool;
typedef SyncMData<int32_t> SyncMInt32;
typedef SyncMData<int64_t> SyncMInt64;
typedef SyncMData<float> SyncMFloat32;
typedef SyncMData<double> SyncMFloat64;

END_NAMESPACE_GMNETWORK;

#endif
