
#include "SDLWindow.hh"


int main(int argc,char **argv) {

  SDLWindow window;

  while (!window.isDone()) {
    window.process();
  }

  return 0;
}

