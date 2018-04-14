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

/*
LevelManager should let me easily add walls to a level using arbitrary in-game
coordinates instead of specific pixel coordinates.
It should keep track of all of the walls in the game so that classes such as
Light and Player don't have to have their own copies.
Reads in level from /levels at runtime
*/

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
  spawnPoint(),
  loadingType(),
  state(LevelState::loading),
  cursorCoords(),
  anchor(NULL) {
  loadLevel("levels/" + Gamedata::getInstance().getXmlStr("level/name"));
}

LevelManager::~LevelManager(){
  for(auto w: walls) delete w.second;
  for(auto w: freeWalls) delete w;
  for(Collectable* c: collectables) delete c;
  for(Collectable* c: freeCollectables) delete c;
  for(SmartSprite* e: enemies) delete e;
  for(SmartSprite* e: freeEnemies) delete e;
}

LevelManager& LevelManager::getInstance(){
  static LevelManager levelManager;
  return levelManager;
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
  // std::cout << "addWall" << std::endl;
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
    } else {
      std::cout << "Cannot load type \""<< l << "\"" << std::endl;
    }
  } else if(regex_match(l, n)){
    if(loadingType == NONE){
      std::cout << "uhh something's wrong with the level file.." << std::endl;
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
    }
  }
}

void LevelManager::loadLevel(const std::string& name){
  std::cout << "Loading Level: " << name << std::endl;
  state = LevelState::loading;
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

  std::ifstream levelData;
  std::string line;
  levelData.open(name);
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
      // customLevel << "Test\n";
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
      if(e[0] == v[0]){
        toDelete = w.first;
        break;
      }
    }
  }
  wallVertices.erase(wallVertices.find(toDelete));
  auto it = walls.find(toDelete);
  freeWalls.push_back((*it).second);
  walls.erase(it);
}
