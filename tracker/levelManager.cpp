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
