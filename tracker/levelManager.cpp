#include <iostream>
#include <fstream>
#include <sstream>
#include "levelManager.h"
#include "light.h"
#include "vector2f.h"
#include "wall.h"

/*
LevelManager should let me easily add walls to a level using arbitrary in-game
coordinates instead of specific pixel coordinates.
It should keep track of all of the walls in the game so that classes such as
Light and Player don't have to have their own copies.
TODO: It should allow me to read in a level from a file so that I don't have to
recompile the game to see if my level makes sense.

*/

int LevelManager::UNIT_SIZE = Gamedata::getInstance().getXmlInt("world/unitSize");

LevelManager& LevelManager::getInstance(){
  static LevelManager levelManager;
  return levelManager;
}

LevelManager::~LevelManager(){
  for(auto w: walls){
    delete w.second;
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

  addWall(new Wall(v[0], v[1], v[2], v[3]));
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
  SDL_Rect worldBorder = { 0, 0,
    Gamedata::getInstance().getXmlInt("world/width"),
    Gamedata::getInstance().getXmlInt("world/height")
  };
  addWall(worldBorder);
}
