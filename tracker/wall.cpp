#include <vector>
#include <SDL.h>
#include "wall.h"
#include "viewport.h"
#include "renderContext.h"
#include "vector2f.h"

int UNIT_SIZE = 64; // READ IN FROM XML

Wall::Wall(int x1, int y1, int x2, int y2) : rect({x1*UNIT_SIZE, y1*UNIT_SIZE, x2*UNIT_SIZE, y2*UNIT_SIZE }) {}
Wall::Wall(const SDL_Rect& r) : rect(r){}

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
