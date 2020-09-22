
#ifndef GRAMODS_TRACK_BUTTONSMAPPER
#define GRAMODS_TRACK_BUTTONSMAPPER

#include <gmTrack/ButtonsTracker.hh>

#include <gmTypes/size.hh>

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
     Standard button index, in the ButtonsSample::buttons map, for
     compatibility.
  */
  struct ButtonIdx {
    static const size_t MAIN;      //< 0
    static const size_t SECONDARY; //< 1
    static const size_t MENU;      //< 2
  };

  /**
     Sets the buttons tracker to re-map.

     \b XML-attribute: \c buttonsTracker
  */
  void setButtonsTracker(std::shared_ptr<ButtonsTracker> bt);

  /**
     Adds a mapping in the form of two integer indices: from and to
     button index, respectively.

     \b XML-attribute: \c mapping
  */
  void addMapping(gmTypes::size2 m);

  /**
     Sets which button to map to main button, as an index starting at
     zero.

     \b XML-attribute: \c mainButton
  */
  void setMainButton(int idx);

  /**
     Sets which button to map to secondary button, as an index
     starting at zero.

     \b XML-attribute: \c secondaryButton
  */
  void setSecondaryButton(int idx);

  /**
     Sets which button to map to menu button, as an index starting at
     zero.

     \b XML-attribute: \c menuButton
  */
  void setMenuButton(int idx);

  /**
     Replaces the contents of p with button data. Returns true if data
     could be read, false otherwise. Use sample time to check if data
     are fresh.
  */
  bool getButtons(ButtonsSample &p);

  GM_OFI_DECLARE;

private:

  std::map<size_t, size_t> mappings;
  std::shared_ptr<ButtonsTracker> buttonsTracker;
};

END_NAMESPACE_GMTRACK;

#endif
