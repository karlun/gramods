
#ifndef GRAMODS_TRACK_ANALOGSMAPPER
#define GRAMODS_TRACK_ANALOGSMAPPER

#include <gmTrack/AnalogsTracker.hh>

#include <gmCore/io_size.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Analogs filter that re-maps analog indices to a standard order for
   application compatibility between tracker systems. Preferred order
   is vertical, horizontal and then trigger, followed by any other
   analog input.
*/
class AnalogsMapper
  : public AnalogsTracker {

public:

  /**
     Standard analogs indices, for compatibility. Vertical and
     horizontal may be a joystick, trackpad or touchpad. Positive
     values should follow right handed convension - positive right for
     horizontal and positive up/forward for vertical.
  */
  struct AnalogIndex {
    static const size_t VERTICAL   = 0;
    static const size_t HORIZONTAL = 1;
    static const size_t TRIGGER    = 2;
    static const size_t COUNT      = 3;
  };

  /**
     Sets the analogs tracker to re-map.
     
     \gmXmlTag{gmTrack,AnalogsMapper,analogsTracker}
  */
  void setAnalogsTracker(std::shared_ptr<AnalogsTracker> bt);

  /**
     Adds a mapping in the form of two integer indices: from and to
     button index, respectively.
     
     \gmXmlTag{gmTrack,AnalogsMapper,mapping}
  */
  void addMapping(gmCore::size2 m);

  /**
     Sets which analog to map to vertical analog, as an index starting
     at zero.
     
     \gmXmlTag{gmTrack,AnalogsMapper,verticalAnalog}
  */
  void setVerticalAnalog(int idx);

  /**
     Sets which analog to map to horizontal analog, as an index
     starting at zero.
     
     \gmXmlTag{gmTrack,AnalogsMapper,horizontalAnalog}
  */
  void setHorizontalAnalog(int idx);

  /**
     Sets which analog to map to trigger analog, as an index starting
     at zero.
     
     \gmXmlTag{gmTrack,AnalogsMapper,triggerAnalog}
  */
  void setTriggerAnalog(int idx);

  /**
     Replaces the contents of p with button data. Returns true if data
     could be read, false otherwise. Use sample time to check if data
     are fresh.
  */
  bool getAnalogs(AnalogsSample &p) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  std::map<size_t, size_t> mappings;
  std::shared_ptr<AnalogsTracker> analogsTracker;
};

END_NAMESPACE_GMTRACK;

#endif
