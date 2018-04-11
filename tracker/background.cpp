#include "background.h"
#include "imageFactory.h"
#include "renderContext.h"
#include "vector2f.h"
#include "gamedata.h"
#include "viewport.h"
#include <SDL.h>

class BackgroundSprite {
public:
  BackgroundSprite() = delete;
  BackgroundSprite(Image* s, int i);
  void update(Uint32 t);
  void draw() const;
private:
  Image* image;
  Vector2f offset;
  float scrollFactor, gravityScale, imageScale;
};

BackgroundSprite::BackgroundSprite(Image* s, int i) :
  image(s),
  offset(Vector2f(Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldWidth()),
                  Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldHeight()))),
  scrollFactor(i),
  gravityScale(Gamedata::getInstance().getRandFloat(0.1f, 0.2f)),
  imageScale(Gamedata::getInstance().getRandFloat(0.8f, 1.5f))
  {}

void BackgroundSprite::update(Uint32 t) {
  offset[1] += t*gravityScale/scrollFactor;
  if(offset[1] > Viewport::getInstance().getWorldHeight()){
    offset[1] = (int)offset[1] % Viewport::getInstance().getWorldHeight();
    offset[0] = Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldWidth());
  }
}

void BackgroundSprite::draw() const {
  SDL_Renderer* renderer = RenderContext::getInstance()->getRenderer();
  Image* i = image;

  // need to scale center of image to match scaled image
  SDL_Point center = {
    static_cast<int>(imageScale*(float)i->getWidth()/2.0f),
    static_cast<int>(imageScale*(float)i->getHeight()/2.0f)
  };
  SDL_Rect pos = i->getSurface()->clip_rect;
  pos.y = offset[1] - Viewport::getInstance().getY();
  pos.x = offset[0] - Viewport::getInstance().getX()/scrollFactor;
  pos.w *= imageScale;
  pos.h *= imageScale;
  SDL_RenderCopyEx(
    renderer,
    i->getTexture(),
    NULL,
    &pos,
    offset[1], // rotation based on y position so it scales nice w/ velocity
    &center,
    SDL_FLIP_NONE
  );

}

Background::Background() :
  image(),
  images(),
  ticks(),
  name()
  {}

Background::Background(const std::string& n) :
  image(ImageFactory::getInstance().getImage(n)),
  images(),
  ticks(),
  name(n)
  {}

Background::~Background(){
  delete image;
  for(auto e : images) delete e;
}
void Background::initialize(){
  for(int i = 0; i < 20; i ++){
    images.push_back(new BackgroundSprite(image,
       Gamedata::getInstance().getXmlInt(name+"/scroll")));
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
