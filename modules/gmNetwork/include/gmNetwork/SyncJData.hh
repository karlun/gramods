
#ifndef GRAMODS_NETWORK_SYNCJDATA
#define GRAMODS_NETWORK_SYNCJDATA

#include <gmNetwork/SyncData.hh>

#ifdef gramods_ENABLE_nlohmann_json

#include <gmCore/InvalidArgument.hh>

#include <nlohmann/json.hpp>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Simple, json object synchronizeable data container.
*/
template<class TYPE>
class SyncJData
  : public SyncData {

public:

  /**
     Initializes without specifying value.
  */
  SyncJData() { back = front = {}; }

  /**
     Initializes the SyncJData to the specified value.
  */
  SyncJData(TYPE val) { back = front = val; }

  SyncJData(SyncJData<TYPE> &&s) {
    back = s.back;
    front = s.front;
  }

  /**
     Retrieves the front value of the container.
  */
  operator TYPE() const {
    TYPE retval;
    {
      std::lock_guard<std::mutex> guard(const_cast<SyncJData<TYPE>*>(this)->lock);
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
      std::lock_guard<std::mutex> guard(const_cast<SyncJData<TYPE>*>(this)->lock);
      retval = front;
    }
    return retval;
  }

  /**
     Sets the back value of the container and immediately sends it to
     the back value of connected peers.
  */
  SyncJData<TYPE>& operator= (TYPE val) {
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

    nlohmann::json j;
    to_json(j, back);
    const auto m = nlohmann::json::to_msgpack(j);

    d.reserve(m.size() + 1);
    d.push_back(0);
    std::copy(m.begin(), m.end(), std::back_inserter(d));
  }

  /**
     Decodes the specified vector into the back data of the container,
     using vector indices 1-N.
  */
  void decode(std::vector<char> d) override {
    std::lock_guard<std::mutex> guard(lock);

    const auto j = nlohmann::json::from_msgpack(d.begin() + 1, d.end());
    back = j.get<TYPE>();
    from_json(j, back);
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

END_NAMESPACE_GMNETWORK;

#endif
#endif
