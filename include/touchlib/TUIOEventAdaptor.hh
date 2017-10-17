
#ifndef __TOUCHLIB_TUIOEVENTADAPTOR_HH__
#define __TOUCHLIB_TUIOEVENTADAPTOR_HH__

#include <touchlib/config.hh>

#ifdef TOUCHLIB_ENABLE_TUIO

#include <touchlib/TouchState.hh>

#include <TuioListener.h>
#include <TuioClient.h>

#include <memory>
#include <mutex>

namespace touchlib {
  
  /**
   * Event adaptor for TUIO, providing means to input events into
   * TouchState.
   * 
   * Typical use:
   * \code{.cpp}
   * int width, height;
   * SDL_GetWindowSize(sdl_window, &width, &height);
   * 
   * touchState.eventsInit(width, height);
   * touchState.getEventAdaptor<touchlib::TUIOEventAdaptor>().sync();
   * touchState.eventsDone();
   * \endcode
   */
  class TUIOEventAdaptor
    : public TouchState::EventAdaptor,
      public TUIO::TuioListener {
    
  public:

    /**
     * Connects the TUIO client with TCP to the specified address and
     * port.
     */
    void connect(std::string address, int port);

    /**
     * Opens the TUIO client to listen to UDP packages on the
     * specified port.
     */
    void open(int port);

    /**
     * Called by the owner (TouchState) when eventsInit is called.
     */
    void init(int width, int height);
    
    /**
     * Synchronizes touch events from Tuio to the touch state. Called
     * by the owner (TouchState) when eventsDone is called.
     */
    void done();

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

  private:

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
}

#endif
#endif
