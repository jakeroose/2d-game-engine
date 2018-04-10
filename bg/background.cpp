#include "background.h"
#include "imageFactory.h"
#include "renderContext.h"
#include "vector2f.h"
#include "gamedata.h"
#include "viewport.h"
#include <SDL.h>

float scaler = 0.1f;

class BackgroundSprite {
public:
  BackgroundSprite();
  BackgroundSprite(Image* s);
  void update(Uint32 t);
  void draw() const;
private:
  Image* image;
  Vector2f offset;
  float scrollFactor;
};

BackgroundSprite::BackgroundSprite() :
  image(),
  offset(Vector2f(Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldWidth()),
                  Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldHeight()))),
  scrollFactor(1)
  {}

BackgroundSprite::BackgroundSprite(Image* s) :
  image(s),
  offset(Vector2f(Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldWidth()),
                  Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldHeight()))),
  scrollFactor(1)
  {}

void BackgroundSprite::update(Uint32 t) {
  offset[1] += t*scaler;
  offset[1] = (int)offset[1] % Viewport::getInstance().getWorldHeight();
}

void BackgroundSprite::draw() const {
  SDL_Renderer* renderer = RenderContext::getInstance()->getRenderer();
  Image* i = image;

  SDL_Point cent = {i->getWidth()/2, i->getHeight()/2 };
  SDL_Rect pos = i->getSurface()->clip_rect;
  pos.y = offset[1] - Viewport::getInstance().getY();
  pos.x = offset[0] - Viewport::getInstance().getX()/scrollFactor;
  SDL_RenderCopyEx(
    renderer,
    i->getTexture(),
    NULL,
    &pos,
    offset[1], // rotation
    &cent,
    SDL_FLIP_NONE
  );

}

Background::Background() :
  image(),
  images()
  {}

Background::Background(const std::string& name) :
  image(ImageFactory::getInstance().getImage(name)),
  images()
  {}

Background::~Background(){
  delete image;
  for(auto e : images) delete e;
}
void Background::initialize(){
  for(int i = 0; i < 20000; i ++){
    images.push_back(new BackgroundSprite(image));
  }
}

void Background::update(Uint32 t){
  for(auto i : images){
    i->update(t);
  }
  ticks += t;
}

void Background::draw() const {
  for(auto i : images){
    i->draw();
  }
}
