#ifndef HUDMANAGER__H
#define HUDMANAGER__H
#include <SDL2/SDL.h>
#include <string>
#include <map>
#include "hudElement.h"
#include "player.h"
#include "levelManager.h"

class HUDManager {
public:
  explicit HUDManager();
  static HUDManager& getInstance();
  void draw() const;
  void initialize();

  static bool condition(Player* p){ return p->getTotalEnergies() >= 0; }
  static bool playerDied(Player* p);
  static bool levelComplete();

  const std::map<std::string, HUDElement*>& getHudElements() const {return hudElements;}
  bool getDisplay() const { return display; }
  void toggleDisplay() { display = !display; }
  void addElement(HUDElement* e) { hudElements.insert(std::make_pair(e->getName(), e)); }
  HUDElement* getElement(const std::string n){ return hudElements[n]; }

  const HUDManager& operator=(const HUDManager& h) = delete;
private:
  std::map<std::string, HUDElement*> hudElements;
  bool display;  // should you draw the HUDElement
};

#endif
