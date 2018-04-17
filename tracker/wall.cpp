#include <vector>
#include <SDL.h>
#include <string>
#include <sstream>

#include "wall.h"
#include "viewport.h"
#include "renderContext.h"
#include "vector2f.h"
#include "levelManager.h"
#include "gamedata.h"

int Wall::count = 0;
int ww = Gamedata::getInstance().getXmlInt("world/wallWidth");

Wall::Wall(int x1, int y1, int x2, int y2) :
  // convert from coords to rect with 1 width
  rect({
    x1*LevelManager::UNIT_SIZE,
    y1*LevelManager::UNIT_SIZE,
    (x2 == x1 ? ww : (x2-x1)*LevelManager::UNIT_SIZE),
    (y2 == y1 ? ww : (y2-y1)*LevelManager::UNIT_SIZE) }),
  id("wall_" + std::to_string(Wall::count)),
  type(rect.w == ww ? WallType::wall : WallType::floor) {
  count++;
}

Wall::Wall(const SDL_Rect& r) :
  rect(r),
  id("wall_"+std::to_string(Wall::count)),
  type(rect.w == 1 ? WallType::wall : WallType::floor) {
  count++;
}

std::ostream& operator<<(std::ostream& out, const Wall* w){
  // return out << w.getId();
  SDL_Rect r = w->getRect();
  return out <<
    (int)(r.x/LevelManager::UNIT_SIZE) <<
    (int)(r.y/LevelManager::UNIT_SIZE) <<
    (int)((r.w+r.x)/LevelManager::UNIT_SIZE) <<
    (int)((r.h+r.y)/LevelManager::UNIT_SIZE);
}

// returns coords of walls scaled to level units
std::string Wall::getSmallCoordString(){
  SDL_Rect r = rect;
  std::stringstream s;
  s << (int)(r.x/LevelManager::UNIT_SIZE) << " " <<
  (int)(r.y/LevelManager::UNIT_SIZE) << " " <<
  (int)((r.w+r.x)/LevelManager::UNIT_SIZE) << " " <<
  (int)((r.h+r.y)/LevelManager::UNIT_SIZE) << " ";
  return s.str();
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

void Wall::setTo(int x1, int y1, int x2, int y2){
  rect = {
    x1*LevelManager::UNIT_SIZE,
    y1*LevelManager::UNIT_SIZE,
    (x2 == x1 ? 1 : (x2-x1)*LevelManager::UNIT_SIZE),
    (y2 == y1 ? 1 : (y2-y1)*LevelManager::UNIT_SIZE)
  };
  type = rect.w == 1 ? WallType::wall : WallType::floor;
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
}
