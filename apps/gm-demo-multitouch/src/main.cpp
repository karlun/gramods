
#include "SDLWindow.hh"

// Undo SDL redef
#ifdef main
#undef main
#endif

int main(int, char **) {

  gramods::SDLWindow window;

  while (!window.isDone()) {
    window.process();
  }

  return 0;
}
