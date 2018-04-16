#ifndef HUDMANAGER__H
#define HUDMANAGER__H
#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "hudElement.h"
#include "player.h"
#include "levelManager.h"

class HUDManager {
public:
  explicit HUDManager();
  static HUDManager& getInstance();
  void draw() const;
  void initialize(Player* p);

  static bool condition(){ return HUDManager::getInstance().getDisplay(); }
  static bool playerDied(Player* p);
  static bool levelComplete();
  static bool gamePaused();
  static bool onlyOnLevel(const std::string n);
  static bool playerInArea(Player* p, const Vector2f& v1, const Vector2f& v2);
  static bool playerHasCollectable(Player* p);

  static bool conditionArray(std::vector<std::function<bool ()> > arr);

  const std::map<std::string, HUDElement*>& getHudElements() const {
    return hudElements; }
  bool getDisplay() const { return display; }
  void toggleDisplay() { display = !display; }
  void addElement(HUDElement* e) {
    hudElements.insert(std::make_pair(e->getName(), e)); }
  HUDElement* getElement(const std::string n){ return hudElements[n]; }

  const HUDManager& operator=(const HUDManager& h) = delete;
private:
  std::map<std::string, HUDElement*> hudElements;
  bool display;  // should you draw the HUDElement
};
#endif
