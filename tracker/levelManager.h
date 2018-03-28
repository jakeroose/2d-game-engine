#ifndef LEVELMANAGER__H
#define LEVELMANAGER__H

#include <map>
#include "wall.h"

class LevelManager {
public:
  LevelManager() : walls(){}
  LevelManager(const LevelManager&) = delete;

  void addWall(Wall* w){walls.emplace(walls.size(), w);}
  const std::map<int, Wall*>& getWalls(){ return walls; }
  static const int UNIT_SIZE;
private:
  std::map<int, Wall*> walls;
};
#endif
