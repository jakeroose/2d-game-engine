#include "hudManager.h"


HUDManager::HUDManager() :
  hudElements(),
  display(true)
  {
}

bool HUDManager::playerDied(Player* p){
  return p->isDead();
}

bool HUDManager::levelComplete(){
  return LevelManager::getInstance().getGoalReached();
}


HUDManager& HUDManager::getInstance() {
  static HUDManager m;
  return m;
}

void HUDManager::draw() const {
  if(display){
    for(auto e : hudElements){
      e.second->draw();
    }
  }
}

void HUDManager::initialize(){
}
