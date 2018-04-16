#include "hudManager.h"


HUDManager::HUDManager() :
  hudElements(),
  display(true)
  {

}

HUDManager& HUDManager::getInstance() {
  static HUDManager m;
  return m;
}

void HUDManager::draw() const {
  for(HUDElement* e : hudElements){
    e->draw();
  }
}
