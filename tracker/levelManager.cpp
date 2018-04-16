#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include "levelManager.h"
#include "light.h"
#include "vector2f.h"
#include "wall.h"
#include "smartSprite.h"
#include "viewport.h"

/*
LevelManager should let me easily add walls to a level using arbitrary in-game
coordinates instead of specific pixel coordinates.
It should keep track of all of the walls in the game so that classes such as
Light and Player don't have to have their own copies.
Reads in level from /levels at runtime
*/

// TODO: Move level editing to its own class

int LevelManager::UNIT_SIZE = Gamedata::getInstance().getXmlInt("world/unitSize");

std::vector<int> strToIntVec(std::string& s){
  std::stringstream ss(s);
  std::string coord;
  std::vector<int> v;
  while(std::getline(ss, coord, ' ')){
    v.push_back(std::stoi(coord));
  }
  return v;
}

LevelManager::LevelManager() :
  walls(),
  freeWalls(),
  wallVertices(),
  collectables(),
  freeCollectables(),
  enemies(),
  freeEnemies(),
  goal(new Sprite("Square1")),
  strategy(new RectangularCollisionStrategy()),
  spawnPoint(),
  loadingType(),
  state(LevelState::loading),
  cursorCoords(),
  anchor(NULL),
  goalReached(false),
  levelName("levels/" + Gamedata::getInstance().getXmlStr("level/name")) {
  loadLevel(levelName);
}

LevelManager::~LevelManager(){
  for(auto w: walls) delete w.second;
  for(auto w: freeWalls) delete w;
  for(Collectable* c: collectables) delete c;
  for(Collectable* c: freeCollectables) delete c;
  for(SmartSprite* e: enemies) delete e;
  for(SmartSprite* e: freeEnemies) delete e;
  delete goal;
  delete strategy;
}

LevelManager& LevelManager::getInstance(){
  static LevelManager levelManager;
  return levelManager;
}

void LevelManager::togglePause(){
  if(state == LevelState::running){
    state = LevelState::paused;
  } else if(state == LevelState::paused){
    state = LevelState::running;
  }
}

void LevelManager::addWall(Wall* w){
  walls.emplace(w->getId(), w);
  wallVertices.emplace(w->getId(), w->getVertices());
}

void LevelManager::addWall(const std::string& s){
  std::stringstream ss(s);
  std::string coord;
  std::vector<int> v;
  while(std::getline(ss, coord, ' ')){
    v.push_back(std::stoi(coord));
  }

  // wall collision doesn't work properly if coords are big->small
  // may be an issue with collision detection but fixing it here for now.
  int tmp;
  if(v[0] == v[2]){
    if(v[1] > v[3]){
      tmp = v[1];
      v[1] = v[3];
      v[3] = tmp;
    }
  }
  else {
    if(v[0] > v[2]){
      tmp = v[0];
      v[0] = v[2];
      v[2] = tmp;
    }
  }
  if(freeWalls.size() > 0){
    Wall* w = freeWalls.back();
    freeWalls.pop_back();
    w->setTo(v[0], v[1], v[2], v[3]);
    addWall(w);
  } else {
    addWall(new Wall(v[0], v[1], v[2], v[3]));
  }
}

/* Adds a collectable to the tile at x, y */
void LevelManager::addCollectable(int x, int y){
  int scaledX =  UNIT_SIZE/2, scaledY = UNIT_SIZE/2;

  Collectable* c;
  if(freeCollectables.size() > 0){
    c = freeCollectables.back();
    freeCollectables.pop_back();
    c->reinitialize();
  } else {
    c = new Collectable("Collectable");
  }

  c->setPosition(Vector2f(scaledX + UNIT_SIZE*x, scaledY + UNIT_SIZE*y));
  collectables.push_back(c);
}

// Remove collectable and place it in freeCollectables object pool
void LevelManager::removeCollectable(Collectable* c){
  std::vector<Collectable*>::const_iterator it;
  it = std::find_if(collectables.begin(), collectables.end(),
    [&c](Collectable* c1){return c1 == c; });

  if(it != collectables.end()){
    freeCollectables.push_back(*it);
    collectables.erase(it);
  }
}

void LevelManager::addEnemy(int x, int y){
  int scaledX =  UNIT_SIZE/2, scaledY = UNIT_SIZE/2;

  SmartSprite* e;
  if(freeEnemies.size() > 0){
    e = freeEnemies.back();
    freeEnemies.pop_back();
    e->reset();
  } else {
    e = new SmartSprite("Enemy");
  }

  e->setPosition(Vector2f(scaledX + UNIT_SIZE*x - e->getScaledWidth()/2,
    scaledY + UNIT_SIZE*y - e->getScaledHeight()/2));
  enemies.push_back(e);
}

void LevelManager::removeEnemy(SmartSprite* s){
  auto it = std::find_if(enemies.begin(), enemies.end(),
    [&s](SmartSprite* e){return e == s;});
  if(it != enemies.end()){
    freeEnemies.push_back(*it);
    enemies.erase(it);
  }
}

