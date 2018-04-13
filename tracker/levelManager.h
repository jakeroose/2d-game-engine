#ifndef LEVELMANAGER__H
#define LEVELMANAGER__H

#include <map>
#include <string>
#include <SDL.h>

#include "wall.h"
#include "collectable.h"
#include "smartSprite.h"

enum class LevelState { loading, running, editing };

class LevelManager {
public:
  LevelManager();
  LevelManager(const LevelManager&) = delete;
  ~LevelManager();
  static LevelManager& getInstance();

  void update();

  const std::map<std::string, Wall*>& getWalls(){ return walls; }
  const std::map<std::string, std::vector<Vector2f> >& getWallVertices(){
    return wallVertices;
  }
  const Wall* getWall(const std::string& s){ return walls.find(s)->second; }
  void loadLevel(const std::string& s);

  void removeCollectable(Collectable* c);
  const std::vector<Collectable*>& getCollectables(){ return collectables; }
  void addEnemy(int x, int y);
  void removeEnemy(SmartSprite* s);
  const std::vector<SmartSprite*>& getEnemies(){ return enemies; }
  const Vector2f& getSpawnPoint(){ return spawnPoint; }

  int getWallCount() const { return (int)walls.size();}
  int getFreeWallCount() const { return (int)freeWalls.size();}
  int getCollectableCount() const { return (int)collectables.size();}
  int getFreeCollectableCount() const { return (int)freeCollectables.size();}
  int getTotalLightIntersections() const;
  int getTotalFreeIntersections() const;

  void saveLevel() const;
  void toggleLevelEdit();
  bool inEditMode() const { return state == LevelState::editing; }

  void setCursor(const Vector2f& v);
  const Vector2f getCursor() const { return cursorCoords*UNIT_SIZE; }
  void setAnchor();
  Vector2f* getAnchor() const { return anchor; }
  void resetAnchor() { delete anchor; anchor = NULL; }

  static int UNIT_SIZE;

  const LevelManager& operator=(const LevelManager& rhs) = delete;
private:
  enum LOADING_TYPE { NONE, ENEMY, PLAYER, WALL, COLLECTABLE};
  std::map<std::string, Wall*> walls;
  std::vector<Wall*> freeWalls;
  std::map<std::string, std::vector<Vector2f> > wallVertices;
  std::vector<Collectable*> collectables;
  std::vector<Collectable*> freeCollectables;
  std::vector<SmartSprite*> enemies;
  std::vector<SmartSprite*> freeEnemies;
  Vector2f spawnPoint;
  LOADING_TYPE loadingType;
  LevelState state;
  Vector2f cursorCoords;
  Vector2f* anchor;

  void setSpawnPoint(Vector2f v);
  void parseLine(std::string& l);
  void addWall(Wall* w);
  void addWall(const std::string& s);
  // vvv should use object pooling vvv
  void addWall(int x1, int y1, int x2, int y2){addWall(new Wall(x1, y1, x2, y2));}
  void addWall(const SDL_Rect& r){ addWall(new Wall(r)); }
  void addCollectable(int x, int y);
};
#endif
