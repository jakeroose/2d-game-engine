#ifndef WALL__H
#define WALL__H
#include <vector>
#include <SDL.h>
#include "vector2f.h"

// 1px wide rect representing a wall.
// currenlty can only have vertical and horizontal walls, no diagonals.
class Wall {
public:
  Wall(int x1, int y1, int x2, int y2);
  Wall(const SDL_Rect& r);
  void draw() const;
  const SDL_Rect& getRect(){ return rect; }
  void display(){
    std::cout << rect.x << "," << rect.y << "," << rect.w << "," << rect.h << std::endl;
  }
  const std::string& getId() const { return id; }
  // const std::string getInfo() const;
  std::vector<Vector2f> getVertices();
private:
  SDL_Rect rect;
  static int count;
  std::string id;
};
#endif