void LevelManager::setSpawnPoint(Vector2f v){
  int scaledX =  UNIT_SIZE/2, scaledY = UNIT_SIZE/2;
  spawnPoint = Vector2f(scaledX + UNIT_SIZE*v[0], scaledY + UNIT_SIZE*v[1]);
}

void LevelManager::parseLine(std::string& l){
  std::regex r("([a-zA-z]+)(\\:)");
  std::regex n("[(\\d+)(\\s)*)]+");
  if(regex_match(l, r)){
    std::regex p("(player\\:)");
    std::regex e("(enemies\\:)");
    std::regex c("(collectables\\:)");
    std::regex w("(walls\\:)");
    std::regex s("(end\\:)");

    if(regex_match(l, p)){
      std::cout << "Loading Player" << std::endl;
      loadingType = PLAYER;
    } else if(regex_match(l, e)){
      std::cout << "Loading Enemies" << std::endl;
      loadingType = ENEMY;
    } else if(regex_match(l, c)){
      std::cout << "Loading Collectables" << std::endl;
      loadingType = COLLECTABLE;
    } else if(regex_match(l, w)){
      std::cout << "Loading Walls" << std::endl;
      loadingType = WALL;
    } else if(regex_match(l, s)){
      std::cout << "Loading Goal" << std::endl;
      loadingType = GOAL;
    } else {
      std::cout << "Cannot load type \""<< l << "\"" << std::endl;
      loadingType = NONE;
    }
  } else if(regex_match(l, n)){
    if(loadingType == NONE){
      std::cout << "Skipping line: " << l << std::endl;
      return;
    }
    std::vector<int> v = strToIntVec(l);
    if(loadingType == PLAYER){
      setSpawnPoint(Vector2f(v[0], v[1]));
    } else if(loadingType == ENEMY){
      addEnemy(v[0], v[1]);
    } else if(loadingType == COLLECTABLE){
      addCollectable(v[0], v[1]);
    } else if(loadingType == WALL){
      addWall(l);
    } else if(loadingType == GOAL){
      goal->setPosition(Vector2f(v[0], v[1])*(UNIT_SIZE));
    }
  }
}

void LevelManager::resetLevel(){
  loadLevel(levelName);
}

void LevelManager::loadLevel(const std::string& name){
  std::cout << "Loading Level: " << name << std::endl;
  state = LevelState::loading;
  levelName = name;
  // std::cout << "=== Before Load ===" << std::endl;
  // std::cout << "Collectables: " << collectables.size() << std::endl;
  // std::cout << "freeCollectables: " << freeCollectables.size() << std::endl;
  // std::cout << "Walls: " << walls.size() << std::endl;
  // std::cout << "freeWalls: " << freeWalls.size() << std::endl;
  wallVertices.erase(wallVertices.begin(), wallVertices.end());
  if(walls.size() > 0){
    auto it = walls.begin();
    while(it != walls.end()){
      freeWalls.push_back((*it).second);
      it = walls.erase(it);
    }
  }

  if(collectables.size() > 0){
    auto it = collectables.begin();
    while(it != collectables.end()){
      freeCollectables.push_back(*it);
      it = collectables.erase(it);
    }
  }
  auto ite = enemies.begin();
  while(ite != enemies.end()){
    freeEnemies.push_back(*ite);
    ite = enemies.erase(ite);
  }

  goal->setPosition(Vector2f(0, 0));
  goal->setScale(UNIT_SIZE/goal->getImage()->getWidth());
  goalReached = false;

  std::ifstream levelData;
  std::string line;
  levelData.open(levelName);
  if(levelData.is_open()){
    while(std::getline(levelData, line)){
      parseLine(line);
    }
    levelData.close();
  } else {
    std::cout << "Couldn't find level" << std::endl;
    throw 0;
  }

  // border of the world
  SDL_Rect worldBorder[] = {
    { 0, 0, Gamedata::getInstance().getXmlInt("world/width"), // top
      1 },
    { Gamedata::getInstance().getXmlInt("world/width"), 0, 1, // right
      Gamedata::getInstance().getXmlInt("world/height") },
    { 0, Gamedata::getInstance().getXmlInt("world/height"),
      Gamedata::getInstance().getXmlInt("world/width"), 1 }, // bottom
    { 0, 0, 1,
      Gamedata::getInstance().getXmlInt("world/height") },
  };

  for(SDL_Rect r: worldBorder){
    addWall(r);
  }
  // std::cout << "=== After Load ===" << std::endl;
  // std::cout << "Collectables: " << collectables.size() << std::endl;
  // std::cout << "freeCollectables: " << freeCollectables.size() << std::endl;
  // std::cout << "Walls: " << walls.size() << std::endl;
  // std::cout << "freeWalls: " << freeWalls.size() << std::endl;
  state = LevelState::running;
}


