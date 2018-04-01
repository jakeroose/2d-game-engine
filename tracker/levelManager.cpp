#include <iostream>
#include <fstream>
#include <sstream>
#include "levelManager.h"
#include "light.h"
#include "vector2f.h"
#include "wall.h"
// #include "collectable.h"

/*
LevelManager should let me easily add walls to a level using arbitrary in-game
coordinates instead of specific pixel coordinates.
It should keep track of all of the walls in the game so that classes such as
Light and Player don't have to have their own copies.
Reads in level from /levels at runtime
TODO: pooling for walls? only load walls that are in the viewport
*/

int LevelManager::UNIT_SIZE = Gamedata::getInstance().getXmlInt("world/unitSize");

LevelManager::LevelManager() :
  walls(),
  wallVertices(),
  collectables() {
  loadLevel("levels/" + Gamedata::getInstance().getXmlStr("level/name"));
}

LevelManager::~LevelManager(){
  for(auto w: walls){
    delete w.second;
  }
  for(Collectable* c: collectables) delete c;
}

LevelManager& LevelManager::getInstance(){
  static LevelManager levelManager;
  return levelManager;
}

// hopefully won't need an update call.
void LevelManager::update(){
  // for(Collectable* c : collectables){
  //   c->update();
  // }
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

  addWall(new Wall(v[0], v[1], v[2], v[3]));
}

/* Adds a collectable to the tile at x, y */
void LevelManager::addCollectable(int x, int y){
  int scaledX =  UNIT_SIZE/2, scaledY = UNIT_SIZE/2;
  Collectable* c = new Collectable("Collectable");
  c->setPosition(Vector2f(scaledX + UNIT_SIZE*x, scaledY + UNIT_SIZE*y));
  std::cout << "here" << std::endl;
  collectables.push_back(c);
  // initialize other things and stuff
  // c->update();
}

void LevelManager::addCollectables(){
  addCollectable(2, 4);
}

void LevelManager::loadLevel(const std::string& name){
  std::ifstream levelData;
  std::string line;
  levelData.open(name);
  if(levelData.is_open()){
    while(std::getline(levelData, line)){
      addWall(line);
    }
    levelData.close();
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

  // add collectables after walls bc their lights depend on there being walls
  addCollectables();
}
