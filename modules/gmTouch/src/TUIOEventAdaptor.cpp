
#include <gmTouch/TUIOEventAdaptor.hh>

#ifdef gramods_ENABLE_TUIO

#include <TuioListener.h>
#include <TuioClient.h>

#include <UdpReceiver.h>
#include <TcpReceiver.h>

using namespace touchlib;

struct TUIOEventAdaptor::Impl : public TUIO::TuioListener {

  void connect(std::string address, int port);
  void open(int port);

  /**
   * Called by Tuio to add a cursor object.
   */
  void addTuioCursor(TUIO::TuioCursor *tcur);

  /**
   * Called by Tuio to update a cursor object.
   */
  void updateTuioCursor(TUIO::TuioCursor *tcur);

  /**
   * Called by Tuio to remove a cursor object.
   */
  void removeTuioCursor(TUIO::TuioCursor *tcur);

  void addTuioObject(TUIO::TuioObject *tobj) {}
  void updateTuioObject(TUIO::TuioObject *tobj) {}
  void removeTuioObject(TUIO::TuioObject *tobj) {}
  void addTuioBlob(TUIO::TuioBlob *tblb) {}
  void updateTuioBlob(TUIO::TuioBlob *tblb) {}
  void removeTuioBlob(TUIO::TuioBlob *tblb) {}

  void refresh(TUIO::TuioTime frameTime) {}

  int width, height;
  bool mouse_down = false;

  std::unique_ptr<TUIO::OscReceiver> osc_receiver;
  std::unique_ptr<TUIO::TuioClient> tuio_client;

  std::mutex cursor_data_lock;

  struct Cursor {
    TouchState::TouchPointId id;
    float x;
    float y;
    bool add;
    double time;
  };

  std::vector<Cursor> cursor_data;
};

TUIOEventAdaptor::TUIOEventAdaptor()
  : _impl(std::make_unique<TUIOEventAdaptor>()) {}

void TUIOEventAdaptor::connect(std::string address, int port) {
  _impl->connect(address, port);
}

void TUIOEventAdaptor::Impl::connect(std::string address, int port) {
  if (tuio_client)
    tuio_client->disconnect();
  
  if (address == "incoming")
    osc_receiver.reset(new TUIO::TcpReceiver(port));
  else
    osc_receiver.reset(new TUIO::TcpReceiver(address.c_str(), port));
  tuio_client.reset(new TUIO::TuioClient(osc_receiver.get()));

  tuio_client->addTuioListener(this);
  tuio_client->connect();
}

void TUIOEventAdaptor::open(int port) {
  _impl->open(port);
}

void TUIOEventAdaptor::Impl::open(int port) {
  if (tuio_client)
    tuio_client->disconnect();

  osc_receiver.reset(new TUIO::UdpReceiver(port));
  tuio_client.reset(new TUIO::TuioClient(osc_receiver.get()));

  tuio_client->addTuioListener(this);
  tuio_client->connect();
}


void TUIOEventAdaptor::init(int width, int height) {
  _impl->init(width, height);
}

void TUIOEventAdaptor::Impl::init(int width, int height) {
  this->width = width;
  this->height = height;
}

void TUIOEventAdaptor::Impl::addTuioCursor(TUIO::TuioCursor *tcur) {
  std::unique_lock<std::mutex> lck(cursor_data_lock);
  double seconds =
    std::chrono::duration_cast<std::chrono::duration<double>>
    (TouchState::clock::now().time_since_epoch()).count();
  cursor_data.push_back(Cursor{
      tcur->getCursorID(),
        tcur->getX(),
        tcur->getY(),
        true,
        seconds
        });
}

void TUIOEventAdaptor::Impl::updateTuioCursor(TUIO::TuioCursor *tcur) {
  std::unique_lock<std::mutex> lck(cursor_data_lock);
  double seconds =
    std::chrono::duration_cast<std::chrono::duration<double>>
    (TouchState::clock::now().time_since_epoch()).count();
  cursor_data.push_back(Cursor {
      tcur->getCursorID(),
        tcur->getX(),
        tcur->getY(),
        true,
        seconds
        });
}

void TUIOEventAdaptor::Impl::removeTuioCursor(TUIO::TuioCursor *tcur) {
  std::unique_lock<std::mutex> lck(cursor_data_lock);
  cursor_data.push_back(Cursor {
      tcur->getCursorID(),
        tcur->getX(),
        tcur->getY(),
        false,
        0.0
        });
}

void TUIOEventAdaptor::done() {
  std::unique_lock<std::mutex> lck(_impl->cursor_data_lock);

  for (auto item : _impl->cursor_data)
    if (item.add)
      addTouchState(
          item.id, item.x * _impl->width, item.y * _impl->height, item.time);
    else
      removeTouchState(item.id, item.x * _impl->width, item.y * _impl->height);

  _impl->cursor_data.clear();
}

#endif
