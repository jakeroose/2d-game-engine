#include <vector>
#include <SDL.h>
#include <string>

#include "wall.h"
#include "viewport.h"
#include "renderContext.h"
#include "vector2f.h"
#include "levelManager.h"

int Wall::count = 0;

Wall::Wall(int x1, int y1, int x2, int y2) :
  // convert from coords to rect with 1 width
  rect({
    x1*LevelManager::UNIT_SIZE,
    y1*LevelManager::UNIT_SIZE,
    (x2 == x1 ? 1 : (x2-x1)*LevelManager::UNIT_SIZE),
    (y2 == y1 ? 1 : (y2-y1)*LevelManager::UNIT_SIZE) }),
    id("wall_" + std::to_string(Wall::count)) {
  count++;
}

Wall::Wall(const SDL_Rect& r) : rect(r), id("wall_"+std::to_string(Wall::count)){
  count++;
}

std::ostream& operator<<(std::ostream out, const Wall& w){
  return out << w.getId();
}
std::ostream& operator<<(std::ostream out, const Wall* w){
  return out << w->getId();
}

std::vector<Vector2f> Wall::getVertices(){
  std::vector<Vector2f> v = {
    Vector2f(rect.x, rect.y), // top left
    Vector2f(rect.x + rect.w, rect.y), // top right
    Vector2f(rect.x + rect.w, rect.y + rect.h), // bottom right
    Vector2f(rect.x, rect.y + rect.h)  // bottom left
  };

  return v;

}

void Wall::draw() const {
  SDL_Renderer* renderer = RenderContext::getInstance()->getRenderer();
  SDL_Rect r = {
    rect.x - (int) Viewport::getInstance().getX(),
    rect.y - (int) Viewport::getInstance().getY(),
    rect.w,
    rect.h
  };
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderDrawRect(renderer, &r);
  SDL_RenderPresent(renderer);
}
