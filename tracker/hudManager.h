#ifndef HUDMANAGER__H
#define HUDMANAGER__H
#include <SDL2/SDL.h>
#include <string>
#include "hudElement.h"
#include "player.h"

class HUDManager {
public:
  explicit HUDManager();
  static HUDManager& getInstance();
  void draw() const;

  static bool condition(Player* p){ return p->getTotalEnergies() >= 0; }

  const std::vector<HUDElement*>& getHudElements() const {return hudElements;}
  bool getDisplay() const { return display; }
  void toggleDisplay() { display = !display; }

  const HUDManager& operator=(const HUDManager& h) = delete;
private:
  std::vector<HUDElement*> hudElements;
  bool display;  // should you draw the HUDElement
};

#endif
