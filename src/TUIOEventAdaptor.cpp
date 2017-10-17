
#include <touchlib/TUIOEventAdaptor.hh>

#ifdef TOUCHLIB_ENABLE_TUIO

#include <UdpReceiver.h>
#include <TcpReceiver.h>

using namespace touchlib;

void TUIOEventAdaptor::connect(std::string address, int port) {
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
  if (tuio_client)
    tuio_client->disconnect();

  osc_receiver.reset(new TUIO::UdpReceiver(port));
  tuio_client.reset(new TUIO::TuioClient(osc_receiver.get()));

  tuio_client->addTuioListener(this);
  tuio_client->connect();
}


void TUIOEventAdaptor::init(int width, int height) {
  this->width = width;
  this->height = height;
}

void TUIOEventAdaptor::addTuioCursor(TUIO::TuioCursor *tcur) {
  std::unique_lock<std::mutex> lck(cursor_data_lock);
  double seconds = TouchState::clock::now().time_since_epoch().count();
  std::cerr << seconds << std::endl;
  cursor_data.push_back(Cursor{
      tcur->getCursorID(),
        tcur->getX(),
        tcur->getY(),
        true,
        seconds
        });
}

void TUIOEventAdaptor::updateTuioCursor(TUIO::TuioCursor *tcur) {
  std::unique_lock<std::mutex> lck(cursor_data_lock);
  double seconds = TouchState::clock::now().time_since_epoch().count();
  std::cerr << seconds << std::endl;
  cursor_data.push_back(Cursor {
      tcur->getCursorID(),
        tcur->getX(),
        tcur->getY(),
        true,
        seconds
        });
}

void TUIOEventAdaptor::removeTuioCursor(TUIO::TuioCursor *tcur) {
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
  std::unique_lock<std::mutex> lck(cursor_data_lock);

  for (auto item : cursor_data)
    if (item.add)
      addTouchState(item.id, item.x, item.y, item.time);
    else
      removeTouchState(item.id, item.x, item.y);

  cursor_data.clear();
}

#endif