int LevelManager::getTotalLightIntersections() const {
  int count = 0;
  for(Collectable* c: collectables) count +=
    c->getLight()->getPolygonSize();
  return count;
}

int LevelManager::getTotalFreeIntersections() const {
  int count = 0;
  for(Collectable* c: collectables) count +=
    c->getLight()->getIntersectionPoolSize();
  return count;
}

void LevelManager::saveLevel() const{
  if(state == LevelState::editing){
    std::ofstream customLevel;
    customLevel.open("levels/customLevel");
    if(customLevel.is_open()){
      customLevel << "player:\n1 1\n";
      customLevel << "walls:\n";
      for(auto w: walls){
        customLevel << w.second->getSmallCoordString() << std::endl;
      }
      customLevel.close();
      std::cout << "Changes saved to levels/customLevel" << std::endl;
    } else {
      std::cout << "Error opening file 'levels/customLevel'" << std::endl;
    }
  } else {
    std::cout << "Couldn't save level, not in edit mode." << std::endl;
  }
}

void LevelManager::toggleLevelEdit(){
  if(state == LevelState::loading){
    std::cout << "Unable to edit level while loading." << std::endl;
  } else if (state == LevelState::editing) {
    std::cout << "=== Edit level disabled ===" << std::endl;
    state = LevelState::running;
  } else if (state == LevelState::running){
    state = LevelState::editing;
    std::cout << "=== Edit level enabled ===" << std::endl;
    std::cout << "Press f9 to save any changes." << std::endl;
  }
}

// return true if p is within x units of the viewport
bool LevelManager::withinRenderDistance(const Vector2f& p) const {
  int x = Viewport::getInstance().getX() +
          Viewport::getInstance().getViewWidth()/2;
  int y = Viewport::getInstance().getY() +
          Viewport::getInstance().getViewHeight()/2;

  return withinRenderDistance(p, Vector2f(x,y));
}

// return true if p is within x units of the viewport window centered on anchor
bool LevelManager::withinRenderDistance(const Vector2f& p, const Vector2f& anchor) const {
  int rad = 2*UNIT_SIZE;

  float viewWidth = Viewport::getInstance().getViewWidth();
  float viewHeight = Viewport::getInstance().getViewHeight();

  float _nx = anchor[0] - viewWidth/2 - rad;
  float _ny = anchor[1] - viewHeight/2 - rad;
  float _mx = viewWidth + _nx + 2*rad;
  float _my = viewHeight + _ny + 2*rad;

  if(p[0] < _nx || p[0] > _mx || p[1] < _ny || p[1] > _my){
    return false;
  }

  return true;
}

// creates a border around the viewport for calculating light
void LevelManager::updateViewBorder(){
  int rad = 20;
  wallVertices["view_border"] = {
    Vector2f(
      Viewport::getInstance().getX()+rad,
      Viewport::getInstance().getY()+rad),
    Vector2f(
      Viewport::getInstance().getX()+Viewport::getInstance().getViewWidth()-rad,
      Viewport::getInstance().getY()+rad),
    Vector2f(
      Viewport::getInstance().getX()+Viewport::getInstance().getViewWidth()-rad,
      Viewport::getInstance().getY()+Viewport::getInstance().getViewHeight()-rad),
    Vector2f(
      Viewport::getInstance().getX()+rad,
      Viewport::getInstance().getY()+Viewport::getInstance().getViewHeight()-rad)
  };
}

// set cursor to nearest whole number coord
void LevelManager::setCursor(const Vector2f& v) {
  cursorCoords = Vector2f(
    (int)(v[0] + UNIT_SIZE/2)/UNIT_SIZE,
    (int)(v[1] + UNIT_SIZE/2)/UNIT_SIZE);
}

void LevelManager::setAnchor(){
  if(anchor != NULL){
    addWall(
      (int)((*anchor)[0] + UNIT_SIZE/2)/UNIT_SIZE,
      (int)((*anchor)[1] + UNIT_SIZE/2)/UNIT_SIZE,
      cursorCoords[0],
      cursorCoords[1]);
  }
  delete anchor;
  anchor = new Vector2f(cursorCoords*UNIT_SIZE);
}

void LevelManager::resetAnchor(){
  if(anchor == NULL){
    eraseWall();
  } else {
    delete anchor;
    anchor = NULL;
  }
}

// deletes wall under the cursor.
// NOTE: must be on the vertex of a wall
void LevelManager::eraseWall(){
  Vector2f v = cursorCoords*UNIT_SIZE;
  std::string toDelete;
  for(auto w : wallVertices){
    for(auto e : w.second){
      if(e[0] == v[0] && e[1] == v[1]){
        toDelete = w.first;
        break;
      }
    }
  }
  if(toDelete.length()){
    wallVertices.erase(wallVertices.find(toDelete));
    auto it = walls.find(toDelete);
    if(it != walls.end()){
      std::cout << "deleting wall: " << toDelete << std::endl;
      freeWalls.push_back((*it).second);
      walls.erase(it);
    }

  }
}
