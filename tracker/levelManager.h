#ifndef LEVELMANAGER__H
#define LEVELMANAGER__H

#include <map>
#include <string>
#include <SDL.h>

#include "wall.h"
#include "collectable.h"

class LevelManager {
public:
  LevelManager();
  LevelManager(const LevelManager&) = delete;
  ~LevelManager();
  static LevelManager& getInstance();

  void update();

  void addWall(Wall* w);
  void addWall(const std::string& s);
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
  void loadLevel(const std::string& s);
  void addCollectable(int x, int y);
  void addCollectables();
  void removeCollectable(Collectable* c);
  const std::vector<Collectable*>& getCollectables(){ return collectables; }
  const Vector2f& getSpawnPoint(){ return spawnPoint; }
  void setSpawnPoint(Vector2f v);
  void parseLine(std::string& l);

  static int UNIT_SIZE;
private:
  std::map<std::string, Wall*> walls;
  std::vector<Wall*> freeWalls;
  std::map<std::string, std::vector<Vector2f> > wallVertices;
  std::vector<Collectable*> collectables;
  std::vector<Collectable*> freeCollectables;
  Vector2f spawnPoint;
};
#endif
