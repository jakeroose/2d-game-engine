#include "hudManager.h"
#include "viewport.h"
#include <fstream>
#include <sstream>

HUDManager::HUDManager() :
  hudElements(),
  display(true)
  {
}
HUDManager::~HUDManager(){
  for(auto e : hudElements) delete e.second;
}

bool HUDManager::playerDied(Player* p){
  return p->isDead() && !gamePaused();
}

bool HUDManager::levelComplete(){
  return LevelManager::getInstance().getGoalReached() && !gamePaused();
}
bool HUDManager::onlyOnLevel(const std::string n){
  return n.compare(LevelManager::getInstance().getLevelName()) == 0;
}

bool HUDManager::playerInArea(Player* p, const Vector2f& v1, const Vector2f& v2){
  Vector2f v = p->getPosition();
  return (v[0] >= v1[0] && v[1] >= v1[1] && v[0] <= v2[0] && v[1] <= v2[1]);
}

bool HUDManager::playerHasCollectable(Player* p){
  return p->getTotalEnergies() > 0;
}

bool HUDManager::gamePaused(){
  return LevelManager::getInstance().isPaused();
}

bool HUDManager::conditionArray(std::vector<std::function<bool ()> > arr){
  for(auto e : arr) if(e() == false) return false;
  return true;
}

HUDManager& HUDManager::getInstance() {
  static HUDManager m;
  return m;
}

void HUDManager::draw() const {
  for(auto e : hudElements){
    e.second->draw();
  }
}

/* Initialize all of the different menus & popups */
void HUDManager::initialize(Player* player){
  HUDElement* tmp = new HUDElement("hud", 15, 15);
  tmp->setCondition(std::bind(HUDManager::condition));
  addElement(tmp);

  tmp = new HUDElement("playerDied",
  Viewport::getInstance().getViewWidth()/2 - 50,
  Viewport::getInstance().getViewHeight()/2 - 30);
  tmp->setCondition(std::bind(HUDManager::playerDied, player));
  addElement(tmp);

  tmp = new HUDElement("levelComplete",
  Viewport::getInstance().getViewWidth()/2 - 50,
  Viewport::getInstance().getViewHeight()/2 - 30);
  tmp->setCondition(std::bind(HUDManager::levelComplete));
  addElement(tmp);

  tmp = new HUDElement("paused",
  Viewport::getInstance().getViewWidth()/2 - 50,
  Viewport::getInstance().getViewHeight()/2 - 30);
  tmp->setCondition(std::bind(HUDManager::gamePaused));
  addElement(tmp);

  /* === Tutorial Level Tips === */
  // I hope I can understand this 2 days from now...
  std::vector<std::function<bool ()> > conds = {
    std::bind(HUDManager::playerInArea, player,
    LevelManager::getInstance().gameToWorldCoord(Vector2f(8, 7)),
    LevelManager::getInstance().gameToWorldCoord(Vector2f(14, 11))),
    std::bind(HUDManager::onlyOnLevel, "levels/tutorial")
  };
  tmp = new HUDElement("stuck");
  tmp->setCondition( std::bind( HUDManager::conditionArray, conds));
  tmp->setPosition(LevelManager::getInstance().gameToWorldCoord(
        Vector2f(10, 7)));
  tmp->setLocationWorld();
  addElement(tmp);

  conds = {
    std::bind(HUDManager::playerHasCollectable, player),
    std::bind(HUDManager::onlyOnLevel, "levels/tutorial")
  };
  tmp = new HUDElement("collectableInfo");
  tmp->setCondition(std::bind(HUDManager::conditionArray, conds));
  tmp->setPosition(LevelManager::getInstance().gameToWorldCoord(
        Vector2f(9, 2.9)));
  tmp->setLocationWorld();
  addElement(tmp);

  conds = {
    std::bind(HUDManager::playerInArea, player,
    LevelManager::getInstance().gameToWorldCoord(Vector2f(5, 3)),
    LevelManager::getInstance().gameToWorldCoord(Vector2f(9, 4))),
    std::bind(HUDManager::onlyOnLevel, "levels/tutorial")
  };
  tmp = new HUDElement("tutInstructions");
  tmp->setCondition(std::bind(HUDManager::conditionArray, conds));
  tmp->setPosition(LevelManager::getInstance().gameToWorldCoord(
        Vector2f(5, 4.1)));
  tmp->setLocationWorld();
  addElement(tmp);

  std::ifstream hudContent;
  std::string line;
  for(auto e :hudElements){
    hudContent.open("strings/" + e.first);
    if(hudContent.is_open()){
      while(std::getline(hudContent, line)){
        e.second->addLine(line);
      }
      hudContent.close();
    } else {
      std::cout << "Unable to locate file: strings/" << e.first << std::endl;
      throw 0;
    }
  }
}
