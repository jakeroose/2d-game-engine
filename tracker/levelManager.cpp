#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "levelManager.h"
#include "light.h"
#include "vector2f.h"
#include "wall.h"

/*
LevelManager should let me easily add walls to a level using arbitrary in-game
coordinates instead of specific pixel coordinates.
It should keep track of all of the walls in the game so that classes such as
Light and Player don't have to have their own copies.
Reads in level from /levels at runtime
TODO: pooling for walls? only load walls that are in the viewport
ActiveWalls and Walls. Only ActiveWalls are used for light rendering
*/

int LevelManager::UNIT_SIZE = Gamedata::getInstance().getXmlInt("world/unitSize");

LevelManager::LevelManager() :
  walls(),
  freeWalls(),
  wallVertices(),
  collectables(),
  freeCollectables(),
  spawnPoint() {
  loadLevel("levels/" + Gamedata::getInstance().getXmlStr("level/name"));
}

LevelManager::~LevelManager(){
  for(auto w: walls) delete w.second;
  for(auto w: freeWalls) delete w;
  for(Collectable* c: collectables) delete c;
  for(Collectable* c: freeCollectables) delete c;
}

LevelManager& LevelManager::getInstance(){
  static LevelManager levelManager;
  return levelManager;
}

// hopefully won't need an update call.
void LevelManager::update(){
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
    c->setTo(false, false, NULL);
  } else {
    c = new Collectable("Collectable");
  }

  c->setPosition(Vector2f(scaledX + UNIT_SIZE*x, scaledY + UNIT_SIZE*y));
  collectables.push_back(c);
}

void LevelManager::addCollectables(){
}

// TODO: this should put collectable into freeCollectables, but that currently
// will cause the explosion to not be drawn. Need to find fix for this because
// currently just wasting memory
void LevelManager::removeCollectable(Collectable* c){
  std::vector<Collectable*>::const_iterator it;
  it = std::find_if(collectables.begin(), collectables.end(),
    [&c](Collectable* c1){return c1 == c; });
  if(it != collectables.end()) (*it)->softDelete();
}

std::vector<int> strToIntVec(std::string& s){
  std::stringstream ss(s);
  std::string coord;
  std::vector<int> v;
  while(std::getline(ss, coord, ' ')){
    v.push_back(std::stoi(coord));
  }
  return v;
}

void LevelManager::setSpawnPoint(Vector2f v){
  int scaledX =  UNIT_SIZE/2, scaledY = UNIT_SIZE/2;
  spawnPoint = Vector2f(scaledX + UNIT_SIZE*v[0], scaledY + UNIT_SIZE*v[1]);
}

void LevelManager::parseLine(std::string& l){
  std::vector<int> v = strToIntVec(l);
  if(v.size() == 2){
    addCollectable(v[0], v[1]);
  } else {
    addWall(l);
  }
}

void LevelManager::loadLevel(const std::string& name){
  // std::cout << walls.size() << std::endl;
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
  // I don't like that we have to call this directly vvv
  // for(Collectable* c : freeCollectables) c->getLight()->update();

  std::ifstream levelData;
  std::string line;
  levelData.open(name);
  if(levelData.is_open()){
    if(std::getline(levelData, line)){
      std::vector<int> v = strToIntVec(line);
      setSpawnPoint(Vector2f(v[0], v[1]));
    }
    while(std::getline(levelData, line)){
      // addWall(line);
      parseLine(line);
    }
    levelData.close();
    std::cout << walls.size() << std::endl;

  }

  // border of the screen
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
}
