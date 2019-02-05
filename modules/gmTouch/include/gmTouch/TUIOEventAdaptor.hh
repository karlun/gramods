
#ifndef GRAMODS_TOUCH_TUIOEVENTADAPTOR
#define GRAMODS_TOUCH_TUIOEVENTADAPTOR

#include <gmTouch/config.hh>

#ifdef gramods_ENABLE_TUIO

#include <gmTouch/TouchState.hh>

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
   * touchState.getEventAdaptor<touchlib::TUIOEventAdaptor>().open();
   * ...
   * int width, height;
   * SDL_GetWindowSize(sdl_window, &width, &height);
   * 
   * touchState.eventsInit(width, height);
   * touchState.eventsDone();
   * \endcode
   */
  class TUIOEventAdaptor
    : public TouchState::EventAdaptor,
      public TUIO::TuioListener {
    
  public:

    /**
     * Connects the TUIO client with TCP to the specified address
     * (localhost if not specified) and port (3333 if not
     * specified).
     */
    void connect(std::string address = "localhost", int port = 3333);

    /**
     * Opens the TUIO client to listen to UDP packages on the
     * specified port (3333 if not specified).
     */
    void open(int port = 3333);

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
