#ifndef LEVELMANAGER__H
#define LEVELMANAGER__H

#include <map>
#include <string>
#include <SDL.h>

#include "wall.h"

class LevelManager {
public:
  LevelManager() : walls(), wallVertices() {}
  LevelManager(const LevelManager&) = delete;
  ~LevelManager();
  static LevelManager& getInstance();

  void addWall(Wall* w); //{walls.emplace(w->getId(), w); }
  void addWall(int x1, int y1, int x2, int y2){
    addWall(new Wall(x1, y1, x2, y2));
  }
  void addWall(const SDL_Rect& r){
    addWall(new Wall(r));
  }
  const std::map<std::string, Wall*>& getWalls(){ return walls; }
  const std::map<std::string, std::vector<Vector2f> >& getWallVertices(){
    return wallVertices;
  }
  const Wall* getWall(const std::string& s){ return walls.find(s)->second; }
  static int UNIT_SIZE;
private:
  std::map<std::string, Wall*> walls;
  std::map<std::string, std::vector<Vector2f> > wallVertices;
};
#endif