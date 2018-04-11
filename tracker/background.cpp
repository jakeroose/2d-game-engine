#include "background.h"
#include "imageFactory.h"
#include "renderContext.h"
#include "vector2f.h"
#include "gamedata.h"
#include "viewport.h"
#include <SDL.h>
#include <algorithm>

class BackgroundSprite {
public:
  BackgroundSprite() = delete;
  BackgroundSprite(Image* s);
  void update(Uint32 t);
  void draw() const;
  float getDistanceScale(){ return distanceScale; }
private:
  Image* image;
  Vector2f offset;
  float gravityScale, distanceScale, imageScale;
};


BackgroundSprite::BackgroundSprite(Image* s) :
  image(s),
  offset(Vector2f(Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldWidth()),
                  Gamedata::getInstance().getRandFloat(0, Viewport::getInstance().getWorldHeight()))),
  gravityScale(Gamedata::getInstance().getXmlFloat("background/gravity")),
  distanceScale(Gamedata::getInstance().getRandFloat(0.5f, 3.0f)),
  imageScale(1/distanceScale)
  {}

void BackgroundSprite::update(Uint32 t) {
  // update it's y coord per tick
  offset[1] += t*gravityScale/distanceScale; // close -> fall faster

  // reset y and give new x once it reaches the bottom
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
  pos.x = offset[0] - Viewport::getInstance().getX()/distanceScale;
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

/* Initialized to number of objects in the background */
Background::Background(int c) :
  images(),
  ticks(),
  objectCount(c)
  {}

Background::~Background(){
  for(auto e : images) delete e;
}

void Background::initialize(){
  // NOTE: these squares never get deleted D:
  Image* square1 = ImageFactory::getInstance().getImage("Square");
  Image* square2 = ImageFactory::getInstance().getImage("Square1");
  
  for(int i = 0; i < objectCount; i ++){
    if(i%2){
      images.push_back(new BackgroundSprite(square1));
    } else {
      images.push_back(new BackgroundSprite(square2));
    }
  }

  std::sort(images.begin(), images.end(), [](
    BackgroundSprite* s1, BackgroundSprite* s2){
      return s1->getDistanceScale() > s2->getDistanceScale();
  });
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
