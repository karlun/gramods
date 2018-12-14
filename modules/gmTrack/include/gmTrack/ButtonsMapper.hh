
#ifndef GRAMODS_TRACK_BUTTONSMAPPER
#define GRAMODS_TRACK_BUTTONSMAPPER

#include <gmTrack/ButtonsTracker.hh>

#include <gmTypes/all.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Buttons filter that re-maps button indices to a standard order for
   application compatibility between tracker systems. Preferred order
   is main button, second button and then menu button, followed by any
   other button, starting at the least significant bit.
*/
class ButtonsMapper
  : public ButtonsTracker {

public:

  /**
     Standard button indices, for compatibility.
  */
  struct ButtonMask {
    static const unsigned int MAIN      = 1;
    static const unsigned int SECONDARY = 1 << 1;
    static const unsigned int MENU      = 1 << 2;
  };

  /**
     Sets the buttons tracker to re-map.
  */
  void setButtonsTracker(std::shared_ptr<ButtonsTracker> bt);

  /**
     Adds a mapping in the form of two integer indices: from and to
     button index, respectively.
  */
  void addMapping(gmTypes::size2 m);

  /**
     Replaces the contents of p with button data.
  */
  bool getButtons(ButtonsSample &p);

  GM_OFI_DECLARE;

private:

  std::map<size_t, size_t> mappings;
  std::shared_ptr<ButtonsTracker> buttonsTracker;
};

END_NAMESPACE_GMTRACK;

#endif
