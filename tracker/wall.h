#ifndef WALL__H
#define WALL__H
#include <vector>
#include <SDL.h>
#include "vector2f.h"

enum class WallType { wall, floor };

// 1px wide rect representing a wall.
// currenlty can only have vertical and horizontal walls, no diagonals.
class Wall {
public:
  Wall(int x1, int y1, int x2, int y2);
  Wall(const SDL_Rect& r);
  void draw() const;
  SDL_Rect getRect() const { return rect; }
  void display(){
    std::cout << rect.x << "," << rect.y << "," << rect.w << "," << rect.h << std::endl;
  }
  const std::string& getId() const { return id; }
  std::string getSmallCoordString();
  std::vector<Vector2f> getVertices();
  WallType getType() { return type; }
  int getMaxx(){ return rect.x + rect.w; }
  int getMaxy(){ return rect.y + rect.h; }
  int getMinx(){ return rect.x; }
  int getMiny(){ return rect.y; }
  void setTo(int x1, int y1, int x2, int y2);
private:
  SDL_Rect rect;
  static int count;
  std::string id;
  WallType type;
};
#endif
